#pragma once

#include "Font.h"
#include "../render/UnitRenderable.h"
#include "edit/Color.h"
#include "../widgets/Widgets.h"

struct TextRender : public WP_IndexedRenderable // LATER rename WP_ to W_ ?
{
	Shader shader;
	Font* font = nullptr;

private:
	UTF::String text;

public:
	TextRender(Font* font, const UTF::String& text);
	TextRender(Font* font, UTF::String&& text);
	TextRender(const TextRender&) = delete;
	TextRender(TextRender&&) noexcept = delete;

	enum class HorizontalAlign : char
	{
		LEFT,
		RIGHT,
		CENTER,
		JUSTIFY,
		JUSTIFY_GLYPHS
	};

	enum class VerticalAlign : char
	{
		TOP,
		MIDDLE,
		BOTTOM,
		JUSTIFY,
		JUSTIFY_LINEBREAKS
	};

	struct Format
	{
		float line_spacing_mult = 1.0f;
		float num_spaces_in_tab = 4;
		HorizontalAlign horizontal_align = HorizontalAlign::LEFT;
		VerticalAlign vertical_align = VerticalAlign::TOP;
		// LATER underline/strikethrough/etc.
		// background color/drop-shadow/reflection/etc.
		int min_width = 0, min_height = 0;
	};

	struct LineInfo
	{
		int width = 0;
		int num_spaces = 0;
		int num_tabs = 0;
	};

	struct LineFormattingInfo
	{
		int add_x = 0;
		float mul_x = 1.0f;
		float space_mul_x = 1.0f;
	};

	struct PageFormattingInfo
	{
		int add_y = 0;
		float mul_y = 1.0f;
		float linebreak_mul_y = 1.0f;
	};

	struct Bounds
	{
		int inner_width = 0;
		int inner_height = 0;
		int lowest_baseline = 0;
		int top_ribbon = 0;
		int bottom_ribbon = 0;
		int num_linebreaks = 0;
		std::vector<LineInfo> lines;
	};

	Format format = {};
	RGBA fore_color = RGBA(1.0f, 1.0f, 1.0f, 1.0f);

	void draw() const;

	void set_text(const UTF::String& text_) { text = text_; update_text(); }
	void set_text(UTF::String&& text_) { text = std::move(text_); update_text(); }

	void send_vp(const glm::mat3 vp) const;

	Bounds get_bounds() const { return bounds; }
	int outer_width() const { return std::max(bounds.inner_width, format.min_width); }
	int outer_height() const { return std::max(bounds.inner_height, format.min_height); }

private:
	void update_text();
	size_t num_printable_glyphs = 0;
	void build_layout();
public:
	void setup_renderable();
private:
	void add_glyph_to_ir(const Font::Glyph& glyph, int x, int y, size_t quad_index);
	void format_line(size_t line, LineFormattingInfo& line_formatting) const;
	PageFormattingInfo format_page() const;

	Bounds bounds;

	struct FormattingData
	{
		int row = 0;
		int x = 0, y = 0;
		Codepoint prev_codepoint = 0;
		int startX = 0, line_height = 0;
		LineFormattingInfo line = {};
		PageFormattingInfo page = {};

		void setup(const TextRender& text_render);
		void next_line(const TextRender& text_render);
		void advance_x(int amount) { x += amount; }
		void advance_x(int amount, Codepoint codepoint) { x += amount; prev_codepoint = codepoint; }
		void advance_x(float amount) { x += static_cast<int>(roundf(amount)); }
		void advance_x(float amount, Codepoint codepoint) { x += static_cast<int>(roundf(amount)); prev_codepoint = codepoint; }
		void kerning_advance_x(const TextRender& text_render, const Font::Glyph& glyph, Codepoint codepoint);
	} formatting;

	struct BoundsFormattingData
	{
		int x = 0, y = 0;
		Codepoint prev_codepoint = 0;
		int line_height = 0;
		int min_ch_y0 = 0, max_ch_y1 = 0;
		bool first_line = true;
		LineInfo line_info = {};
		LineFormattingInfo line_formatting = {};

		void setup(const TextRender& text_render);
		void next_line(TextRender& text_render);
		void last_line(TextRender& text_render);
		void advance_x(int amount) { x += amount; }
		void advance_x(int amount, Codepoint codepoint) { x += amount; prev_codepoint = codepoint; }
		void advance_x(float amount) { x += static_cast<int>(roundf(amount)); }
		void advance_x(float amount, Codepoint codepoint) { x += static_cast<int>(roundf(amount)); prev_codepoint = codepoint; }
		void kerning_advance_x(const TextRender& text_render, const Font::Glyph& glyph, Codepoint codepoint);
		void update_min_ch_y0(const Font::Glyph& glyph);
		void update_max_ch_y1(const Font::Glyph& glyph);
	} bounds_formatting;
};

inline TextRender& tr_wget(Widget& w, size_t i)
{
	return *w.get<TextRender>(i);
}

inline const TextRender& tr_wget(const Widget& w, size_t i)
{
	return *w.get<TextRender>(i);
}
