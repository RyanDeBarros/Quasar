#include "Font.h"

#include "variety/IO.h"

static bool read_kern_part(const std::string& p, int& k)
{
	if (p[0] == '\\')
	{
		if (p.size() == 1)
			return false;
		if (p[1] == 'x')
			k = std::stoi(p.substr(2, p.size() - 2), nullptr, 16);
		else if (p[1] == '\\')
			k = '\\';
		else if (p[1] == '\'')
			k = '\'';
		else if (p[1] == '"')
			k = '\"';
		else if (p[1] == '?')
			k = '\?';
		else if (p[1] == 'a')
			k = '\a';
		else if (p[1] == 'b')
			k = '\b';
		else if (p[1] == 'f')
			k = '\f';
		else if (p[1] == 'n')
			k = '\n';
		else if (p[1] == 'r')
			k = '\r';
		else if (p[1] == 't')
			k = '\t';
		else if (p[1] == 'v')
			k = '\v';
		else if (p[1] == '0')
			k = '\0';
		else
			return false;
	}
	else
		k = p[0];
	return true;
}

static bool parse_kerning_line(const std::string& p0, const std::string& p1,
	const std::string& p2, std::pair<std::pair<Codepoint, Codepoint>, int>& insert)
{
	if (!read_kern_part(p0, insert.first.first))
		return false;
	if (!read_kern_part(p1, insert.first.second))
		return false;
	insert.second = std::stoi(p2);
	return true;
}

static void parse_kerning(const FilePath& filepath, Kerning::Map& kerning)
{
	std::string content;
	if (IO.read_file(filepath, content))
	{
		char part = 0;
		std::string p0, p1, p2;
		for (auto iter = content.begin(); iter != content.end(); ++iter)
		{
			if (carriage_return_1(*iter))
			{
				std::pair<std::pair<Codepoint, Codepoint>, int> insert;
				if (parse_kerning_line(p0, p1, p2, insert))
					kerning.insert_or_assign(insert.first, insert.second);
				p0.clear();
				p1.clear();
				p2.clear();
				part = 0;
			}
			else if (carriage_return_2(*iter, iter + 1 != content.end() ? *(iter + 1) : 0))
			{
				std::pair<std::pair<Codepoint, Codepoint>, int> insert;
				if (parse_kerning_line(p0, p1, p2, insert))
					kerning.insert_or_assign(insert.first, insert.second);
				p0.clear();
				p1.clear();
				p2.clear();
				part = 0;
				++iter;
			}
			else if (part == 0)
			{
				if (*iter == ' ' || *iter == '\t')
				{
					if (!p0.empty())
						++part;
				}
				else
					p0.push_back(*iter);
			}
			else if (part == 1)
			{
				if (*iter == ' ' || *iter == '\t')
				{
					if (!p1.empty())
						++part;
				}
				else
					p1.push_back(*iter);
			}
			else if (*iter != ' ' && *iter != '\t')
			{
				p2.push_back(*iter);
			}
		}
		if (part == 2)
		{
			std::pair<std::pair<Codepoint, Codepoint>, int> insert;
			if (parse_kerning_line(p0, p1, p2, insert))
				kerning.insert_or_assign(insert.first, insert.second);
		}
	}
}

Kerning::Kerning(const FilePath& filepath)
{
	if (!filepath.empty())
		parse_kerning(filepath, map);
}

Font::Glyph::Glyph(Font* font, int index, float scale, size_t buffer_pos)
	: index(index), texture(nullptr), buffer_pos(buffer_pos)
{
	stbtt_GetGlyphHMetrics(&font->font_info, index, &advance_width, &left_bearing);
	int ch_x0, ch_x1, ch_y1;
	stbtt_GetGlyphBitmapBox(&font->font_info, index, scale, scale, &ch_x0, &ch_y0, &ch_x1, &ch_y1);
	width = ch_x1 - ch_x0;
	height = ch_y1 - ch_y0;
}

void Font::Glyph::render_on_bitmap_shared(const Font& font, const Buffer& buffer, int left_padding, int right_padding, int bottom_padding, int top_padding)
{
	unsigned char* temp = new unsigned char[width * height];
	stbtt_MakeGlyphBitmap(&font.font_info, temp, width, height, width, font.scale, font.scale, index);
	for (size_t row = 0; row < bottom_padding; ++row)
		memset(buffer.pixels + row * buffer.width, 0, left_padding + width + right_padding);
	for (size_t row = bottom_padding; row < bottom_padding + height; ++row)
	{
		memset(buffer.pixels + row * buffer.width, 0, left_padding);
		memcpy(buffer.pixels + row * buffer.width + left_padding, temp + (row - bottom_padding) * width, width);
		memset(buffer.pixels + row * buffer.width + left_padding + width, 0, right_padding);

	}
	for (size_t row = height + bottom_padding; row < buffer.height; ++row)
		memset(buffer.pixels + row * buffer.width, 0, left_padding + width + right_padding);
	delete[] temp;
	location = buffer.pixels;
}

void Font::Glyph::render_on_bitmap_unique(const Font& font, const Buffer& buffer)
{
	stbtt_MakeGlyphBitmap(&font.font_info, buffer.pixels, width, height, width, font.scale, font.scale, index);
	location = buffer.pixels;
}

