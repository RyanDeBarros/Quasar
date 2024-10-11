#include "Color.h"

#include <algorithm>

constexpr float NEAR_ZERO = 0.00001f;

HSV RGB::to_hsv() const
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
	
	HSV hsv(0, 0, max_hex);
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

HSL RGB::to_hsl() const
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

RGB HSV::to_rgb() const
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

HSL HSV::to_hsl() const
{
	if (v == 0)
		return HSL(get_hue(), 0, 0);
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

RGB HSL::to_rgb() const
{
	// Chroma
	unsigned short hue = get_hue();
	float nl = l * inv255;
	float chroma = (1.0f - std::abs(2 * nl - 1.0f)) * sat_as_float();
	float x = chroma * (1.0f - std::abs(modulo((hue * inv60), 2.0f) - 1.0f));

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

HSV HSL::to_hsv() const
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
