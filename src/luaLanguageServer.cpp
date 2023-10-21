#include "luaLanguageServer.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <utility>

LuaLanguageServer::LuaLanguageServer() {
	server_process = Subprocess::new_platform_subprocess();
}

LuaLanguageServer::~LuaLanguageServer() {
	if (running) {
		stop();
	}

	delete server_process;
}

void LuaLanguageServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("stop"), &LuaLanguageServer::stop);
	ClassDB::bind_method(D_METHOD("is_running"), &LuaLanguageServer::is_running);
	ClassDB::bind_method(D_METHOD("start", "luals_path"), &LuaLanguageServer::start);
}

void LuaLanguageServer::confirm_code_completion(bool replace) {
	if (!running) {
		UtilityFunctions::printerr("LuaLanguageServer is not running!");
		return;
	}

	UtilityFunctions::print(vformat("Buffer: %s", read_message()));
}

void LuaLanguageServer::request_code_completion(bool force) {
	if (!running) {
		UtilityFunctions::printerr("LuaLanguageServer is not running!");
		return;
	}

	UtilityFunctions::print(vformat("Buffer: %s", read_message()));
}

TypedArray<Dictionary> LuaLanguageServer::filter_code_completion_candidates(const TypedArray<Dictionary> &candidates) const {
	if (!running) {
		UtilityFunctions::printerr("LuaLanguageServer is not running!");
		return candidates;
	}

	UtilityFunctions::print(vformat("Buffer: %s", read_message()));

	return candidates;
}

void LuaLanguageServer::stop() {
	if (!running) {
		return;
	}

	running = false;
	Subprocess::ProcessError err = server_process->kill_process();
	if (err != Subprocess::OK) {
		UtilityFunctions::printerr(vformat("Failed to kill LuaLanguageServer: %d", err));
	}
}

bool LuaLanguageServer::is_running() {
	return running;
}

Error LuaLanguageServer::start(String luals_path) {
	if (running) {
		return ERR_ALREADY_IN_USE;
	}

	Ref<FileAccess> file = FileAccess::open(luals_path, FileAccess::READ);
	if (!file->is_open()) {
		return ERR_FILE_CANT_READ;
	}
	String path = file->get_path_absolute();
	file->close();

	Subprocess::ProcessError err = server_process->start(path.utf8().get_data(), nullptr);
	if (err != Subprocess::OK) {
		return ERR_CANT_CREATE;
	}

	running = true;

	return OK;
}

void LuaLanguageServer::send_message(String message) {
	if (!running) {
		UtilityFunctions::printerr("LuaLanguageServer is not running!");
		return;
	}

	server_process->write_message(message.utf8().get_data(), message.length());
}

String LuaLanguageServer::read_message() const {
	if (!running) {
		UtilityFunctions::printerr("LuaLanguageServer is not running!");
		return "";
	}

	Subprocess::ProcessError err = server_process->poll();
	if (err != Subprocess::OK) {
		UtilityFunctions::printerr(vformat("Failed to poll LuaLanguageServer: %d", err));
		return "";
	}

	String buffer_content;
	const char *buffer = server_process->read_buffer();
	if (buffer != nullptr) {
		buffer_content = buffer;
	}

	return buffer_content;
}