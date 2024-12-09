#pragma once

#include <algorithm>
#include <climits>

#include <glm/glm.hpp>

#include "variety/Utils.h"
#include "variety/Geometry.h"

constexpr float NEAR_ZERO = 1.0f / (255 * 255);
constexpr float inv6 = 1.0f / 6;
constexpr float inv255 = 1.0f / 255;
constexpr float inv359 = 1.0f / 359;

struct RGB;
struct HSV;
struct HSL;

struct RGB
{
	float r;
	float g;
	float b;

	constexpr RGB() : r(0.0f), g(0.0f), b(0.0f) {}
	constexpr RGB(int r, int g, int b) : r(std::clamp(r, 0, 255) * inv255), g(std::clamp(g, 0, 255) * inv255), b(std::clamp(b, 0, 255) * inv255) {}
	constexpr RGB(float r, float g, float b) : r(std::clamp(r, 0.0f, 1.0f)), g(std::clamp(g, 0.0f, 1.0f)), b(std::clamp(b, 0.0f, 1.0f)) {}
	constexpr RGB(unsigned int hex) : r(hex_to_byte<2>(hex) * inv255), g(hex_to_byte<1>(hex) * inv255), b(hex_to_byte<0>(hex) * inv255) {}
	
	constexpr bool operator==(const RGB&) const = default;

	constexpr glm::ivec3 pixel3() const { return { get_pixel_r(), get_pixel_g(), get_pixel_b() }; }
	constexpr int get_pixel_r() const { return roundi(r * 255); }
	constexpr int get_pixel_g() const { return roundi(g * 255); }
	constexpr int get_pixel_b() const { return roundi(b * 255); }
	constexpr void set_pixel_r(int r_) { r = std::clamp(r_, 0, 255) * inv255; }
	constexpr void set_pixel_g(int g_) { g = std::clamp(g_, 0, 255) * inv255; }
	constexpr void set_pixel_b(int b_) { b = std::clamp(b_, 0, 255) * inv255; }

	constexpr HSV to_hsv() const;
	constexpr HSL to_hsl() const;
};

struct HSV
{
	float h;
	float s;
	float v;

	constexpr HSV() : h(0.0f), s(0.0f), v(0.0f) {}
	constexpr HSV(int h, int s, int v) : h(std::clamp(h, 0, 359) * inv359), s(std::clamp(s, 0, 255) * inv255), v(std::clamp(v, 0, 255) * inv255) {}
	constexpr HSV(float h, float s, float v) : h(std::clamp(h, 0.0f, 1.0f)), s(std::clamp(s, 0.0f, 1.0f)), v(std::clamp(v, 0.0f, 1.0f)) {}

	constexpr bool operator==(const HSV&) const = default;

	constexpr glm::ivec3 pixel3() const { return { get_pixel_h(), get_pixel_s(), get_pixel_v() }; }
	constexpr int get_pixel_h() const { return roundi(h * 359); }
	constexpr int get_pixel_s() const { return roundi(s * 255); }
	constexpr int get_pixel_v() const { return roundi(v * 255); }
	constexpr void set_pixel_h(int h_) { h = std::clamp(h_, 0, 359) * inv359; }
	constexpr void set_pixel_s(int s_) { s = std::clamp(s_, 0, 255) * inv255; }
	constexpr void set_pixel_v(int v_) { v = std::clamp(v_, 0, 255) * inv255; }
	
	constexpr RGB to_rgb() const;
	constexpr HSL to_hsl() const;
};

struct HSL
{
	float h;
	float s;
	float l;

	constexpr HSL() : h(0.0f), s(0.0f), l(0.0f) {}
	constexpr HSL(int h, int s, int l) : h(std::clamp(h, 0, 359) * inv359), s(std::clamp(s, 0, 255) * inv255), l(std::clamp(l, 0, 255) * inv255) {}
	constexpr HSL(float h, float s, float l) : h(std::clamp(h, 0.0f, 1.0f)), s(std::clamp(s, 0.0f, 1.0f)), l(std::clamp(l, 0.0f, 1.0f)) {}

	constexpr bool operator==(const HSL&) const = default;