Font::Font(const FilePath& filepath, float font_size, UTF::String common_buffer, TextureParams texture_params, const std::shared_ptr<Kerning>& kerning)
	: font_size(font_size), font_info{}, texture_params(texture_params), kerning(kerning)
{
	common_texture.buf.chpp = 1;

	unsigned char* font_file = nullptr;
	size_t font_filesize;
	if (!IO.read_file_uc(filepath, font_file, font_filesize))
	{
		LOG << LOG.fatal << LOG.start << "Cannot load font file" << LOG.endl;
		return;
	}
	if (!stbtt_InitFont(&font_info, font_file, 0))
	{
		LOG << LOG.fatal << LOG.start << "Cannot init font" << LOG.endl;
		return;
	}

	scale = stbtt_ScaleForPixelHeight(&font_info, font_size);
	stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &linegap);
	baseline = static_cast<int>(roundf(ascent * scale));

	std::vector<Codepoint> codepoints;
	auto iter = common_buffer.begin();
	while (iter)
	{
		int codepoint = iter.advance();
		if (glyphs.find(codepoint) != glyphs.end())
			continue;
		int gIndex = stbtt_FindGlyphIndex(&font_info, codepoint);
		if (!gIndex) // LATER add warning/info logs throughout font/text code
			continue;
		Glyph glyph(this, gIndex, scale, common_texture.buf.width);
		common_texture.buf.width += glyph.width + size_t(2);
		if (glyph.height > common_texture.buf.height)
			common_texture.buf.height = glyph.height;
		glyphs.insert({ codepoint, std::move(glyph) });
		codepoints.push_back(codepoint);
	}
	common_texture.buf.height += 2;
	if (common_texture.buf.width > 0)
	{
		common_texture.buf.pxnew();
		Buffer offset = common_texture.buf;
		for (Codepoint codepoint : codepoints)
		{
			Font::Glyph& glyph = glyphs[codepoint];
			offset.pixels = common_texture.buf.pixels + glyph.buffer_pos;
			glyph.render_on_bitmap_shared(*this, offset, 1, 1, 1, 1);
		}
		common_texture.buf = common_texture.buf;
		common_texture.gen_texture(texture_params);
		for (Codepoint codepoint : codepoints)
			glyphs[codepoint].texture = &common_texture;
	}
	auto space = glyphs.find(' ');
	if (space == glyphs.end())
	{
		int space_advance_width, space_left_bearing;
		stbtt_GetCodepointHMetrics(&font_info, ' ', &space_advance_width, &space_left_bearing);
		space_width = static_cast<int>(roundf(space_advance_width * scale));
	}
	else
		space_width = space->second.width;
}

Font::~Font()
{
	for (auto t : cached_textures)
		delete t;
}

bool Font::cache(Codepoint codepoint)
{
	if (glyphs.find(codepoint) != glyphs.end())
		return true;
	int index = stbtt_FindGlyphIndex(&font_info, codepoint);
	if (!index) return false;

	Font::Glyph glyph(this, index, scale, -1);
	Buffer bmp;
	bmp.width = glyph.width;
	bmp.height = glyph.height;
	bmp.chpp = 1;
	bmp.pxnew();
	glyph.render_on_bitmap_unique(*this, bmp);

	Image* img = new Image();
	img->buf = bmp;
	img->gen_texture();
	glyph.texture = img;
	cached_textures.push_back(img);
	glyphs.insert({ codepoint, std::move(glyph) });
	return true;
}

void Font::cache_all(const Font& other)
{
	for (const auto& [codepoint, glyph] : other.glyphs)
		cache(codepoint);
}

bool Font::supports(Codepoint codepoint) const
{
	if (glyphs.find(codepoint) != glyphs.end())
		return true;
	return stbtt_FindGlyphIndex(&font_info, codepoint) != 0;
}

int Font::kerning_of(Codepoint c1, Codepoint c2, int g1, int g2, float sc) const
{
	if (g1 == 0)
		return 0;
	auto k = kerning->map.find({ c1, c2 });
	if (k != kerning->map.end())
		return static_cast<int>(roundf(k->second * scale * sc));
	else
		return static_cast<int>(roundf(stbtt_GetGlyphKernAdvance(&font_info, g1, g2) * scale * sc));
}

void Font::set_texture_params(TextureParams params)
{
	texture_params = params;
	common_texture.update_texture_params(params);
	for (auto t : cached_textures)
		t->update_texture_params(params);
}

int Font::line_height(float line_spacing) const
{
	return static_cast<int>(roundf((ascent - descent + linegap) * scale * line_spacing));
}

Bounds Font::uvs(const Glyph& glyph) const
{
	Bounds b{};
	if (glyph.buffer_pos != size_t(-1))
	{
		b.x1 = static_cast<float>(glyph.buffer_pos + 1) / common_texture.buf.width;
		b.x2 = static_cast<float>(glyph.buffer_pos + 1 + glyph.width) / common_texture.buf.width;
		b.y1 = 0.0f;
		b.y2 = static_cast<float>(glyph.height) / common_texture.buf.height;
	}
	else
	{
		b.x1 = 0;
		b.x2 = 1;
		b.y1 = 0;
		b.y2 = 1;
	}
	return b;
}

float FontRange::get_font_and_multiplier(float font_size, Font*& font)
{
	if (fonts.empty())
	{
		font = nullptr;
		return 0.0f;
	}
	auto iter = fonts.lower_bound(font_size);
	if (iter == fonts.end())
	{
		auto prev = std::prev(iter);
		font = &prev->second;
		return font_size / prev->first;
	}
	if (iter->first == font_size)
	{
		font = &iter->second;
		return 1.0f;
	}
	if (iter == fonts.begin())
	{
		font = &iter->second;
		return font_size / iter->first;
	}
	auto prev = std::prev(iter);
	if (std::abs(iter->first - font_size) < 1.5f * std::abs(prev->first - font_size))
	{
		font = &iter->second;
		return font_size / iter->first;
	}
	else
	{
		font = &prev->second;
		return font_size / prev->first;
	}
}
