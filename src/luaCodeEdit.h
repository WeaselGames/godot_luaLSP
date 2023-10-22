#ifndef LUACODEEDIT_H
#define LUACODEEDIT_H

#include "luaHighlighter.h"
#include "luaLanguageServer.h"

#include <godot_cpp/classes/code_edit.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

using namespace godot;

class LuaCodeEdit : public CodeEdit {
	GDCLASS(LuaCodeEdit, CodeEdit);

protected:
	static void _bind_methods();

public:
	LuaCodeEdit();

	virtual void _enter_tree() override;
	virtual void _confirm_code_completion(bool replace) override;
	virtual void _request_code_completion(bool force) override;
	virtual TypedArray<Dictionary> _filter_code_completion_candidates(const TypedArray<Dictionary> &candidates) const;

	void _on_code_completion_timeout();

	Ref<LuaLanguageServer> get_lua_language_server() const;

	void set_lua_language_server_path(String luals_path);
	String get_lua_language_server_path() const;

	void set_lua_workspace_path(String luals_workspace_path);
	String get_lua_workspace_path() const;

	void set_code_completion_timeout(int timeout);
	int get_code_completion_timeout() const;

	void set_member_variable_color(const Color &color);
	Color get_member_variable_color() const;

	void set_function_color(const Color &color);
	Color get_function_color() const;

	void set_symbol_color(const Color &color);
	Color get_symbol_color() const;

	void set_number_color(const Color &color);
	Color get_number_color() const;

	void set_keyword_color(const Color &color);
	Color get_keyword_color() const;

	void set_comment_color(const Color &color);
	Color get_comment_color() const;

	void set_string_color(const Color &color);
	Color get_string_color() const;

private:
	String luals_path;
	String luals_workspace_path;

	TypedArray<String> code_completion_prefixes;

	Ref<LuaHighlighter> lua_highlighter;
	Ref<LuaLanguageServer> lua_language_server;

	Timer *code_completion_timer = nullptr;
};

#endif