	constexpr glm::ivec3 pixel3() const { return { get_pixel_h(), get_pixel_s(), get_pixel_l() }; }
	constexpr int get_pixel_h() const { return roundi(h * 359); }
	constexpr int get_pixel_s() const { return roundi(s * 255); }
	constexpr int get_pixel_l() const { return roundi(l * 255); }
	constexpr void set_pixel_h(int h_) { h = std::clamp(h_, 0, 359) * inv359; }
	constexpr void set_pixel_s(int s_) { s = std::clamp(s_, 0, 255) * inv255; }
	constexpr void set_pixel_l(int l_) { l = std::clamp(l_, 0, 255) * inv255; }
	
	constexpr RGB to_rgb() const;
	constexpr HSV to_hsv() const;
};

constexpr HSV RGB::to_hsv() const
{
	// Compute max, min and chroma
	float max = std::max({ r, g, b });
	float min = std::min({ r, g, b });
	float chroma = max - min;

	HSV hsv(0.0f, 0.0f, max);
	if (max > NEAR_ZERO)
		hsv.s = chroma / max;
	if (chroma > NEAR_ZERO)
	{
		// Compute hue angle
		float hue = 0.0f;
		if (max == r)
			hue = (g - b) / chroma;
		else if (max == g)
			hue = 2.0f + (b - r) / chroma;
		else
			hue = 4.0f + (r - g) / chroma;

		if (hue < 0.0f)
			hue += 6.0f;
		hsv.h = hue * inv6;
	}
	return hsv;
}

constexpr HSL RGB::to_hsl() const
{
	HSL hsl;
	// Compute max, min and chroma
	float max = std::max({ r, g, b });
	float min = std::min({ r, g, b });
	float chroma = max - min;
	// Lightness
	hsl.l = (max + min) * 0.5f;
	if (chroma > NEAR_ZERO)
	{
		hsl.s = chroma / (1.0f - std::abs(max + min - 1.0f));
		// Compute hue angle
		float hue = 0.0f;
		if (max == r)
			hue = (g - b) / chroma;
		else if (max == g)
			hue = 2.0f + (b - r) / chroma;
		else
			hue = 4.0f + (r - g) / chroma;

		if (hue < 0.0f)
			hue += 6.0f;
		hsl.h = hue * inv6;
	}
	return hsl;
}

constexpr RGB HSV::to_rgb() const
{
	if (s <= NEAR_ZERO)
		return RGB(v, v, v);
	// Sextant index
	unsigned char si = static_cast<unsigned char>(h * 6.0f);
	// Fractional part
	float fr = h * 6.0f - si;
	// Compute non-primary color characteristics
	float min = v * (1 - s);
	float pre = v * (1 - s * fr);
	float post = v * (1 - s * (1.0f - fr));
	// Switch on sextant
	switch (si % 6)
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
	return RGB();
}

constexpr HSL HSV::to_hsl() const
{
	if (v <= NEAR_ZERO)
		return HSL(h, 0.0f, 0.0f);
	HSL hsl(h, 0.0f, 1.0f);
	// Lightness
	hsl.l = v * (1.0f - 0.5f * s);
	// HSL saturation
	float denominator = std::min(hsl.l, 1.0f - hsl.l);
	if (denominator > NEAR_ZERO)
		hsl.s = 0.5f * v * s / denominator;
	return hsl;
}

constexpr RGB HSL::to_rgb() const
{
	// Chroma
	float chroma = (1.0f - absolute(2 * l - 1.0f)) * s;
	float x = chroma * (1.0f - absolute(modulo(h * 6.0f, 2.0f) - 1.0f));
	// RGB channels (unordered)
	float c1 = l + chroma * 0.5f;
	float c2 = l - chroma * 0.5f + x;
	float c3 = l - chroma * 0.5f;
	// Order RGB channels
	unsigned char si = static_cast<unsigned char>(h * 6.0f);
	switch (si % 6)
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
	return RGB();
}

constexpr HSV HSL::to_hsv() const
{
	HSV hsv(h, 0.0f, 0.0f);
	// Value
	hsv.v = l + s * std::min(l, 1.0f - l);
	// HSV saturation
	if (hsv.v > NEAR_ZERO)
		hsv.s = 2.0f * (1.0f - l / hsv.v);
	return hsv;
}

struct RGBA;
struct PixelRGBA;
struct HSVA;
struct HSLA;

struct RGBA
{
	RGB rgb;
	float alpha;

