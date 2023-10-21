#include "luaHighlighter.h"

void LuaHighlighter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_member_variable_color", "color"), &LuaHighlighter::set_member_variable_color);
	ClassDB::bind_method(D_METHOD("get_member_variable_color"), &LuaHighlighter::get_member_variable_color);

	ClassDB::bind_method(D_METHOD("set_function_color", "color"), &LuaHighlighter::set_function_color);
	ClassDB::bind_method(D_METHOD("get_function_color"), &LuaHighlighter::get_function_color);

	ClassDB::bind_method(D_METHOD("set_symbol_color", "color"), &LuaHighlighter::set_symbol_color);
	ClassDB::bind_method(D_METHOD("get_symbol_color"), &LuaHighlighter::get_symbol_color);

	ClassDB::bind_method(D_METHOD("set_number_color", "color"), &LuaHighlighter::set_number_color);
	ClassDB::bind_method(D_METHOD("get_number_color"), &LuaHighlighter::get_number_color);

	ClassDB::bind_method(D_METHOD("set_keyword_color", "color"), &LuaHighlighter::set_keyword_color);
	ClassDB::bind_method(D_METHOD("get_keyword_color"), &LuaHighlighter::get_keyword_color);

	ClassDB::bind_method(D_METHOD("set_comment_color", "color"), &LuaHighlighter::set_comment_color);
	ClassDB::bind_method(D_METHOD("get_comment_color"), &LuaHighlighter::get_comment_color);

	ClassDB::bind_method(D_METHOD("set_string_color", "color"), &LuaHighlighter::set_string_color);
	ClassDB::bind_method(D_METHOD("get_string_color"), &LuaHighlighter::get_string_color);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "member_variable_color"), "set_member_variable_color", "get_member_variable_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "function_color"), "set_function_color", "get_function_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "symbol_color"), "set_symbol_color", "get_symbol_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "number_color"), "set_number_color", "get_number_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "keyword_color"), "set_keyword_color", "get_keyword_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "comment_color"), "set_comment_color", "get_comment_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "string_color"), "set_string_color", "get_string_color");
}

Dictionary LuaHighlighter::_get_line_syntax_highlighting(int p_line) const {
	return const_cast<LuaHighlighter *>(this)->get_line_syntax_highlighting(p_line);
}

