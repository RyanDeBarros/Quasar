#include "TextRender.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "../render/Uniforms.h"

Shader* TextRender::shader;

void TextRender::load_shader()
{
	shader = new Shader(FileSystem::shader_path("text.vert"), FileSystem::shader_path("text.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", "32" } }); // TODO actually query the number of texture slots supported.
}

void TextRender::invalidate_shader()
{
	delete shader;
	shader = nullptr;
}

TextRender::TextRender(Font* font, const UTF::String& text)
	: font(font), WP_IndexedRenderable(*shader)
{
	set_text(text);
	held.pivot.x = 0;
	held.pivot.y = 1;
}

TextRender::TextRender(Font* font, UTF::String&& text)
	: font(font), WP_IndexedRenderable(*shader)
{
	set_text(std::move(text));
	held.pivot.x = 0;
	held.pivot.y = 1;
}

void TextRender::draw() const
{
	bind_shader(*shader);
	Uniforms::send_4(*shader, "u_ForeColor", fore_color.as_vec());
	bind_texture(font->common_texture.tid, 0); // TODO bind all textures - draw ir in separate steps when exceeding MAX_TEXTURES.
	ir->draw();
}

void TextRender::update_text(FlatTransform parent)
{
	ir->varr.clear();
	ir->iarr.clear();
	build_layout();
	setup_renderable(parent);
}

void TextRender::build_layout()
{
	num_printable_glyphs = 0;
	bounds = {};
	bounds_formatting.setup(*this);
	auto iter1 = text.begin();
	while (iter1)
	{
		Codepoint codepoint = iter1.advance();

		if (codepoint == ' ')
		{
			bounds_formatting.advance_x(font->space_width, 0);
			++bounds_formatting.line_info.num_spaces;
		}
		else if (codepoint == '\t')
		{
			bounds_formatting.advance_x(font->space_width * format.num_spaces_in_tab, 0);
			++bounds_formatting.line_info.num_tabs;
		}
		else if (carriage_return_2(codepoint, iter1 ? iter1.codepoint() : 0))
		{
			bounds_formatting.next_line(*this);
			++iter1;
		}
		else if (carriage_return_1(codepoint))
		{
			bounds_formatting.next_line(*this);
		}
		else if (font->cache(codepoint))
		{
			const Font::Glyph& glyph = font->glyphs[codepoint];
			bounds_formatting.kerning_advance_x(*this, glyph, codepoint);
			bounds_formatting.advance_x(glyph.advance_width * font->scale, codepoint);
			bounds_formatting.update_min_ch_y0(glyph);
			bounds_formatting.update_max_ch_y1(glyph);
			++num_printable_glyphs;
		}
	}
	bounds_formatting.last_line(*this);
}

void TextRender::setup_renderable(FlatTransform parent)
{
	ir->fill_iarr_with_quads(num_printable_glyphs);
	ir->push_back_vertices(num_printable_glyphs * 4);
	size_t quad_index = 0;
	formatting.setup(*this);
	auto iter2 = text.begin(); // TODO define copy constructor for iter
	while (iter2)
	{
		Codepoint codepoint = iter2.advance();

		if (codepoint == ' ')
			formatting.advance_x(font->space_width * formatting.line.space_mul_x, 0);
		else if (codepoint == '\t')
			formatting.advance_x(font->space_width * format.num_spaces_in_tab * formatting.line.space_mul_x, 0);
		else if (carriage_return_2(codepoint, iter2 ? iter2.codepoint() : 0))
		{
			formatting.next_line(*this);
			++iter2;
		}
		else if (carriage_return_1(codepoint))
			formatting.next_line(*this);
		else if (font->cache(codepoint))
		{
			const Font::Glyph& glyph = font->glyphs[codepoint];
			formatting.kerning_advance_x(*this, glyph, codepoint);
			add_glyph_to_ir(glyph, formatting.x, formatting.y, quad_index++, parent);
			formatting.advance_x(glyph.advance_width * font->scale * formatting.line.mul_x, codepoint);
		}
	}
	ir->send_both_buffers_resized();
}

