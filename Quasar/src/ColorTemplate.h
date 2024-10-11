#pragma once

#include <type_traits>
#include <concepts>
#include <algorithm>

#include "Macros.h"

struct Color{};
struct RGB : Color
{
	float r, g, b;

	RGB() = default;
	RGB(float r, float g, float b) : r(r), g(g), b(b) {}
	operator glm::vec3() const { return { r, g, b }; }

	void validate() { r = std::clamp(r, 0.0f, 1.0f); g = std::clamp(g, 0.0f, 1.0f); b = std::clamp(b, 0.0f, 1.0f); }
};
struct RGBA : Color {};
struct HSV : Color {};
struct HSVA : Color {};
struct HSL : Color {};
struct HSLA : Color {};
struct CMY : Color {};
struct CMYA : Color {};
struct CMYK : Color {};
struct CMYKA : Color {};

extern RGBA to_rgba(const RGB&);
extern RGB to_rgb(const RGBA&);
extern HSV to_hsv(const RGB&);
extern RGB to_rgb(const HSV&);
extern CMY to_cmy(const RGB&);
extern RGB to_rgb(const CMY&);
extern HSL to_hsl(const HSV&);
extern HSV to_hsv(const HSL&);
extern CMYK to_cmyk(const CMY&);
extern CMY to_cmy(const CMYK&);
extern HSVA to_hsva(const HSV&);
extern HSV to_hsv(const HSVA&);
extern HSLA to_hsla(const HSL&);
extern HSL to_hsl(const HSLA&);
extern CMYA to_cmya(const CMY&);
extern CMY to_cmy(const CMYA&);
extern CMYKA to_cmyka(const CMYK&);
extern CMYK to_cmyk(const CMYKA&);

template<typename T, typename... Ts>
constexpr bool is_any_of_v = (std::is_same_v<T, Ts> || ...);

template<typename From, typename To>
struct ColorConverter
{
};

template<typename From>
struct ColorConverter<From, RGB>
{
	RGB operator()(const RGBA& src) const { return to_rgb(src); }
	RGB operator()(const HSV& src) const { return to_rgb(src); }
	RGB operator()(const CMY& src) const { return to_rgb(src); }
	RGB operator()(const From& src) const
	{
		if constexpr (is_any_of_v<From, HSVA, HSL, HSLA>)
			return to_rgb(ColorConverter<From, HSV>{}(src));
		else if constexpr (is_any_of_v < From, CMYA, CMYK, CMYKA)
			return to_rgb(ColorConverter<From, CMY>{}(src));
		else
			static_assert(false, "Color conversion is not supported.");
	}
};

template<typename From>
struct ColorConverter<From, RGBA>
{
	RGBA operator()(const RGB& src) const { return to_rgba(src); }
	RGBA operator()(const From& src) const { return to_rgba(ColorConverter<From, RGB>{}(src)); }
};

template<typename From>
struct ColorConverter<From, HSV>
{
	HSV operator()(const HSVA& src) const { return to_hsv(src); }
	HSV operator()(const HSL& src) const { return to_hsv(src); }
	HSV operator()(const RGB& src) const { return to_hsv(src); }
	HSV operator()(const From& src) const
	{
		if constexpr (std::is_same_v<From, HSLA>)
			return to_hsv(ColorConverter<From, HSL>{}(src));
		else
			return to_hsv(ColorConverter<From, RGB>{}(src));
	}
};

template<typename From>
struct ColorConverter<From, HSVA>
{
	HSVA operator()(const HSV& src) const { return to_hsva(src); }
	HSVA operator()(const From& src) const { return to_hsva(ColorConverter<From, HSV>{}(src)); }
};

template<typename From>
struct ColorConverter<From, HSL>
{
	HSL operator()(const HSLA& src) const { return to_hsl(src); }
	HSL operator()(const HSV& src) const { return to_hsl(src); }
	HSL operator()(const From& src) const { return to_hsl(ColorConverter<From, HSV>{}(src)); }
};

template<typename From>
struct ColorConverter<From, HSLA>
{
	HSLA operator()(const HSL& src) const { return to_hsla(src); }
	HSLA operator()(const From& src) const { return to_hsla(ColorConverter<From, HSL>{}(src)); }
};

template<typename From>
struct ColorConverter<From, CMY>
{
	CMY operator()(const CMYA& src) const { return to_cmy(src); }
	CMY operator()(const CMYK& src) const { return to_cmy(src); }
	CMY operator()(const RGB& src) const { return to_cmy(src); }
	CMY operator()(const From& src) const
	{
		if constexpr (std::is_same_v<From, CMYKA>)
			return to_cmy(ColorConverter<From, CMYK>{}(src));
		else
			return to_cmy(ColorConverter<From, RGB>{}(src));
	}
};

template<typename From>
struct ColorConverter<From, CMYA>
{
	CMYA operator()(const CMY& src) const { return to_cmya(src); }
	CMYA operator()(const From& src) const { return to_cmya(ColorConverter<From, CMY>{}(src)); }
};

template<typename From>
struct ColorConverter<From, CMYK>
{
	CMYK operator()(const CMYKA& src) const { return to_cmyk(src); }
	CMYK operator()(const CMY& src) const { return to_cmyk(src); }
	CMYK operator()(const From& src) const { return to_cmyk(ColorConverter<From, CMY>{}(src)); }
};

template<typename From>
struct ColorConverter<From, CMYKA>
{
	CMYKA operator()(const CMYK& src) const { return to_cmyka(src); }
	CMYKA operator()(const From& src) const { return to_cmyka(ColorConverter<From, CMYK>{}(src)); }
};

template<typename From, typename To>
concept colors_are_convertible = requires(From src)
{
	requires std::is_base_of_v<Color, From> && std::is_base_of_v<Color, To>;
	{ ColorConverter<From, To>{}(src) } -> std::same_as<To>;
};

template<typename From, typename To>
To convert(From src) requires colors_are_convertible<From, To>
{
	if constexpr (std::is_same_v<From, To>)
		return To(src);
	return ColorConverter<From, To>{}(src);
}