Dictionary LuaHighlighter::get_line_syntax_highlighting(int p_line) {
	Dictionary color_map;

	bool prev_is_char = false;
	bool prev_is_number = false;
	bool in_keyword = false;
	bool in_word = false;
	bool in_function_name = false;
	bool in_member_variable = false;
	bool is_hex_notation = false;
	Color color;

	color_region_cache[p_line] = -1;
	int in_region = -1;
	if (p_line != 0) {
		int prev_region_line = p_line - 1;
		while (prev_region_line > 0 && !color_region_cache.has(prev_region_line)) {
			prev_region_line--;
		}
		for (int i = prev_region_line; i < p_line - 1; i++) {
			get_line_syntax_highlighting(i);
		}
		if (!color_region_cache.has(p_line - 1)) {
			get_line_syntax_highlighting(p_line - 1);
		}
		in_region = color_region_cache[p_line - 1];
	}

	const String &str = text_edit->get_line(p_line);
	const int line_length = str.length();
	Color prev_color;

	if (in_region != -1 && str.length() == 0) {
		color_region_cache[p_line] = in_region;
	}
	for (int j = 0; j < line_length; j++) {
		Dictionary highlighter_info;

		color = font_color;
		bool is_char = !is_symbol(str[j]);
		bool is_a_symbol = is_symbol(str[j]);
		bool is_number = is_digit(str[j]);

		/* color regions */
		if (is_a_symbol || in_region != -1) {
			int from = j;

			if (in_region == -1) {
				for (; from < line_length; from++) {
					if (str[from] == '\\') {
						from++;
						continue;
					}
					break;
				}
			}

			if (from != line_length) {
				/* check if we are in entering a region */
				if (in_region == -1) {
					for (int c = 0; c < color_regions.size(); c++) {
						/* check there is enough room */
						int chars_left = line_length - from;
						int start_key_length = color_regions[c].start_key.length();
						int end_key_length = color_regions[c].end_key.length();
						if (chars_left < start_key_length) {
							continue;
						}

						/* search the line */
						bool match = true;
						const char32_t *start_key = color_regions[c].start_key.utf32().get_data();
						for (int k = 0; k < start_key_length; k++) {
							if (start_key[k] != str[from + k]) {
								match = false;
								break;
							}
						}
						if (!match) {
							continue;
						}
						in_region = c;
						from += start_key_length;

						/* check if it's the whole line */
						if (end_key_length == 0 || from + end_key_length > line_length) {
							if (from + end_key_length > line_length && (color_regions[in_region].start_key == "\"" || color_regions[in_region].start_key == "\'")) {
								// If it's key length and there is a '\', dont skip to highlight esc chars.
								if (str.find("\\", from) >= 0) {
									break;
								}
							}
							prev_color = color_regions[in_region].comment ? comment_color : string_color;
							highlighter_info["color"] = color_regions[c].comment ? comment_color : string_color;
							color_map[j] = highlighter_info;

							j = line_length;
							if (!color_regions[c].line_only) {
								color_region_cache[p_line] = c;
							}
						}
						break;
					}

					if (j == line_length) {
						continue;
					}
				}

				/* if we are in one find the end key */
				if (in_region != -1) {
					bool is_string = (color_regions[in_region].start_key == "\"" || color_regions[in_region].start_key == "\'");

					Color region_color = color_regions[in_region].comment ? comment_color : string_color;
					prev_color = region_color;
					highlighter_info["color"] = region_color;
					color_map[j] = highlighter_info;

					/* search the line */
					int region_end_index = -1;
					int end_key_length = color_regions[in_region].end_key.length();
					const char32_t *end_key = color_regions[in_region].end_key.utf32().get_data();
					for (; from < line_length; from++) {
						if (line_length - from < end_key_length) {
							// Don't break if '\' to highlight esc chars.
							if (!is_string || str.find("\\", from) < 0) {
								break;
							}
						}

						if (!is_symbol(str[from])) {
							continue;
						}

						if (str[from] == '\\') {
							if (is_string) {
								Dictionary escape_char_highlighter_info;
								escape_char_highlighter_info["color"] = symbol_color;
								color_map[from] = escape_char_highlighter_info;
							}

							from++;

							if (is_string) {
								Dictionary region_continue_highlighter_info;
								prev_color = region_color;
								region_continue_highlighter_info["color"] = region_color;
								color_map[from + 1] = region_continue_highlighter_info;
							}
							continue;
						}

						region_end_index = from;
						for (int k = 0; k < end_key_length; k++) {
							if (end_key[k] != str[from + k]) {
								region_end_index = -1;
								break;
							}
						}

						if (region_end_index != -1) {
							break;
						}
					}

					j = from + (end_key_length - 1);
					if (region_end_index == -1) {
						color_region_cache[p_line] = in_region;
					}

					in_region = -1;
					prev_is_char = false;
					prev_is_number = false;
					continue;
				}
			}
		}

		// Allow ABCDEF in hex notation.
		if (is_hex_notation && (is_hex_digit(str[j]) || is_number)) {
			is_number = true;
		} else {
			is_hex_notation = false;
		}

		// Check for dot or underscore or 'x' for hex notation in floating point number or 'e' for scientific notation.
		if ((str[j] == '.' || str[j] == 'x' || str[j] == '_' || str[j] == 'f' || str[j] == 'e') && !in_word && prev_is_number && !is_number) {
			is_number = true;
			is_a_symbol = false;
			is_char = false;

			if (str[j] == 'x' && str[j - 1] == '0') {
				is_hex_notation = true;
			}
		}

		if (!in_word && (is_ascii_char(str[j]) || is_underscore(str[j])) && !is_number) {
			in_word = true;
		}

		if ((in_keyword || in_word) && !is_hex_notation) {
			is_number = false;
		}

		if (is_a_symbol && str[j] != '.' && in_word) {
			in_word = false;
		}

		if (!is_char) {
			in_keyword = false;
		}

		if (!in_keyword && is_char && !prev_is_char) {
			int to = j;
			while (to < line_length && !is_symbol(str[to])) {
				to++;
			}

			String word = str.substr(j, to - j);
			if (keywords.has(word)) {
				in_keyword = true;
			}
		}

		if (!in_function_name && in_word && !in_keyword) {
			int k = j;
			while (k < line_length && !is_symbol(str[k]) && str[k] != '\t' && str[k] != ' ') {
				k++;
			}

			// Check for space between name and bracket.
			while (k < line_length && (str[k] == '\t' || str[k] == ' ')) {
				k++;
			}

			if (str[k] == '(') {
				in_function_name = true;
			}
		}

		if (!in_function_name && !in_member_variable && !in_keyword && !is_number && in_word) {
			int k = j;
			while (k > 0 && !is_symbol(str[k]) && str[k] != '\t' && str[k] != ' ') {
				k--;
			}

			if (str[k] == '.') {
				in_member_variable = true;
			}
		}

		if (is_a_symbol) {
			in_function_name = false;
			in_member_variable = false;
		}

		if (in_keyword) {
			color = keyword_color;
		} else if (in_member_variable) {
			color = member_color;
		} else if (in_function_name) {
			color = function_color;
		} else if (is_a_symbol) {
			color = symbol_color;
		} else if (is_number) {
			color = number_color;
		}

		prev_is_char = is_char;
		prev_is_number = is_number;

		if (color != prev_color) {
			prev_color = color;
			highlighter_info["color"] = color;
			color_map[j] = highlighter_info;
		}
	}

	return color_map;
}

void LuaHighlighter::_clear_highlighting_cache() {
	color_region_cache.clear();
}

void LuaHighlighter::_update_cache() {
	text_edit = get_text_edit();
	font_color = text_edit->get_theme_color("font_color", "TextEdit");
}

void LuaHighlighter::set_member_variable_color(const Color &color) {
	member_color = color;
	clear_highlighting_cache();
}

Color LuaHighlighter::get_member_variable_color() const {
	return member_color;
}

void LuaHighlighter::set_function_color(const Color &color) {
	function_color = color;
	clear_highlighting_cache();
}

Color LuaHighlighter::get_function_color() const {
	return function_color;
}

void LuaHighlighter::set_symbol_color(const Color &color) {
	symbol_color = color;
	clear_highlighting_cache();
}

Color LuaHighlighter::get_symbol_color() const {
	return symbol_color;
}

void LuaHighlighter::set_number_color(const Color &color) {
	number_color = color;
	clear_highlighting_cache();
}

Color LuaHighlighter::get_number_color() const {
	return number_color;
}

void LuaHighlighter::set_keyword_color(const Color &color) {
	keyword_color = color;
	clear_highlighting_cache();
}

Color LuaHighlighter::get_keyword_color() const {
	return keyword_color;
}

void LuaHighlighter::set_comment_color(const Color &color) {
	comment_color = color;
	clear_highlighting_cache();
}

Color LuaHighlighter::get_comment_color() const {
	return comment_color;
}

void LuaHighlighter::set_string_color(const Color &color) {
	string_color = color;
	clear_highlighting_cache();
}

Color LuaHighlighter::get_string_color() const {
	return string_color;
}