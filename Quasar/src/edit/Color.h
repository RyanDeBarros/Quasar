#pragma once

#include <algorithm>
#include <climits>

#include <glm/glm.hpp>

#include "variety/Utils.h"

// TODO define literals for unsigned char/short

constexpr float NEAR_ZERO = 0.00001f;
constexpr float inv60 = 1.0f / 60;
constexpr float inv255 = 1.0f / 255;
constexpr float inv359 = 1.0f / 359;
constexpr unsigned short sq255 = 255 * 255;

constexpr float float_from_7bit(unsigned char _7bit)
{
	return _7bit < 64 ? (_7bit << 1) * inv255 : (_7bit << 1 | 0x1) * inv255;
}

struct RGB;
struct HSV;
struct HSL;

/// R[0,255] - G[0,255] - B[0,255]
struct RGB
{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	constexpr RGB() = default;
	constexpr RGB(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}
	constexpr RGB(int r, int g, int b) : r(static_cast<unsigned char>(r)), g(static_cast<unsigned char>(g)), b(static_cast<unsigned char>(b)) {}
	constexpr RGB(unsigned int hex) : r(hex_to_byte<2>(hex)), g(hex_to_byte<1>(hex)), b(hex_to_byte<0>(hex)) {}
	constexpr RGB(const glm::vec3& vec) : r(round_uchar(255 * vec.x)), g(round_uchar(255 * vec.y)), b(round_uchar(255 * vec.z)) {}
	constexpr RGB(float r, float g, float b) : r(round_uchar(255 * r)), g(round_uchar(255 * g)), b(round_uchar(255 * b)) {}

	constexpr HSV to_hsv() const;
	constexpr HSL to_hsl() const;
	constexpr glm::vec3 as_vec() const { return { r * inv255, g * inv255, b * inv255 }; }
};

/// H[0,359] - S[0,127] - V[0,255]
struct HSV
{
private:
	unsigned char h : 8 = 0;
	unsigned char h_msb : 1 = 0;

public:
	unsigned char s : 7 = 0;
	unsigned char v : 8 = 0;

	constexpr HSV() = default;
	constexpr HSV(unsigned short h, unsigned char s, unsigned char v) : s(s), v(v) { set_hue(h); }
	constexpr HSV(int h, int s, int v) : s(static_cast<unsigned char>(s)), v(static_cast<unsigned char>(v)) { set_hue(static_cast<unsigned char>(h)); }
	constexpr HSV(unsigned int hex) : s(hex_to_byte<1>(hex)), v(hex_to_byte<0>(hex)) { set_hue(hex_to_byte<2>(hex)); }
	constexpr HSV(const glm::vec3& vec) : s(round_uchar(127 * vec.y)), v(round_uchar(255 * vec.z)) { set_hue(round_ushort(359 * vec.x)); }
	constexpr HSV(float h, float s, float v) : s(round_uchar(127 * s)), v(round_uchar(255 * v)) { set_hue(round_ushort(359 * h)); }
	
	constexpr unsigned short get_hue() const { return static_cast<unsigned short>(h) | (h_msb << 8); }
	constexpr void set_hue(unsigned short hue) { if (hue < 360) { h = hue & 0xFF; h_msb = hue >> 8; } else { h = 0xFF; h_msb = 0x1; } }
	constexpr float sat_as_float() const { return float_from_7bit(s); }

	constexpr RGB to_rgb() const;
	constexpr HSL to_hsl() const;
	constexpr glm::vec3 as_vec() const { return { get_hue() * inv359, sat_as_float(), v * inv255 }; }
};

/// H[0,359] - S[0,127] - L[0,255]
struct HSL
{
private:
	unsigned char h : 8 = 0;
	unsigned char h_msb : 1 = 0;

public:
	unsigned char s : 7 = 0;
	unsigned char l : 8 = 0;

	constexpr HSL() = default;
	constexpr HSL(unsigned short h, unsigned char s, unsigned char l) : s(s), l(l) { set_hue(h); }
	constexpr HSL(int h, int s, int l) : s(static_cast<unsigned char>(s)), l(static_cast<unsigned char>(l)) { set_hue(static_cast<unsigned char>(h)); }
	constexpr HSL(unsigned int hex) : s(hex_to_byte<1>(hex)), l(hex_to_byte<0>(hex)) { set_hue(hex_to_byte<2>(hex)); }
	constexpr HSL(const glm::vec3& vec) : s(round_uchar(127 * vec.y)), l(round_uchar(255 * vec.z)) { set_hue(round_ushort(359 * vec.x)); }
	constexpr HSL(float h, float s, float l) : s(round_uchar(127 * s)), l(round_uchar(255 * l)) { set_hue(round_ushort(359 * h)); }

	constexpr unsigned short get_hue() const { return static_cast<unsigned short>(h) | (h_msb << 8); }
	constexpr void set_hue(unsigned short hue) { if (hue < 360) { h = hue & 0xFF; h_msb = hue >> 8; } else { h = 0xFF; h_msb = 0x1; } }
	constexpr float sat_as_float() const { return float_from_7bit(s); }

