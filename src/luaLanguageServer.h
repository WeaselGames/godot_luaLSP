#ifndef LUALANGUAGESERVER_H
#define LUALANGUAGESERVER_H

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "platform/subprocess.h"

using namespace godot;

class LuaLanguageServer : public RefCounted {
	GDCLASS(LuaLanguageServer, RefCounted);

protected:
	static void _bind_methods();

public:
	LuaLanguageServer();
	~LuaLanguageServer();

	Error start(String luals_path);

	void stop();

	void confirm_code_completion(bool replace);
	void request_code_completion(bool force);
	TypedArray<Dictionary> filter_code_completion_candidates(const TypedArray<Dictionary> &candidates) const;

	bool is_running();

private:
	bool running = false;

	void send_message(String message);
	String read_message() const;

	Subprocess *server_process = nullptr;
};

#endif