#include "luaLanguageServer.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <utility>

LuaLanguageServer::LuaLanguageServer() {
}

LuaLanguageServer::~LuaLanguageServer() {
	if (running) {
		stop();
	}
}

void LuaLanguageServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("stop"), &LuaLanguageServer::stop);
	ClassDB::bind_method(D_METHOD("is_running"), &LuaLanguageServer::is_running);
	ClassDB::bind_method(D_METHOD("start", "luals_path"), &LuaLanguageServer::start);
	ClassDB::bind_method(D_METHOD("send_message", "message"), &LuaLanguageServer::send_message);
}

void LuaLanguageServer::stop() {
	if (!running) {
		return;
	}

	running = false;
	server_process->kill();
	delete server_process;
	server_process = nullptr;
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

	running = true;
	server_process = new subprocess::Popen({ path.utf8().get_data() },
			subprocess::input{ subprocess::PIPE },
			subprocess::output{ subprocess::PIPE });

	return OK;
}

String LuaLanguageServer::send_message(String message) {
	if (!running) {
		UtilityFunctions::printerr("LuaLanguageServer is not running!");
		return "";
	}

	const char *msg = message.utf8().get_data();
	server_process->send(msg, strlen(msg));
	std::pair<subprocess::OutBuffer, subprocess::ErrBuffer> result = server_process->communicate();
	if (result.second.buf.size() > 0) {
		UtilityFunctions::printerr(vformat("LuaLanguageServer comunication error: %s", result.second.buf.data()));
	}

	String output;
	output.parse_utf8(result.first.buf.data());
	return output;
}