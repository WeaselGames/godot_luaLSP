#include "luaLanguageServer.h"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <utility>
#include <vector>

LuaLanguageServer::LuaLanguageServer() {
	server_process = Subprocess::new_platform_subprocess();
}

LuaLanguageServer::~LuaLanguageServer() {
	if (is_running()) {
		LuaLSError err = stop();
		if (err != LUALS_OK) {
			UtilityFunctions::printerr(vformat("Failed to stop LuaLanguageServer: %s", error_string(err)));
		}
	}

	if (status_monitor_thread != nullptr) {
		status_monitor_thread->join();
		delete status_monitor_thread;
	}

	delete server_process;
}

void LuaLanguageServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("stop"), &LuaLanguageServer::stop);
	ClassDB::bind_method(D_METHOD("is_running"), &LuaLanguageServer::is_running);
	ClassDB::bind_method(D_METHOD("start", "luals_path", "luals_workspace_path"), &LuaLanguageServer::start);

	ClassDB::bind_method(D_METHOD("confirm_code_completion", "replace"), &LuaLanguageServer::confirm_code_completion);
	ClassDB::bind_method(D_METHOD("request_code_completion", "force"), &LuaLanguageServer::request_code_completion);
	ClassDB::bind_method(D_METHOD("filter_code_completion_candidates", "candidates"), &LuaLanguageServer::filter_code_completion_candidates);

	ClassDB::bind_static_method("LuaLanguageServer", D_METHOD("error_string", "error"), &LuaLanguageServer::error_string);

	BIND_ENUM_CONSTANT(LUALS_OK);
	BIND_ENUM_CONSTANT(LUALS_NOT_RUNNING);
	BIND_ENUM_CONSTANT(LUALS_ALREADY_RUNNING);
	BIND_ENUM_CONSTANT(LUALS_FAILED_TO_START);
	BIND_ENUM_CONSTANT(LUALS_FAILED_TO_STOP);
	BIND_ENUM_CONSTANT(LUALS_NO_MESSAGE);
	BIND_ENUM_CONSTANT(LUALS_NOT_FOUND);
	BIND_ENUM_CONSTANT(LUALS_WORKSPACE_NOT_FOUND);
	BIND_ENUM_CONSTANT(LUALS_INVALID_HEADER);
	BIND_ENUM_CONSTANT(LUALS_INVALID_CONTENT);
}

void LuaLanguageServer::confirm_code_completion(bool replace) {
	if (!is_running()) {
		return;
	}
}

void LuaLanguageServer::request_code_completion(bool force) {
	if (!is_running()) {
		return;
	}
}

TypedArray<Dictionary> LuaLanguageServer::filter_code_completion_candidates(const TypedArray<Dictionary> &candidates) const {
	if (!is_running()) {
		return candidates;
	}

	return candidates;
}

LuaLanguageServer::LuaLSError LuaLanguageServer::stop() {
	if (!is_running()) {
		return LUALS_NOT_RUNNING;
	}

	server_process_mutex.lock();
	Subprocess::Error err = server_process->kill_process();
	server_process_mutex.unlock();
	if (err != Subprocess::OK) {
		UtilityFunctions::printerr(vformat("Failed to kill LuaLanguageServer: %d", err));
	}

	return LUALS_OK;
}

bool LuaLanguageServer::is_running() const {
	running_mutex.lock();
	bool running = this->running;
	running_mutex.unlock();
	return running;
}

LuaLanguageServer::LuaLSError LuaLanguageServer::start(String luals_path, String luals_workspace_path) {
	if (is_running()) {
		return LUALS_ALREADY_RUNNING;
	}

	workspace_path = luals_workspace_path;

	Ref<FileAccess> file = FileAccess::open(luals_path, FileAccess::READ);
	if (!file.is_valid() || !file->is_open()) {
		return LUALS_NOT_FOUND;
	}

	String path = file->get_path_absolute();
	file->close();

	server_process_mutex.lock();
	Subprocess::Error err = server_process->start(path.utf8().get_data(), {});
	server_process_mutex.unlock();
	if (err != Subprocess::OK) {
		return LUALS_FAILED_TO_START;
	}

	status_monitor_thread = new std::thread(&LuaLanguageServer::status_monitor, this);

	return LUALS_OK;
}

void LuaLanguageServer::status_monitor() {
	// Wait for the server to start
	server_process_mutex.lock();
	Subprocess::Status status = server_process->get_status();
	server_process_mutex.unlock();
	UtilityFunctions::print("Waiting for LuaLanguageServer to start");
	while (status != Subprocess::RUNNING) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		UtilityFunctions::print(vformat("LuaLanguageServer status: %d", status));
		server_process_mutex.lock();
		status = server_process->get_status();
		server_process_mutex.unlock();
	}

	_on_server_started();

	// Wait for the server to stop
	while (status == Subprocess::RUNNING) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		_check_stderr();
		server_process_mutex.lock();
		status = server_process->get_status();
		server_process_mutex.unlock();
	}

	_on_server_stopped();
}

