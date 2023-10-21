#ifndef LUAHIGHLIGHTER_H
#define LUAHIGHLIGHTER_H

#include <godot_cpp/classes/syntax_highlighter.hpp>
#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

// This is heavily based on the build in CodeHighlighter class.
class LuaHighlighter : public SyntaxHighlighter {
	GDCLASS(LuaHighlighter, SyntaxHighlighter);

protected:
	static void _bind_methods();

public:
	Dictionary get_line_syntax_highlighting(int p_line);

	virtual Dictionary _get_line_syntax_highlighting(int p_line) const override;

	virtual void _clear_highlighting_cache() override;
	virtual void _update_cache() override;

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

	void set_true_nil_false_color(const Color &color);
	Color get_true_nil_false_color() const;

	void set_comment_color(const Color &color);
	Color get_comment_color() const;

	void set_string_color(const Color &color);
	Color get_string_color() const;

private:
	struct ColorRegion {
		String start_key;
		String end_key;
		bool comment;
		bool line_only;
	};

	const Vector<ColorRegion> color_regions = {
		{ "\"", "\"", false, true },
		{ "'", "'", false, true },
		{ "[[", "]]", false, true },

		{ "--", "", true, true },
		{ "--[[", "--]]", true, false }
	};

	const Vector<String> keywords = {
		"do", "else", "elseif", "end", "for", "if", "in", "local",
		"not", "or", "repeat", "return", "then", "until", "while",
		"function", "and", "break", "true", "nil", "false"
	};

	HashMap<int, int> color_region_cache;

	Color font_color;
	Color member_color = Color::hex(0xFFFFFFFF);
	Color function_color = Color::hex(0x0578BCFF);
	Color symbol_color = Color::hex(0xFFFFFFFF);
	Color number_color = Color::hex(0xFE8946FF);
	Color keyword_color = Color::hex(0x0E94E6FF);
	Color comment_color = Color::hex(0x818181FF);
	Color string_color = Color::hex(0x97D82DFF);

	TextEdit *text_edit = nullptr;
};

#endif