void TextRender::add_glyph_to_ir(const Font::Glyph& glyph, int x, int y, size_t quad_index, FlatTransform parent)
{
	FlatTransform local{ { float(x), float(y - glyph.ch_y0) }, { float(glyph.width), -float(glyph.height) } };
	local = local.relative_to(held.transform.relative_to(parent));
	float left = local.position.x - 0.5f * local.scale.x;
	float right = local.position.x + 0.5f * local.scale.x;
	float bottom = local.position.y - 0.5f * local.scale.y;
	float top = local.position.y + 0.5f * local.scale.y;

	ir->set_attribute_single_vertex(quad_index * 4 + 0, 0, glm::value_ptr(glm::vec2{ left, bottom }));
	ir->set_attribute_single_vertex(quad_index * 4 + 1, 0, glm::value_ptr(glm::vec2{ right, bottom }));
	ir->set_attribute_single_vertex(quad_index * 4 + 2, 0, glm::value_ptr(glm::vec2{ right, top }));
	ir->set_attribute_single_vertex(quad_index * 4 + 3, 0, glm::value_ptr(glm::vec2{ left, top }));

	//float tex_slot = ; // TODO tex_slot and binding correct texture slot. bind common texture to 0, and then increase until flushing at MAX_TEXTURES = 32 (for now).
	float tex_slot = 0;
	ir->set_attribute_single_vertex(quad_index * 4 + 0, 1, &tex_slot);
	ir->set_attribute_single_vertex(quad_index * 4 + 1, 1, &tex_slot);
	ir->set_attribute_single_vertex(quad_index * 4 + 2, 1, &tex_slot);
	ir->set_attribute_single_vertex(quad_index * 4 + 3, 1, &tex_slot);

	auto uvs = font->uvs(glyph);
	ir->set_attribute_single_vertex(quad_index * 4 + 0, 2, glm::value_ptr(glm::vec2{ uvs.x1, uvs.y1 }));
	ir->set_attribute_single_vertex(quad_index * 4 + 1, 2, glm::value_ptr(glm::vec2{ uvs.x2, uvs.y1 }));
	ir->set_attribute_single_vertex(quad_index * 4 + 2, 2, glm::value_ptr(glm::vec2{ uvs.x2, uvs.y2 }));
	ir->set_attribute_single_vertex(quad_index * 4 + 3, 2, glm::value_ptr(glm::vec2{ uvs.x1, uvs.y2 }));
}

void TextRender::format_line(size_t line, LineFormattingInfo& line_formatting) const
{
	line_formatting = {};

	if (format.horizontal_align == HorizontalAlign::RIGHT)
		line_formatting.add_x = outer_width() - bounds.lines[line].width;
	else if (format.horizontal_align == HorizontalAlign::CENTER)
		line_formatting.add_x = static_cast<int>(0.5f * (outer_width() - bounds.lines[line].width));
	else if (format.horizontal_align == HorizontalAlign::JUSTIFY_GLYPHS)
	{
		if (bounds.lines[line].width)
			line_formatting.mul_x = static_cast<float>(outer_width()) / bounds.lines[line].width;
		line_formatting.space_mul_x = line_formatting.mul_x;
	}
	else if (format.horizontal_align == HorizontalAlign::JUSTIFY)
	{
		float num_spaces = bounds.lines[line].num_spaces + format.num_spaces_in_tab * bounds.lines[line].num_tabs;
		if (num_spaces > 0.0f)
			line_formatting.space_mul_x += static_cast<float>(outer_width() - bounds.lines[line].width) / (num_spaces * font->space_width);
	}
}

TextRender::PageFormattingInfo TextRender::format_page() const
{
	PageFormattingInfo page_formatting;

	if (format.vertical_align == VerticalAlign::BOTTOM)
		page_formatting.add_y = outer_height() - bounds.inner_height;
	else if (format.vertical_align == VerticalAlign::MIDDLE)
		page_formatting.add_y = static_cast<int>(0.5f * (outer_height() - bounds.inner_height));
	else if (format.vertical_align == VerticalAlign::JUSTIFY)
	{
		int line_height = font->line_height(format.line_spacing_mult);
		if (bounds.inner_height != line_height)
			page_formatting.mul_y = static_cast<float>(outer_height() - line_height) / (bounds.inner_height - line_height);
		page_formatting.linebreak_mul_y = page_formatting.mul_y;
	}
	else if (format.vertical_align == VerticalAlign::JUSTIFY_LINEBREAKS)
	{
		int line_height = font->line_height(format.line_spacing_mult);
		if (bounds.num_linebreaks * line_height != 0)
			page_formatting.linebreak_mul_y += static_cast<float>(outer_height() - bounds.inner_height) / (bounds.num_linebreaks * line_height);
	}

	return page_formatting;
}

