#pragma once

#include <unordered_map>
#include <map>

#include <stb/stb_truetype.h>

#include "edit/Image.h"
#include "variety/UTF.h"
#include "variety/Geometry.h"

typedef int Codepoint;

template<>
struct std::hash<std::pair<Codepoint, Codepoint>>
{
	size_t operator()(std::pair<Codepoint, Codepoint> p) const { return hash<Codepoint>{}(p.first) ^ (hash<Codepoint>{}(p.second) << 1); }
};

namespace Fonts
{

	static constexpr const char8_t* COMMON = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,./<>?;:\'\"\\|[]{}!@#$%^&*()-=_+`~";
	static constexpr const char8_t* ALPHA_NUMERIC = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	static constexpr const char8_t* NUMERIC = u8"0123456789";
	static constexpr const char8_t* ALPHA = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static constexpr const char8_t* ALPHA_LOWERCASE = u8"abcdefghijklmnopqrstuvwxyz";
	static constexpr const char8_t* ALPHA_UPPERCASE = u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
}

struct Kerning
{
	typedef std::unordered_map<std::pair<Codepoint, Codepoint>, int> Map;
	Map map;

	Kerning(const FilePath& filepath);
};

struct Font
{
	struct Glyph
	{
		int index = 0;
		int width = 0, height = 0;
		int ch_y0 = 0;
		int advance_width = 0, left_bearing = 0;
		std::shared_ptr<Image> texture = nullptr;
		size_t buffer_pos = -1;
		unsigned char* location = nullptr;

		Glyph() = default;
		Glyph(Font* font, int index, float scale, size_t buffer_pos);
		Glyph(const Glyph&) = delete;
		Glyph(Glyph&&) noexcept = default;
		Glyph& operator=(Glyph&&) noexcept = default;

		void render_on_bitmap_shared(const Font& font, const Buffer& buffer, int left_padding, int right_padding, int bottom_padding, int top_padding);
		void render_on_bitmap_unique(const Font& font, const Buffer& buffer);
	};

	std::unordered_map<Codepoint, Glyph> glyphs;
	stbtt_fontinfo font_info = {};
	float font_size = 0.0f;
	float scale = 0.0f;
	int ascent = 0, descent = 0, linegap = 0, baseline = 0;
	int space_width = 0;
	TextureParams texture_params = TextureParams::linear;
	std::shared_ptr<Image> common_texture = {};
	std::vector<std::shared_ptr<Image>> cached_textures;
	std::shared_ptr<Kerning> kerning = nullptr;

	Font() = default;
	Font(const FilePath& filepath, float font_size, UTF::String common_buffer = Fonts::COMMON, TextureParams texture_params = TextureParams::linear, const std::shared_ptr<Kerning>& kerning = nullptr);
	Font(const Font&) = delete;
	Font(Font&&) noexcept = default;
	Font& operator=(Font&&) noexcept = default;

	bool cache(Codepoint codepoint);
	void cache_all(const Font& other);
	bool supports(Codepoint codepoint) const;
	int kerning_of(Codepoint c1, Codepoint c2, int g1, int g2, float sc = 1.0f) const;
	void set_texture_params(TextureParams params);
	int line_height(float line_spacing = 1.0f) const;
	Bounds uvs(const Glyph& glyph) const;
};

constexpr bool carriage_return_1(Codepoint codepoint)
{
	return codepoint == '\n' || codepoint == '\r';
}

constexpr bool carriage_return_2(Codepoint r, Codepoint n)
{
	return r == '\r' && n == '\n';
}

class FontRange
{
	std::map<float, Font> fonts;
	FilePath font_filepath;
	std::shared_ptr<Kerning> kerning = nullptr;
	
public:
	FontRange(const FilePath& filepath, const FilePath& kerning_filepath = "") : font_filepath(filepath), kerning(std::make_shared<Kerning>(kerning_filepath)) {}
	FontRange(const FontRange&) = delete;
	FontRange(FontRange&&) noexcept = delete;

	bool construct_fontsize(float font_size, UTF::String common_buffer = Fonts::COMMON, TextureParams texture_params = TextureParams::linear);
	float get_font_and_multiplier(float font_size, Font*& font);
};