	constexpr RGB to_rgb() const;
	constexpr HSV to_hsv() const;
	constexpr glm::vec3 color_as_vec() const { return { get_hue() * inv359, sat_as_float(), l * inv255 }; }
};

constexpr HSV RGB::to_hsv() const
{
	// Normalize RGB
	float nr = r * inv255;
	float ng = g * inv255;
	float nb = b * inv255;
	// Compute max, min and chroma
	unsigned char max_hex = std::max({ r, g, b });
	float max = max_hex * inv255;
	float min = std::min({ nr, ng, nb });
	float chroma = max - min;

	HSV hsv(unsigned short(0), unsigned char(0), max_hex);
	if (max_hex != 0)
		hsv.s = round_uchar(127 * chroma / max);
	if (chroma > NEAR_ZERO)
	{
		// Compute hue angle
		float hue = 0;
		if (max_hex == r)
			hue = (ng - nb) / chroma;
		else if (max_hex == g)
			hue = 2.0f + (nb - nr) / chroma;
		else
			hue = 4.0f + (nr - ng) / chroma;
		hue *= 60.0f;
		if (hue < 0.0f)
			hue += 360.0f;
		hsv.set_hue(round_ushort(hue));
	}
	return hsv;
}

constexpr HSL RGB::to_hsl() const
{
	HSL hsl{};
	// Normalize RGB
	float nr = r * inv255;
	float ng = g * inv255;
	float nb = b * inv255;
	// Compute max, min and chroma
	unsigned char max_hex = std::max({ r, g, b });
	unsigned char min_hex = std::min({ r, g, b });
	float chroma = (max_hex - min_hex) * inv255;
	// Lightness
	hsl.l = round_uchar((max_hex + min_hex) * 0.5f);
	if (chroma > NEAR_ZERO)
	{
		// Compute hue angle
		float hue = 0;
		if (max_hex == r)
			hue = (ng - nb) / chroma;
		else if (max_hex == g)
			hue = 2.0f + (nb - nr) / chroma;
		else
			hue = 4.0f + (nr - ng) / chroma;
		hue *= 60.0f;
		if (hue < 0.0f)
			hue += 360.0f;
		hsl.set_hue(round_ushort(hue));
		hsl.s = round_uchar(127 * (max_hex - min_hex) / (255.0f - std::abs(max_hex + min_hex - 255)));
	}
	return hsl;
}

constexpr RGB HSV::to_rgb() const
{
	if (s == 0)
		return RGB(v, v, v);
	// Normalize
	float ns = sat_as_float();
	// Sextant index
	unsigned char si = get_hue() / 60;
	// Fractional part
	float fr = (get_hue() * inv60) - si;
	// Compute non-primary color characteristics
	unsigned char min = round_uchar(v * (1 - ns));
	unsigned char pre = round_uchar(v * (1 - ns * fr));
	unsigned char post = round_uchar(v * (1 - ns * (1.0f - fr)));
	// Switch on sextant
	switch (si)
	{
	case 0:
		return RGB(v, post, min);
	case 1:
		return RGB(pre, v, min);
	case 2:
		return RGB(min, v, post);
	case 3:
		return RGB(min, pre, v);
	case 4:
		return RGB(post, min, v);
	case 5:
		return RGB(v, min, pre);
	}
	return RGB{};
}

constexpr HSL HSV::to_hsl() const
{
	if (v == 0)
		return HSL(get_hue(), unsigned char(0), unsigned char(0));
	// Normalize
	float sat_hsv = sat_as_float();
	float nv = v * inv255;
	// Lightness
	float l = nv * (1.0f - 0.5f * sat_hsv);
	// HSL saturation
	float sat_hsl = 0.0f;
	if (l < 1.0f - NEAR_ZERO)
		sat_hsl = 0.5f * nv * sat_hsv / std::min(l, 1.0f - l);
	return HSL(get_hue(), round_uchar(127 * sat_hsl), round_uchar(255 * l));
}

constexpr RGB HSL::to_rgb() const
{
	// Chroma
	unsigned short hue = get_hue();
	float nl = l * inv255;
	float chroma = (1.0f - absolute(2 * nl - 1.0f)) * sat_as_float();
	float x = chroma * (1.0f - absolute(modulo((hue * inv60), 2.0f) - 1.0f));

	// RGB channels (unordered)
	unsigned char c1 = round_uchar(l + 255 * chroma * 0.5f);
	unsigned char c2 = round_uchar(l - 255 * chroma * 0.5f + 255 * x);
	unsigned char c3 = round_uchar(l - 255 * chroma * 0.5f);

	// Order RGB channels
	switch (h / 60)
	{
	case 0:
		return RGB(c1, c2, c3);
	case 1:
		return RGB(c2, c1, c3);
	case 2:
		return RGB(c3, c1, c2);
	case 3:
		return RGB(c3, c2, c1);
	case 4:
		return RGB(c2, c3, c1);
	case 5:
		return RGB(c1, c3, c2);
	}
	return RGB{};
}

