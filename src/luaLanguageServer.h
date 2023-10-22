#ifndef LUALANGUAGESERVER_H
#define LUALANGUAGESERVER_H

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <mutex>
#include <thread>

#include "platform/subprocess.h"

using namespace godot;

class LuaLanguageServer : public RefCounted {
	GDCLASS(LuaLanguageServer, RefCounted);

protected:
	static void _bind_methods();

public:
	enum LuaLSError {
		LUALS_OK,
		LUALS_NOT_RUNNING,
		LUALS_ALREADY_RUNNING,
		LUALS_FAILED_TO_WRITE,
		LUALS_FAILED_TO_START,
		LUALS_FAILED_TO_STOP,
		LUALS_NO_MESSAGE,
		LUALS_NOT_FOUND,
		LUALS_WORKSPACE_NOT_FOUND,
		LUALS_INVALID_HEADER,
		LUALS_INVALID_CONTENT,
	};

	LuaLanguageServer();
	~LuaLanguageServer();

	LuaLSError start(String luals_path, String luals_workspace_path);

	LuaLSError stop();

	void confirm_code_completion(bool replace);
	void request_code_completion(bool force);
	TypedArray<Dictionary> filter_code_completion_candidates(const TypedArray<Dictionary> &candidates) const;

	bool is_running() const;

	static String error_string(LuaLSError error);

private:
	void status_monitor();

	void _on_server_started();
	void _on_server_stopped();
	void _check_stderr();

	LuaLanguageServer::LuaLSError send_message(String message) const;
	std::pair<String, LuaLSError> read_message() const;

	bool running = false;
	String workspace_path = "";
	mutable std::mutex running_mutex;

	Subprocess *server_process = nullptr;
	mutable std::mutex server_process_mutex;

	std::thread *status_monitor_thread = nullptr;
};

VARIANT_ENUM_CAST(LuaLanguageServer::LuaLSError);

#endif