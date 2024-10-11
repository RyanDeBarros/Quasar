#pragma once

#include <climits>
#include <glm/glm.hpp>

#include "Utils.h"

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
	unsigned char r;
	unsigned char g;
	unsigned char b;

	RGB() = default;
	RGB(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}
	RGB(unsigned int hex) : r(hex_to_byte<2>(hex)), g(hex_to_byte<1>(hex)), b(hex_to_byte<0>(hex)) {}
	RGB(const glm::vec3& vec) : r(round_uchar(255 * vec.x)), g(round_uchar(255 * vec.y)), b(round_uchar(255 * vec.z)) {}
	RGB(float r, float g, float b) : r(round_uchar(255 * r)), g(round_uchar(255 * g)), b(round_uchar(255 * b)) {}

	HSV to_hsv() const;
	HSL to_hsl() const;
	glm::vec3 as_vec() const { return { r * inv255, g * inv255, b * inv255 }; }
};

/// H[0,359] - S[0,127] - V[0,255]
struct HSV
{
private:
	unsigned char h : 8;
	unsigned char h_msb : 1;

public:
	unsigned char s : 7;
	unsigned char v : 8;

	HSV() = default;
	HSV(unsigned short h, unsigned char s, unsigned char v) : s(s), v(v) { set_hue(h); }
	HSV(unsigned int hex) : s(hex_to_byte<1>(hex)), v(hex_to_byte<0>(hex)) { set_hue(hex_to_byte<2>(hex)); }
	HSV(const glm::vec3& vec) : s(round_uchar(127 * vec.y)), v(round_uchar(255 * vec.z)) { set_hue(round_ushort(359 * vec.x)); }
	HSV(float h, float s, float v) : s(round_uchar(127 * s)), v(round_uchar(255 * v)) { set_hue(round_ushort(359 * h)); }
	
	unsigned short get_hue() const { return static_cast<unsigned short>(h) | (h_msb << 8); }
	void set_hue(unsigned short hue) { if (hue < 360) { h = hue & 0xFF; h_msb = hue >> 8; } else { h = 0xFF; h_msb = 0x1; } }
	float sat_as_float() const { return float_from_7bit(s); }

	RGB to_rgb() const;
	HSL to_hsl() const;
	glm::vec3 as_vec() const { return { get_hue() * inv359, sat_as_float(), v * inv255 }; }
};

/// H[0,359] - S[0,127] - L[0,255]
struct HSL
{
private:
	unsigned char h : 8;
	unsigned char h_msb : 1;

public:
	unsigned char s : 7;
	unsigned char l : 8;

	HSL() = default;
	HSL(unsigned short h, unsigned char s, unsigned char l) : s(s), l(l) { set_hue(h); }
	HSL(unsigned int hex) : s(hex_to_byte<1>(hex)), l(hex_to_byte<0>(hex)) { set_hue(hex_to_byte<2>(hex)); }
	HSL(const glm::vec3& vec) : s(round_uchar(127 * vec.y)), l(round_uchar(255 * vec.z)) { set_hue(round_ushort(359 * vec.x)); }
	HSL(float h, float s, float l) : s(round_uchar(127 * s)), l(round_uchar(255 * l)) { set_hue(round_ushort(359 * h)); }

	unsigned short get_hue() const { return static_cast<unsigned short>(h) | (h_msb << 8); }
	void set_hue(unsigned short hue) { if (hue < 360) { h = hue & 0xFF; h_msb = hue >> 8; } else { h = 0xFF; h_msb = 0x1; } }
	float sat_as_float() const { return float_from_7bit(s); }

	RGB to_rgb() const;
	HSV to_hsv() const;
	glm::vec3 color_as_vec() const { return { get_hue() * inv359, sat_as_float(), l * inv255 }; }
};

struct RGBA
{
	RGB rgb;
	unsigned char alpha;
};

struct HSVA
{
	HSV hsv;
	unsigned char alpha;
};

struct HSLA
{
	HSL hsl;
	unsigned char alpha;
};

struct ColorFrame
{
	unsigned char alpha;

private:
	RGB _rgb{};
	HSV _hsv{};
	HSL _hsl{};

public:
	ColorFrame(unsigned char alpha = 0) : alpha(alpha) {}
	ColorFrame(RGB rgb, unsigned char alpha = 0) : alpha(alpha) { set_rgb(rgb); }
	ColorFrame(HSV hsv, unsigned char alpha = 0) : alpha(alpha) { set_hsv(hsv); }
	ColorFrame(HSL hsl, unsigned char alpha = 0) : alpha(alpha) { set_hsl(hsl); }
	ColorFrame(RGB rgb, int alpha = 0) : alpha(static_cast<unsigned char>(alpha)) { set_rgb(rgb); }
	ColorFrame(HSV hsv, int alpha = 0) : alpha(static_cast<unsigned char>(alpha)) { set_hsv(hsv); }
	ColorFrame(HSL hsl, int alpha = 0) : alpha(static_cast<unsigned char>(alpha)) { set_hsl(hsl); }
	ColorFrame(RGB rgb, float alpha = 0.0f) : alpha(round_uchar(255 * alpha)) { set_rgb(rgb); }
	ColorFrame(HSV hsv, float alpha = 0.0f) : alpha(round_uchar(255 * alpha)) { set_hsv(hsv); }
	ColorFrame(HSL hsl, float alpha = 0.0f) : alpha(round_uchar(255 * alpha)) { set_hsl(hsl); }

	RGB rgb() const { return _rgb; }
	HSV hsv() const { return _hsv; }
	HSL hsl() const { return _hsl; }
	
	void set_rgb(RGB rgb) { _rgb = rgb; _hsv = rgb.to_hsv(); _hsl = rgb.to_hsl(); }
	void set_hsv(HSV hsv) { _hsv = hsv; _rgb = hsv.to_rgb(); _hsl = hsv.to_hsl(); }
	void set_hsl(HSL hsl) { _hsl = hsl; _rgb = hsl.to_rgb(); _hsv = hsl.to_hsv(); }

	glm::vec4 rgba_as_vec() const { return { _rgb.r * inv255, _rgb.g * inv255, _rgb.b * inv255, alpha * inv255 }; }
	glm::vec4 hsva_as_vec() const { return { _hsv.get_hue() * inv359, _hsv.sat_as_float(), _hsv.v * inv255, alpha * inv255 }; }
	glm::vec4 hsla_as_vec() const { return { _hsl.get_hue() * inv359, _hsl.sat_as_float(), _hsl.l * inv255, alpha * inv255 }; }
};