void TextRender::FormattingData::setup(const TextRender& text_render)
{
	line_height = text_render.font->line_height(text_render.format.line_spacing_mult);
	startX = static_cast<int>(-text_render.held.pivot.x * text_render.outer_width());
	row = 0;
	prev_codepoint = 0;
	text_render.format_line(row++, line);
	x = startX + line.add_x;
	page = text_render.format_page();
	y = static_cast<int>(roundf(-text_render.font->baseline + (1.0f - text_render.held.pivot.y) * text_render.outer_height())) + text_render.bounds.top_ribbon - page.add_y;
}

void TextRender::FormattingData::next_line(const TextRender& text_render)
{
	text_render.format_line(row++, line);
	if (x == startX + line.add_x)
		y -= static_cast<int>(roundf(line_height * page.linebreak_mul_y));
	else
		y -= static_cast<int>(roundf(line_height * page.mul_y));
	x = startX + line.add_x;
	prev_codepoint = 0;
}

void TextRender::FormattingData::kerning_advance_x(const TextRender& text_render, const Font::Glyph& glyph, Codepoint codepoint)
{
	x += text_render.font->kerning_of(prev_codepoint, codepoint, text_render.font->glyphs[prev_codepoint].index, glyph.index, line.mul_x);
}

void TextRender::BoundsFormattingData::setup(const TextRender& text_render)
{
	line_height = text_render.font->line_height(text_render.format.line_spacing_mult);
	x = 0;
	y = -text_render.font->baseline;
	min_ch_y0 = 0;
	max_ch_y1 = INT_MIN;
	line_info = {};
	line_formatting = {};
	first_line = true;
	prev_codepoint = 0;
}

void TextRender::BoundsFormattingData::next_line(TextRender& text_render)
{
	Bounds& bounds = text_render.bounds;
	if (x > bounds.inner_width)
		bounds.inner_width = x;
	line_info.width = x;
	bounds.lines.push_back(line_info);
	line_info = {};
	if (x == 0)
		++bounds.num_linebreaks;
	x = 0;
	y -= line_height;
	first_line = false;
	max_ch_y1 = INT_MIN;
	prev_codepoint = 0;
}

void TextRender::BoundsFormattingData::last_line(TextRender& text_render)
{
	Bounds& bounds = text_render.bounds;
	Font* font = text_render.font;
	if (x > bounds.inner_width)
		bounds.inner_width = x;
	line_info.width = x;
	bounds.lines.push_back(line_info);
	line_info = {};
	if (x == 0)
		++bounds.num_linebreaks;
	bounds.lowest_baseline = -y;
	bounds.top_ribbon = static_cast<int>(font->ascent * font->scale + min_ch_y0);
	if (max_ch_y1 == INT_MAX)
		max_ch_y1 = 0;
	bounds.bottom_ribbon = static_cast<int>(max_ch_y1 - font->descent * font->scale);
	bounds.inner_height = static_cast<int>(bounds.lowest_baseline - font->descent * font->scale - bounds.top_ribbon);
}

void TextRender::BoundsFormattingData::kerning_advance_x(const TextRender& text_render, const Font::Glyph& glyph, Codepoint codepoint)
{
	x += text_render.font->kerning_of(prev_codepoint, codepoint, text_render.font->glyphs[prev_codepoint].index, glyph.index);
}

void TextRender::BoundsFormattingData::update_min_ch_y0(const Font::Glyph& glyph)
{
	if (first_line && glyph.ch_y0 < min_ch_y0)
		min_ch_y0 = glyph.ch_y0;
}

void TextRender::BoundsFormattingData::update_max_ch_y1(const Font::Glyph& glyph)
{
	if (glyph.ch_y0 + glyph.height > max_ch_y1)
		max_ch_y1 = glyph.ch_y0 + glyph.height;
}
