#include "Color.h"

#include <algorithm>

constexpr float NEAR_ZERO = 0.00001f;

HSV to_hsv(RGB rgb)
{
	// Normalize RGB
	float r = rgb.r * inv255;
	float g = rgb.g * inv255;
	float b = rgb.b * inv255;
	// Compute max, min and chroma
	unsigned char max_hex = std::max({ rgb.r, rgb.g, rgb.b });
	float max = max_hex * inv255;
	float min = std::min({ r, g, b });
	float chroma = max - min;
	
	HSV hsv(0, 0, max_hex);
	if (max_hex != 0)
		hsv.s = round_uchar(127 * chroma / max);
	if (chroma > NEAR_ZERO)
	{
		// Compute hue angle
		float hue = 0;
		if (max == r)
			hue = (g - b) / chroma;
		else if (max == g)
			hue = 2.0f + (b - r) / chroma;
		else
			hue = 4.0f + (r - g) / chroma;
		hue *= 60.0f;
		if (hue < 0.0f)
			hue += 360.0f;
		hsv.set_hue(round_ushort(hue));
	}
	return hsv;
}

RGB to_rgb(HSV hsv)
{
	if (hsv.s == 0)
		return RGB(hsv.v, hsv.v, hsv.v);
	// Normalize
	float s = hsv.sat_as_float();
	// Sextant index
	unsigned char si = hsv.get_hue() / 60;
	// Fractional part
	float fr = (hsv.get_hue() * inv60) - si;
	// Compute non-primary color characteristics
	unsigned char min = round_uchar(hsv.v * (1 - s));
	unsigned char pre = round_uchar(hsv.v * (1 - s * fr));
	unsigned char post = round_uchar(hsv.v * (1 - s * (1.0f - fr)));
	// Switch on sextant
	RGB rgb{};
	switch (si)
	{
	case 0:
		rgb.r = hsv.v;
		rgb.g = post;
		rgb.b = min;
		break;
	case 1:
		rgb.g = hsv.v;
		rgb.r = pre;
		rgb.b = min;
		break;
	case 2:
		rgb.g = hsv.v;
		rgb.b = post;
		rgb.r = min;
		break;
	case 3:
		rgb.b = hsv.v;
		rgb.g = pre;
		rgb.r = min;
		break;
	case 4:
		rgb.b = hsv.v;
		rgb.r = post;
		rgb.g = min;
		break;
	case 5:
		rgb.r = hsv.v;
		rgb.b = pre;
		rgb.g = min;
		break;
	}
	return rgb;
}

HSL to_hsl(RGB rgb)
{
	HSL hsl{};
	// Normalize RGB
	float r = rgb.r * inv255;
	float g = rgb.g * inv255;
	float b = rgb.b * inv255;
	// Compute max, min and chroma
	unsigned char max_hex = std::max({ rgb.r, rgb.g, rgb.b });
	unsigned char min_hex = std::min({ rgb.r, rgb.g, rgb.b });
	float chroma = (max_hex - min_hex) * inv255;
	// Lightness
	hsl.l = round_uchar((max_hex + min_hex) * 0.5f);
	if (chroma > NEAR_ZERO)
	{
		// Compute hue angle
		float hue = 0;
		if (max_hex == rgb.r)
			hue = (g - b) / chroma;
		else if (max_hex == rgb.g)
			hue = 2.0f + (b - r) / chroma;
		else
			hue = 4.0f + (r - g) / chroma;
		hue *= 60.0f;
		if (hue < 0.0f)
			hue += 360.0f;
		hsl.set_hue(round_ushort(hue));
		hsl.s = round_uchar(127 * (max_hex - min_hex) / (255.0f - std::abs(max_hex + min_hex - 255)));
	}
	return hsl;
}

RGB to_rgb(HSL hsl)
{
	// Chroma
	unsigned short h = hsl.get_hue();
	float l = hsl.l * inv255;
	float chroma = (1.0f - std::abs(2 * l - 1.0f)) * hsl.sat_as_float();
	float x = chroma * (1.0f - std::abs(fmod((h * inv60), 2.0f) - 1.0f));
	
	// RGB channels (unordered)
	unsigned char c1 = round_uchar(hsl.l + 255 * chroma * 0.5f);
	unsigned char c2 = round_uchar(hsl.l - 255 * chroma * 0.5f + 255 * x);
	unsigned char c3 = round_uchar(hsl.l - 255 * chroma * 0.5f);

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

HSV to_hsv(HSL hsl)
{
	// Normalize
	float sat_hsl = hsl.sat_as_float();
	float l = hsl.l * inv255;
	// Value
	float v = l + sat_hsl * std::min(l, 1.0f - l);
	// HSV saturation
	float sat_hsv = 0.0f;
	if (v > NEAR_ZERO)
		sat_hsv = 2.0f * (1.0f - l / v);
	return HSV(hsl.get_hue(),  round_uchar(127 * sat_hsv), round_uchar(255 * v));
}

HSL to_hsl(HSV hsv)
{
	if (hsv.v == 0)
		return HSL(hsv.get_hue(), 0, 0);
	// Normalize
	float sat_hsv = hsv.sat_as_float();
	float v = hsv.v * inv255;
	// Lightness
	float l = v * (1.0f - 0.5f * sat_hsv);
	// HSL saturation
	float sat_hsl = 0.0f;
	if (l < 1.0f - NEAR_ZERO)
		sat_hsl = 0.5f * v * sat_hsv / std::min(l, 1.0f - l);
	return HSL(hsv.get_hue(), round_uchar(127 * sat_hsl), round_uchar(255 * l));
}