	constexpr RGBA(RGB rgb = RGB(), float alpha = 0.0f) : rgb(rgb), alpha(std::clamp(alpha, 0.0f, 1.0f)) {}
	constexpr RGBA(RGB rgb, int alpha) : rgb(rgb), alpha(std::clamp(alpha, 0, 255) * inv255) {}
	constexpr RGBA(float r, float g, float b, float a) : rgb(r, g, b), alpha(std::clamp(a, 0.0f, 1.0f)) {}
	constexpr RGBA(int r, int g, int b, int a) : rgb(r, g, b), alpha(std::clamp(a, 0, 255) * inv255) {}

	constexpr bool operator==(const RGBA&) const = default;

	glm::vec4 as_vec() const { return { rgb.r, rgb.g, rgb.b, alpha }; }

	constexpr int get_pixel_r() const { return rgb.get_pixel_r(); }
	constexpr int get_pixel_g() const { return rgb.get_pixel_g(); }
	constexpr int get_pixel_b() const { return rgb.get_pixel_b(); }
	constexpr int get_pixel_a() const { return roundi(alpha * 255); }
	constexpr void set_pixel_r(int r) { rgb.set_pixel_r(r); }
	constexpr void set_pixel_g(int g) { rgb.set_pixel_g(g); }
	constexpr void set_pixel_b(int b) { rgb.set_pixel_b(b); }
	constexpr void set_pixel_a(int a) { alpha = std::clamp(a, 0, 255) * inv255; }
	constexpr PixelRGBA get_pixel_rgba() const;
	constexpr void set_pixel_rgba(PixelRGBA px_rgba);

	constexpr HSVA to_hsva() const;
	constexpr HSLA to_hsla() const;

	constexpr void blend_over(RGBA bkg);
	constexpr RGBA no_alpha_equivalent() const { return RGBA(rgb.r * alpha, rgb.g * alpha, rgb.b * alpha, 1.0f); }

	static const RGBA WHITE;
	static const RGBA BLACK;
};

constexpr void RGBA::blend_over(RGBA bkg)
{
	float new_alpha = std::clamp(alpha + bkg.alpha * (1.0f - alpha), 0.0f, 1.0f);
	if (new_alpha != 0.0f)
	{
		float inv_alpha = 1.0f / new_alpha;
		rgb.r = std::clamp((rgb.r * alpha + bkg.rgb.r * bkg.alpha * (1.0f - alpha)) * inv_alpha, 0.0f, 1.0f);
		rgb.g = std::clamp((rgb.g * alpha + bkg.rgb.g * bkg.alpha * (1.0f - alpha)) * inv_alpha, 0.0f, 1.0f);
		rgb.b = std::clamp((rgb.b * alpha + bkg.rgb.b * bkg.alpha * (1.0f - alpha)) * inv_alpha, 0.0f, 1.0f);
	}
	alpha = new_alpha;
}

inline const RGBA RGBA::WHITE = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
inline const RGBA RGBA::BLACK = RGBA(0.0f, 0.0f, 0.0f, 1.0f);

struct PixelRGBA
{
	unsigned char r = 0, g = 0, b = 0, a = 0;
	constexpr RGBA to_rgba() const { return RGBA(r, g, b, a); }
	constexpr unsigned char operator[](int i) const { return i == 0 ? r : i == 1 ? g : i == 2 ? b : a; }
	const unsigned char& at(int i) const { return i == 0 ? r : i == 1 ? g : i == 2 ? b : a; }
	unsigned char& at(int i) { return i == 0 ? r : i == 1 ? g : i == 2 ? b : a; }
	bool operator==(const PixelRGBA&) const = default;

	constexpr void blend_over(PixelRGBA bkg);
};

constexpr void PixelRGBA::blend_over(PixelRGBA bkg)
{
	float new_alpha = std::clamp((a + bkg.a * (1.0f - a * inv255)) * inv255, 0.0f, 1.0f);
	if (new_alpha != 0.0f)
	{
		float inv_alpha = inv255 / new_alpha;
		r = std::clamp(roundi((r * a + bkg.r * bkg.a * (1.0f - a * inv255)) * inv_alpha), 0, 255);
		g = std::clamp(roundi((g * a + bkg.g * bkg.a * (1.0f - a * inv255)) * inv_alpha), 0, 255);
		b = std::clamp(roundi((b * a + bkg.b * bkg.a * (1.0f - a * inv255)) * inv_alpha), 0, 255);
	}
	a = std::clamp(roundi(new_alpha * 255), 0, 255);
}

constexpr PixelRGBA blend_over(PixelRGBA fore, PixelRGBA back)
{
	PixelRGBA blend = fore;
	blend.blend_over(back);
	return blend;
}

