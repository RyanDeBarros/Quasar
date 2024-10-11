#pragma once

#include <climits>
#include <glm/glm.hpp>

constexpr float inv60 = 1.0f / 60;
constexpr float inv255 = 1.0f / 255;
constexpr float inv359 = 1.0f / 359;
constexpr unsigned short sq255 = 255 * 255;

constexpr unsigned char round_uchar(float x)
{
	if (x <= 0.0f) return 0;
	if (x >= static_cast<float>(UCHAR_MAX)) return UCHAR_MAX;
	return static_cast<unsigned char>(x + 0.5f);
}
constexpr unsigned short round_ushort(float x)
{
	if (x <= 0.0f) return 0;
	if (x >= static_cast<float>(USHRT_MAX)) return USHRT_MAX;
	return static_cast<unsigned short>(x + 0.5f);
}
constexpr float float_from_7bit(unsigned char _7bit)
{
	return _7bit < 64 ? (_7bit << 1) * inv255 : (_7bit << 1 | 0x1) * inv255;
}

template<char i>
constexpr unsigned char hex_to_byte(unsigned int hex)
{
	if constexpr (i == 0)
		return hex & 0x000000FF;
	else if constexpr (i == 1)
		return (hex & 0x0000FF00) >> 8;
	else if constexpr (i == 2)
		return (hex & 0x00FF0000) >> 16;
	else if constexpr (i == 3)
		return (hex & 0xFF000000) >> 24;
	else
		static_assert(false);
}

/// R[0,255] - G[0,255] - B[0,255]
struct RGB
{
	unsigned char r;
	unsigned char g;
	unsigned char b;

	RGB() = default;
	RGB(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}
	RGB(unsigned int hex) : r(hex_to_byte<2>(hex)), g(hex_to_byte<1>(hex)), b(hex_to_byte<0>(hex)) {}
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

	unsigned short get_hue() const { return static_cast<unsigned short>(h) | (h_msb << 8); }
	void set_hue(unsigned short hue)
	{
		if (hue < 360)
		{
			h = hue & 0xFF;
			h_msb = hue >> 8;
		}
		else
		{
			h = 0xFF;
			h_msb = 0x1;
		}
	}
	float sat_as_float() const { return float_from_7bit(s); }
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

	unsigned short get_hue() const { return static_cast<unsigned short>(h) | (h_msb << 8); }
	void set_hue(unsigned short hue)
	{
		if (hue < 360)
		{
			h = hue & 0xFF;
			h_msb = hue >> 8;
		}
		else
		{
			h = 0xFF;
			h_msb = 0x1;
		}
	}
	float sat_as_float() const { return float_from_7bit(s); }
};

extern HSV to_hsv(RGB rgb);
extern RGB to_rgb(HSV hsv);
extern HSL to_hsl(RGB rgb);
extern RGB to_rgb(HSL hsl);
extern HSV to_hsv(HSL hsl);
extern HSL to_hsl(HSV hsv);

inline glm::vec3 color_as_vec(RGB rgb) { return { rgb.r * inv255, rgb.g * inv255, rgb.b * inv255 }; }
inline glm::vec3 color_as_vec(HSV hsv) { return { hsv.get_hue() * inv359, hsv.sat_as_float(), hsv.v * inv255}; }
inline glm::vec3 color_as_vec(HSL hsl) { return { hsl.get_hue() * inv359, hsl.sat_as_float(), hsl.l * inv255 }; }
inline RGB rgb_from_vec(const glm::vec3& vec) { return RGB(round_uchar(255 * vec.x), round_uchar(255 * vec.y), round_uchar(255 * vec.z)); }
inline HSV hsv_from_vec(const glm::vec3& vec) { return HSV(round_ushort(359 * vec.x), round_uchar(127 * vec.y), round_uchar(255 * vec.z)); }
inline HSL hsl_from_vec(const glm::vec3& vec) { return HSL(round_ushort(359 * vec.x), round_uchar(127 * vec.y), round_uchar(255 * vec.z)); }

struct ColorFrame
{
private:
	RGB _rgb;
	HSV _hsv;
	HSL _hsl;

public:
	ColorFrame() = default;
	ColorFrame(RGB rgb) { set_rgb(rgb); }
	ColorFrame(HSV hsv) { set_hsv(hsv); }
	ColorFrame(HSL hsl) { set_hsl(hsl); }

	RGB rgb() const { return _rgb; }
	HSV hsv() const { return _hsv; }
	HSL hsl() const { return _hsl; }
	void set_rgb(RGB rgb) { _rgb = rgb; _hsv = to_hsv(rgb); _hsl = to_hsl(rgb); }
	void set_hsv(HSV hsv) { _hsv = hsv; _rgb = to_rgb(hsv); _hsl = to_hsl(hsv); }
	void set_hsl(HSL hsl) { _hsl = hsl; _rgb = to_rgb(hsl); _hsv = to_hsv(hsl); }
};