void LuaLanguageServer::_on_server_started() {
	UtilityFunctions::print("LuaLanguageServer started");

	running_mutex.lock();
	running = true;
	running_mutex.unlock();

	Dictionary initialize_message;
	Dictionary initialize_params;
	initialize_message["jsonrpc"] = "2.0";
	initialize_message["id"] = 0;
	initialize_message["method"] = "initialize";

	initialize_message["params"] = initialize_params;
	initialize_params["processId"] = OS::get_singleton()->get_process_id();
	initialize_params["rootUri"] = workspace_path;

	initialize_params["capabilities"] = Dictionary();

	LuaLSError ls_err = send_message(JSON::stringify(initialize_message));
	if (ls_err != LUALS_OK) {
		UtilityFunctions::printerr(vformat("Failed to send initialize message to LuaLanguageServer: %s", error_string(ls_err)));
		return;
	}

	std::pair<String, LuaLSError> response = read_message();
	if (response.second != LUALS_OK) {
		UtilityFunctions::printerr(vformat("Failed to read initialize response from LuaLanguageServer: %s", error_string(response.second)));
		return;
	}

	UtilityFunctions::print(vformat("LuaLanguageServer response: %s", response.first));
}

void LuaLanguageServer::_on_server_stopped() {
	UtilityFunctions::print("LuaLanguageServer stopped");

	running_mutex.lock();
	running = false;
	running_mutex.unlock();
}

void LuaLanguageServer::_check_stderr() {
	std::vector<char> error_chars;
	String error;

	server_process_mutex.lock();
	std::pair<char, Subprocess::Error> ret = server_process->read_error_char();
	server_process_mutex.unlock();
	if (ret.second == Subprocess::NOTHING_TO_READ) {
		return;
	}

	while (ret.second == Subprocess::OK) {
		error_chars.push_back(ret.first);
		server_process_mutex.lock();
		ret = server_process->read_error_char();
		server_process_mutex.unlock();
	}

	if (error_chars.size() > 0) {
		error.parse_utf8(error_chars.data(), error_chars.size());
		if (error.is_empty()) {
			return;
		}

		UtilityFunctions::printerr(vformat("LuaLanguageServer error: %s", error));
	}
}

LuaLanguageServer::LuaLSError LuaLanguageServer::send_message(String message) const {
	if (!is_running()) {
		return LUALS_NOT_RUNNING;
	}

	std::string header = vformat("Content-Length: %d\n\n", message.length()).utf8().get_data();
	server_process_mutex.lock();
	Subprocess::Error err = server_process->write_message(header + message.utf8().get_data());
	server_process_mutex.unlock();
	if (err != Subprocess::OK) {
		return LUALS_FAILED_TO_WRITE;
	}

	return LUALS_OK;
}

std::pair<String, LuaLanguageServer::LuaLSError> LuaLanguageServer::read_message() const {
	if (!is_running()) {
		return std::make_pair("", LUALS_NOT_RUNNING);
	}

	std::vector<char> header_chars;
	std::pair<char, Subprocess::Error> out;
	server_process_mutex.lock();
	while ((out = server_process->read_output_char()).first != '\n') {
		if (out.second != Subprocess::OK) {
			if (out.second == Subprocess::NOTHING_TO_READ) {
				server_process_mutex.unlock();
				return std::make_pair("", LUALS_NO_MESSAGE);
			}

			server_process_mutex.unlock();
			return std::make_pair("", LUALS_INVALID_HEADER);
		}
		header_chars.push_back(out.first);
	}
	server_process_mutex.unlock();

	String header;
	header.parse_utf8(header_chars.data(), header_chars.size());

	if (!header.contains("Content-Length: ")) {
		return std::make_pair("", LUALS_INVALID_HEADER);
	}

	int content_length = header.get_slice("Content-Length: ", 1).to_int();
	if (content_length == 0) {
		return std::make_pair("", LUALS_INVALID_HEADER);
	}

	server_process_mutex.lock();
	std::pair<std::string, Subprocess::Error> output = server_process->read_output(content_length);
	server_process_mutex.unlock();
	if (output.second != Subprocess::OK) {
		if (output.second == Subprocess::NOTHING_TO_READ) {
			return std::make_pair("", LUALS_INVALID_HEADER);
		}

		return std::make_pair("", LUALS_INVALID_CONTENT);
	}
	String content;
	content.parse_utf8(output.first.c_str(), output.first.length());
	return std::make_pair(content, LUALS_OK);
}

String LuaLanguageServer::error_string(LuaLSError error) {
	switch (error) {
		case LUALS_OK:
			return "No error";
		case LUALS_NOT_RUNNING:
			return "LuaLanguageServer is not running";
		case LUALS_ALREADY_RUNNING:
			return "LuaLanguageServer is already running";
		case LUALS_FAILED_TO_START:
			return "Failed to start LuaLanguageServer";
		case LUALS_FAILED_TO_STOP:
			return "Failed to stop LuaLanguageServer";
		case LUALS_NO_MESSAGE:
			return "No message to read";
		case LUALS_NOT_FOUND:
			return "LuaLanguageServer not found";
		case LUALS_WORKSPACE_NOT_FOUND:
			return "Workspace not found";
		case LUALS_INVALID_HEADER:
			return "Invalid message header";
		case LUALS_INVALID_CONTENT:
			return "Invalid message content";
		default:
			return vformat("Unknown error: %d", error);
	}
}