template<>
struct std::hash<PixelRGBA>
{
	size_t operator()(const PixelRGBA& p) const { return std::hash<unsigned char>{}(p.r) ^ std::hash<unsigned char>{}(p.g)
		^ std::hash<unsigned char>{}(p.b) ^ std::hash<unsigned char>{}(p.a); }
};

constexpr PixelRGBA RGBA::get_pixel_rgba() const
{
	return { (unsigned char)rgb.get_pixel_r(), (unsigned char)rgb.get_pixel_g(), (unsigned char)rgb.get_pixel_b(), (unsigned char)get_pixel_a() };
}

constexpr void RGBA::set_pixel_rgba(PixelRGBA px_rgba)
{
	rgb.set_pixel_r(px_rgba.r);
	rgb.set_pixel_g(px_rgba.g);
	rgb.set_pixel_b(px_rgba.b);
	set_pixel_a(px_rgba.a);
}

struct HSVA
{
	HSV hsv;
	float alpha;

	constexpr HSVA(HSV hsv = HSV(), float alpha = 0.0f) : hsv(hsv), alpha(std::clamp(alpha, 0.0f, 1.0f)) {}
	constexpr HSVA(HSV hsv, int alpha) : hsv(hsv), alpha(std::clamp(alpha, 0, 255) * inv255) {}
	constexpr HSVA(float h, float s, float v, float a) : hsv(h, s, v), alpha(std::clamp(a, 0.0f, 1.0f)) {}
	constexpr HSVA(int h, int s, int v, int a) : hsv(h, s, v), alpha(std::clamp(a, 0, 255) * inv255) {}

	constexpr bool operator==(const HSVA&) const = default;

	glm::vec4 as_vec() const { return { hsv.h, hsv.s, hsv.v, alpha}; }

	constexpr int get_pixel_h() const { return hsv.get_pixel_h(); }
	constexpr int get_pixel_s() const { return hsv.get_pixel_s(); }
	constexpr int get_pixel_v() const { return hsv.get_pixel_v(); }
	constexpr int get_pixel_a() const { return roundi(alpha * 255); }
	constexpr void set_pixel_h(int h) { hsv.set_pixel_h(h); }
	constexpr void set_pixel_s(int s) { hsv.set_pixel_s(s); }
	constexpr void set_pixel_v(int v) { hsv.set_pixel_v(v); }
	constexpr void set_pixel_a(int a) { alpha = std::clamp(a, 0, 255) * inv255; }

	constexpr RGBA to_rgba() const;
	constexpr HSLA to_hsla() const;
};

struct HSLA
{
	HSL hsl;
	float alpha;

	constexpr HSLA(HSL hsl = HSL(), float alpha = 0.0f) : hsl(hsl), alpha(std::clamp(alpha, 0.0f, 1.0f)) {}
	constexpr HSLA(HSL hsl, int alpha) : hsl(hsl), alpha(std::clamp(alpha, 0, 255) * inv255) {}
	constexpr HSLA(float h, float s, float l, float a) : hsl(h, s, l), alpha(std::clamp(a, 0.0f, 1.0f)) {}
	constexpr HSLA(int h, int s, int l, int a) : hsl(h, s, l), alpha(std::clamp(a, 0, 255) * inv255) {}

	constexpr bool operator==(const HSLA&) const = default;

	glm::vec4 as_vec() const { return { hsl.h, hsl.s, hsl.l, alpha }; }

	constexpr int get_pixel_h() const { return hsl.get_pixel_h(); }
	constexpr int get_pixel_s() const { return hsl.get_pixel_s(); }
	constexpr int get_pixel_l() const { return hsl.get_pixel_l(); }
	constexpr int get_pixel_a() const { return roundi(alpha * 255); }
	constexpr void set_pixel_h(int h) { hsl.set_pixel_h(h); }
	constexpr void set_pixel_s(int s) { hsl.set_pixel_s(s); }
	constexpr void set_pixel_l(int l) { hsl.set_pixel_l(l); }
	constexpr void set_pixel_a(int a) { alpha = std::clamp(a, 0, 255) * inv255; }

	constexpr RGBA to_rgba() const;
	constexpr HSVA to_hsva() const;
};

constexpr HSVA RGBA::to_hsva() const
{
	return HSVA(rgb.to_hsv(), alpha);
}

