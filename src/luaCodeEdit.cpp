#include "luaCodeEdit.h"

#include <godot_cpp/variant/utility_functions.hpp>

LuaCodeEdit::LuaCodeEdit() {
	lua_highlighter.instantiate();

	code_completion_prefixes.push_back(".");
	code_completion_prefixes.push_back(":");
	code_completion_prefixes.push_back(",");
	code_completion_prefixes.push_back("=");
	code_completion_prefixes.push_back("(");
}

void LuaCodeEdit::_enter_tree() {
	set_syntax_highlighter(lua_highlighter);

	if (luals_path == "") {
		UtilityFunctions::printerr("LuaLanguageServer path is not set!");
		return;
	}

	Error err = lua_language_server->start(luals_path);
	if (err != OK) {
		UtilityFunctions::printerr(vformat("Failed to start LuaLanguageServer: %s", UtilityFunctions::error_string(err)));
	}

	set_code_completion_enabled(true);
	set_code_completion_prefixes((code_completion_prefixes));
}

void LuaCodeEdit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_member_variable_color", "color"), &LuaCodeEdit::set_member_variable_color);
	ClassDB::bind_method(D_METHOD("get_member_variable_color"), &LuaCodeEdit::get_member_variable_color);

	ClassDB::bind_method(D_METHOD("set_function_color", "color"), &LuaCodeEdit::set_function_color);
	ClassDB::bind_method(D_METHOD("get_function_color"), &LuaCodeEdit::get_function_color);

	ClassDB::bind_method(D_METHOD("set_symbol_color", "color"), &LuaCodeEdit::set_symbol_color);
	ClassDB::bind_method(D_METHOD("get_symbol_color"), &LuaCodeEdit::get_symbol_color);

	ClassDB::bind_method(D_METHOD("set_number_color", "color"), &LuaCodeEdit::set_number_color);
	ClassDB::bind_method(D_METHOD("get_number_color"), &LuaCodeEdit::get_number_color);

	ClassDB::bind_method(D_METHOD("set_keyword_color", "color"), &LuaCodeEdit::set_keyword_color);
	ClassDB::bind_method(D_METHOD("get_keyword_color"), &LuaCodeEdit::get_keyword_color);

	ClassDB::bind_method(D_METHOD("set_comment_color", "color"), &LuaCodeEdit::set_comment_color);
	ClassDB::bind_method(D_METHOD("get_comment_color"), &LuaCodeEdit::get_comment_color);

	ClassDB::bind_method(D_METHOD("set_string_color", "color"), &LuaCodeEdit::set_string_color);
	ClassDB::bind_method(D_METHOD("get_string_color"), &LuaCodeEdit::get_string_color);

	ClassDB::bind_method(D_METHOD("set_lua_language_server_path", "luals_path"), &LuaCodeEdit::set_lua_language_server_path);
	ClassDB::bind_method(D_METHOD("get_lua_language_server_path"), &LuaCodeEdit::get_lua_language_server_path);

	ClassDB::bind_method(D_METHOD("get_lua_language_server"), &LuaCodeEdit::get_lua_language_server);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "lua_language_server_path"), "set_lua_language_server_path", "get_lua_language_server_path");

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "member_variable_color"), "set_member_variable_color", "get_member_variable_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "function_color"), "set_function_color", "get_function_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "symbol_color"), "set_symbol_color", "get_symbol_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "number_color"), "set_number_color", "get_number_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "keyword_color"), "set_keyword_color", "get_keyword_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "true_nil_false_color"), "set_true_nil_false_color", "get_true_nil_false_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "comment_color"), "set_comment_color", "get_comment_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "string_color"), "set_string_color", "get_string_color");
}

void LuaCodeEdit::_confirm_code_completion(bool replace) {
	if (lua_language_server->is_running()) {
		lua_language_server->confirm_code_completion(replace);
		return;
	}
	UtilityFunctions::printerr("LuaLanguageServer is not running!");
}

void LuaCodeEdit::_request_code_completion(bool force) {
	if (lua_language_server->is_running()) {
		lua_language_server->request_code_completion(force);
		return;
	}

	UtilityFunctions::printerr("LuaLanguageServer is not running!");
}

TypedArray<Dictionary> LuaCodeEdit::_filter_code_completion_candidates(const TypedArray<Dictionary> &candidates) const {
	if (lua_language_server->is_running()) {
		return lua_language_server->filter_code_completion_candidates(candidates);
	}

	UtilityFunctions::printerr("LuaLanguageServer is not running!");
	return TypedArray<Dictionary>();
}

void LuaCodeEdit::set_member_variable_color(const Color &color) {
	lua_highlighter->set_member_variable_color(color);
}

Color LuaCodeEdit::get_member_variable_color() const {
	return lua_highlighter->get_member_variable_color();
}

void LuaCodeEdit::set_function_color(const Color &color) {
	lua_highlighter->set_function_color(color);
}

Color LuaCodeEdit::get_function_color() const {
	return lua_highlighter->get_function_color();
}

void LuaCodeEdit::set_symbol_color(const Color &color) {
	lua_highlighter->set_symbol_color(color);
}

Color LuaCodeEdit::get_symbol_color() const {
	return lua_highlighter->get_symbol_color();
}

void LuaCodeEdit::set_number_color(const Color &color) {
	lua_highlighter->set_number_color(color);
}

Color LuaCodeEdit::get_number_color() const {
	return lua_highlighter->get_number_color();
}

void LuaCodeEdit::set_keyword_color(const Color &color) {
	lua_highlighter->set_keyword_color(color);
}

Color LuaCodeEdit::get_keyword_color() const {
	return lua_highlighter->get_keyword_color();
}

void LuaCodeEdit::set_comment_color(const Color &color) {
	lua_highlighter->set_comment_color(color);
}

Color LuaCodeEdit::get_comment_color() const {
	return lua_highlighter->get_comment_color();
}

void LuaCodeEdit::set_string_color(const Color &color) {
	lua_highlighter->set_string_color(color);
}

Color LuaCodeEdit::get_string_color() const {
	return lua_highlighter->get_string_color();
}

void LuaCodeEdit::set_lua_language_server_path(String luals_path) {
	this->luals_path = luals_path;
}

String LuaCodeEdit::get_lua_language_server_path() const {
	return luals_path;
}