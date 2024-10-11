#include "Color.h"

#include <algorithm>

constexpr float DELTA_TOLERANCE = 0.00001f;

static float alpha_of(const Color::ToAlphaConversion& conv, const glm::vec3& color)
{
	if (conv.mode == Color::ToAlphaConversionMode::ONE)
		return 1.0f;
	else if (conv.mode == Color::ToAlphaConversionMode::ZERO)
		return 0.0f;
	else if (conv.mode == Color::ToAlphaConversionMode::AVERAGE)
		return 0.333333f * (color.r + color.g + color.b);
	else if (conv.mode == Color::ToAlphaConversionMode::MIN)
		return std::min({ color.r, color.g, color.b });
	else if (conv.mode == Color::ToAlphaConversionMode::MAX)
		return std::max({ color.r, color.g, color.b });
	else
		return conv.a;
}

Color::RGBA Color::RGB::to_rgba(const ToAlphaConversion& conv) const
{
	return RGBA(r, g, b, alpha_of(conv, *this));
}

Color::HSV Color::RGB::to_hsv() const
{
	float max = std::max({ r, g, b });
	float min = std::min({ r, g, b });
	float delta = max - min;

	HSV hsv(0.0f, 0.0f, max);
	if (max > DELTA_TOLERANCE)
		hsv.s = delta / max;
	if (delta > DELTA_TOLERANCE)
	{
		if (max == r)
			hsv.h = (g - b) / delta;
		else if (max == g)
			hsv.h = 2.0f + (b - r) / delta;
		else
			hsv.h = 4.0f + (r - g) / delta;
		hsv.h *= 60.0f;
		if (hsv.h < 0.0f)
			hsv.h += 360.0f;
	}
	return hsv;
}

Color::HSL Color::RGB::to_hsl() const
{
	return to_hsv().to_hsl();
}

//Color::CMY Color::RGB::to_cmy() const
//{
//	// TODO
//	return CMY();
//}

Color::RGB Color::RGBA::to_rgb() const
{
	return RGB(r, g, b);
}

Color::HSVA Color::RGBA::to_hsva() const
{
	return to_rgb().to_hsv().to_hsva({ ToAlphaConversionMode::SOLID, a });
}

Color::HSLA Color::RGBA::to_hsla() const
{
	return to_rgb().to_hsv().to_hsl().to_hsla({ToAlphaConversionMode::SOLID, a});
}

//Color::CMYK Color::RGBA::to_cmyk() const
//{
//	// TODO
//	return CMYK();
//}

Color::HSVA Color::HSV::to_hsva(const ToAlphaConversion& conv) const
{
	return HSVA(h, s, v, alpha_of(conv, *this));
}

Color::RGB Color::HSV::to_rgb() const
{
	if (s < DELTA_TOLERANCE)
		return RGB(v, v, v);
	RGB rgb{};
	static constexpr float inv60 = 1 / 60.0f;
	int i = static_cast<int>(h * inv60);
	float f = (h * inv60) - i;
	float min = v * (1.0f - s);
	float pre = v * (1.0f - s * f);
	float post = v * (1.0f - s * (1.0f - f));
	switch (i)
	{
	case 0:
		rgb.r = v;
		rgb.g = post;
		rgb.b = min;
		break;
	case 1:
		rgb.g = v;
		rgb.r = pre;
		rgb.b = min;
		break;
	case 2:
		rgb.g = v;
		rgb.b = post;
		rgb.r = min;
		break;
	case 3:
		rgb.b = v;
		rgb.g = pre;
		rgb.r = min;
		break;
	case 4:
		rgb.b = v;
		rgb.r = post;
		rgb.g = min;
		break;
	case 5:
		rgb.r = v;
		rgb.b = pre;
		rgb.g = min;
		break;
	}
	return rgb;
}

Color::HSL Color::HSV::to_hsl() const
{
	HSL hsl(h, 0.0f, 0.0f);
	hsl.l = v * (1.0f - 0.5f * s);
	if (hsl.l > DELTA_TOLERANCE && hsl.l < 1.0f - DELTA_TOLERANCE)
		hsl.s = 0.5f * v * s / std::min(hsl.l, 1.0f - hsl.l);
	return hsl;
}

//Color::CMY Color::HSV::to_cmy() const
//{
//	return to_rgb().to_cmy();
//}

Color::HSV Color::HSVA::to_hsv() const
{
	return HSV(h, s, v);
}

Color::RGBA Color::HSVA::to_rgba() const
{
	return to_hsv().to_rgb().to_rgba({ ToAlphaConversionMode::SOLID, a });
}

Color::HSLA Color::HSVA::to_hsla() const
{
	return to_hsv().to_hsl().to_hsla({ ToAlphaConversionMode::SOLID, a });
}

//Color::CMYK Color::HSVA::to_cmyk() const
//{
//	// TODO
//}

Color::HSLA Color::HSL::to_hsla(const ToAlphaConversion& conv) const
{
	return HSLA(h, s, l, alpha_of(conv, *this));
}

Color::RGB Color::HSL::to_rgb() const
{
	return to_hsv().to_rgb();
}

Color::HSV Color::HSL::to_hsv() const
{
	HSV hsv(h, 0.0f, 0.0f);
	if (l < DELTA_TOLERANCE || l > 1.0f - DELTA_TOLERANCE)
		return hsv;
	hsv.v = l + s * std::min(l, 1.0f - l);
	hsv.s = 2.0f - 2.0f * l / hsv.v;
	return hsv;
}

//Color::CMY Color::HSL::to_cmy() const
//{
//	// TODO
//	return CMY();
//}

Color::HSL Color::HSLA::to_hsl() const
{
	return HSL(h, s, l);
}

Color::RGBA Color::HSLA::to_rgba() const
{
	return to_hsl().to_hsv().to_rgb().to_rgba({ ToAlphaConversionMode::SOLID, a });
}

Color::HSVA Color::HSLA::to_hsva() const
{
	return to_hsl().to_hsv().to_hsva({ ToAlphaConversionMode::SOLID, a });
}

//Color::CMYK Color::HSLA::to_cmyk() const
//{
//	// TODO
//	return CMYK();
//}
//
//Color::CMYK Color::CMY::to_cmyk(const ToAlphaConversion& conv) const
//{
//	// TODO
//	return CMYK();
//}
//
//Color::RGB Color::CMY::to_rgb() const
//{
//	// TODO
//	return RGB();
//}
//
//Color::HSV Color::CMY::to_hsv() const
//{
//	// TODO
//	return HSV();
//}
//
//Color::HSL Color::CMY::to_hsl() const
//{
//	// TODO
//	return HSL();
//}
//
//Color::CMY Color::CMYK::to_cmy() const
//{
//	// TODO
//	return CMY();
//}
//
//Color::RGBA Color::CMYK::to_rgba() const
//{
//	// TODO
//	return RGBA();
//}
//
//Color::HSVA Color::CMYK::to_hsva() const
//{
//	// TODO
//	return HSVA();
//}
//
//Color::HSLA Color::CMYK::to_hsla() const
//{
//	// TODO
//	return HSLA();
//}