constexpr HSLA RGBA::to_hsla() const
{
	return HSLA(rgb.to_hsl(), alpha);
}

constexpr RGBA HSVA::to_rgba() const
{
	return RGBA(hsv.to_rgb(), alpha);
}

constexpr HSLA HSVA::to_hsla() const
{
	return HSLA(hsv.to_hsl(), alpha);
}

constexpr RGBA HSLA::to_rgba() const
{
	return RGBA(hsl.to_rgb(), alpha);
}

constexpr HSVA HSLA::to_hsva() const
{
	return HSVA(hsl.to_hsv(), alpha);
}

struct ColorFrame
{
	float alpha = 1.0f;

private:
	RGB _rgb = RGB(1.0f, 1.0f, 1.0f);
	HSV _hsv = HSV(0.0f, 0.0f, 1.0f);
	HSL _hsl = HSL(0.0f, 0.0f, 1.0f);

public:
	constexpr ColorFrame(float alpha = 1.0f) : alpha(std::clamp(alpha, 0.0f, 1.0f)) {}
	constexpr ColorFrame(int alpha) : alpha(std::clamp(alpha, 0, 255) * inv255) {}
	constexpr ColorFrame(RGB rgb, float alpha = 1.0f) : alpha(std::clamp(alpha, 0.0f, 1.0f)) { set_rgb(rgb); }
	constexpr ColorFrame(HSV hsv, float alpha = 1.0f) : alpha(std::clamp(alpha, 0.0f, 1.0f)) { set_hsv(hsv); }
	constexpr ColorFrame(HSL hsl, float alpha = 1.0f) : alpha(std::clamp(alpha, 0.0f, 1.0f)) { set_hsl(hsl); }
	constexpr ColorFrame(RGB rgb, int alpha) : alpha(std::clamp(alpha, 0, 255) * inv255) { set_rgb(rgb); }
	constexpr ColorFrame(HSV hsv, int alpha) : alpha(std::clamp(alpha, 0, 255) * inv255) { set_hsv(hsv); }
	constexpr ColorFrame(HSL hsl, int alpha) : alpha(std::clamp(alpha, 0, 255) * inv255) { set_hsl(hsl); }
	constexpr ColorFrame(RGBA rgba) : alpha(rgba.alpha) { set_rgb(rgba.rgb); }
	constexpr ColorFrame(HSVA hsva) : alpha(hsva.alpha) { set_hsv(hsva.hsv); }
	constexpr ColorFrame(HSLA hsla) : alpha(hsla.alpha) { set_hsl(hsla.hsl); }

	constexpr bool operator==(const ColorFrame&) const = default;

	constexpr RGB rgb() const { return _rgb; }
	constexpr HSV hsv() const { return _hsv; }
	constexpr HSL hsl() const { return _hsl; }
	constexpr RGBA rgba() const { return RGBA(_rgb, alpha); }
	constexpr HSVA hsva() const { return HSVA(_hsv, alpha); }
	constexpr HSLA hsla() const { return HSLA(_hsl, alpha); }

	constexpr void set_rgb(RGB rgb) { _rgb = rgb; _hsv = rgb.to_hsv(); _hsl = rgb.to_hsl(); }
	constexpr void set_hsv(HSV hsv) { _hsv = hsv; _rgb = hsv.to_rgb(); _hsl = hsv.to_hsl(); }
	constexpr void set_hsl(HSL hsl) { _hsl = hsl; _rgb = hsl.to_rgb(); _hsv = hsl.to_hsv(); }

	constexpr int get_pixel_a() const { return roundi(alpha * 255); }
	constexpr void set_pixel_a(int a) { alpha = std::clamp(a, 0, 255) * inv255; }
};

struct RedGreen
{
	float r = 0.0f, g = 0.0f;

	constexpr bool operator==(const RedGreen&) const = default;
};

struct GrayScale
{
	float v = 0.0f;

	constexpr bool operator==(const GrayScale&) const = default;
};

extern float contrast_wb_value_simple_hue(float hue);
extern float contrast_wb_value_simple_hue_and_sat(float hue, float sat);
extern float contrast_wb_value_simple_hue_and_value(float hue, float value);
extern float contrast_wb_value_complex_hsv(HSV hsv);
extern float contrast_wb_value_complex_hsva(HSVA hsva);
extern float contrast_wb_value_simple_hue_and_lightness(float hue, float lightness);
extern float contrast_wb_value_complex_hsl(HSL hsv);