constexpr HSV HSL::to_hsv() const
{
	// Normalize
	float nl = l * inv255;
	// Value
	float v = nl + sat_as_float() * std::min(nl, 1.0f - nl);
	// HSV saturation
	float sat_hsv = 0.0f;
	if (v > NEAR_ZERO)
		sat_hsv = 2.0f * (1.0f - nl / v);
	return HSV(get_hue(), round_uchar(127 * sat_hsv), round_uchar(255 * v));
}

struct RGBA
{
	RGB rgb;
	unsigned char alpha = 255;

	constexpr RGBA(RGB rgb = RGB(255, 255, 255), unsigned char alpha = 255) : rgb(rgb), alpha(alpha) {}
	constexpr RGBA(RGB rgb, int alpha) : rgb(rgb), alpha(static_cast<unsigned char>(alpha)) {}
	constexpr RGBA(RGB rgb, float alpha) : rgb(rgb), alpha(round_uchar(255 * alpha)) {}
};

struct HSVA
{
	HSV hsv;
	unsigned char alpha = 255;

	constexpr HSVA(HSV hsv = HSV(0, 0, 255), unsigned char alpha = 255) : hsv(hsv), alpha(alpha) {}
	constexpr HSVA(HSV hsv, int alpha) : hsv(hsv), alpha(static_cast<unsigned char>(alpha)) {}
	constexpr HSVA(HSV hsv, float alpha) : hsv(hsv), alpha(round_uchar(255 * alpha)) {}
};

struct HSLA
{
	HSL hsl;
	unsigned char alpha = 255;

	constexpr HSLA(HSL hsl = HSL(0, 0, 255), unsigned char alpha = 255) : hsl(hsl), alpha(alpha) {}
	constexpr HSLA(HSL hsl, int alpha) : hsl(hsl), alpha(static_cast<unsigned char>(alpha)) {}
	constexpr HSLA(HSL hsl, float alpha) : hsl(hsl), alpha(round_uchar(255 * alpha)) {}
};

struct ColorFrame
{
	unsigned char alpha = 255;

private:
	RGB _rgb = RGB(255, 255, 255);
	HSV _hsv = HSV(0, 0, 255);
	HSL _hsl = HSL(0, 0, 255);

public:
	constexpr ColorFrame(unsigned char alpha = 255) : alpha(alpha) {}
	constexpr ColorFrame(int alpha) : alpha(static_cast<unsigned char>(alpha)) {}
	constexpr ColorFrame(RGB rgb, unsigned char alpha = 255) : alpha(alpha) { set_rgb(rgb); }
	constexpr ColorFrame(HSV hsv, unsigned char alpha = 255) : alpha(alpha) { set_hsv(hsv); }
	constexpr ColorFrame(HSL hsl, unsigned char alpha = 255) : alpha(alpha) { set_hsl(hsl); }
	constexpr ColorFrame(RGB rgb, int alpha) : alpha(static_cast<unsigned char>(alpha)) { set_rgb(rgb); }
	constexpr ColorFrame(HSV hsv, int alpha) : alpha(static_cast<unsigned char>(alpha)) { set_hsv(hsv); }
	constexpr ColorFrame(HSL hsl, int alpha) : alpha(static_cast<unsigned char>(alpha)) { set_hsl(hsl); }
	constexpr ColorFrame(RGB rgb, float alpha) : alpha(round_uchar(255 * alpha)) { set_rgb(rgb); }
	constexpr ColorFrame(HSV hsv, float alpha) : alpha(round_uchar(255 * alpha)) { set_hsv(hsv); }
	constexpr ColorFrame(HSL hsl, float alpha) : alpha(round_uchar(255 * alpha)) { set_hsl(hsl); }

	constexpr RGB rgb() const { return _rgb; }
	constexpr HSV hsv() const { return _hsv; }
	constexpr HSL hsl() const { return _hsl; }
	
	constexpr void set_rgb(RGB rgb) { _rgb = rgb; _hsv = rgb.to_hsv(); _hsl = rgb.to_hsl(); }
	constexpr void set_hsv(HSV hsv) { _hsv = hsv; _rgb = hsv.to_rgb(); _hsl = hsv.to_hsl(); }
	constexpr void set_hsl(HSL hsl) { _hsl = hsl; _rgb = hsl.to_rgb(); _hsv = hsl.to_hsv(); }

	constexpr glm::vec4 rgba_as_vec() const { return { _rgb.r * inv255, _rgb.g * inv255, _rgb.b * inv255, alpha * inv255 }; }
	constexpr glm::vec4 hsva_as_vec() const { return { _hsv.get_hue() * inv359, _hsv.sat_as_float(), _hsv.v * inv255, alpha * inv255 }; }
	constexpr glm::vec4 hsla_as_vec() const { return { _hsl.get_hue() * inv359, _hsl.sat_as_float(), _hsl.l * inv255, alpha * inv255 }; }
};

struct RedGreen
{
	unsigned char r = 0, g = 0;
};

struct GrayScale
{
	unsigned char v = 0;
};
