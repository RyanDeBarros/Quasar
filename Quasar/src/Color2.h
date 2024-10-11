#pragma once

#include <glm/glm.hpp>

namespace Color
{
	struct RGB;
	struct RGBA;
	struct HSV;
	struct HSVA;
	struct HSL;
	struct HSLA;
	struct CMY;
	struct CMYK;

	enum class ToAlphaConversionMode
	{
		SOLID,
		ONE,
		ZERO,
		AVERAGE,
		MIN,
		MAX
	};

	struct ToAlphaConversion
	{
		ToAlphaConversionMode mode;
		float a = 1.0f;
	};

	/// R[0,1] G[0,1] B[0,1]
	struct RGB
	{
		float r, g, b;

		RGB() = default;
		RGB(float r, float g, float b) : r(r), g(g), b(b) {}
		operator glm::vec3() const { return { r, g, b }; }

		void validate() { r = std::clamp(r, 0.0f, 1.0f); g = std::clamp(g, 0.0f, 1.0f); b = std::clamp(b, 0.0f, 1.0f); }

		RGBA to_rgba(const ToAlphaConversion& conv) const;
		HSV to_hsv() const;
		HSL to_hsl() const;
		//CMY to_cmy() const;
	};

	/// R[0,1] G[0,1] B[0,1] A[0,1]
	struct RGBA
	{
		float r, g, b, a;

		RGBA() = default;
		RGBA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
		operator glm::vec4() const { return { r, g, b, a }; }

		void validate() { r = std::clamp(r, 0.0f, 1.0f); g = std::clamp(g, 0.0f, 1.0f); b = std::clamp(b, 0.0f, 1.0f); a = std::clamp(a, 0.0f, 1.0f); }

		RGB to_rgb() const;
		HSVA to_hsva() const;
		HSLA to_hsla() const;
		//CMYK to_cmyk() const;
	};

	/// H[0,360] S[0,1] V[0,1]
	struct HSV
	{
		float h, s, v;

		HSV() = default;
		HSV(float h, float s, float v) : h(h), s(s), v(v) {}
		operator glm::vec3() const { return { h, s, v }; }

		void validate() { h = std::clamp(h, 0.0f, 360.0f); s = std::clamp(s, 0.0f, 1.0f); v = std::clamp(v, 0.0f, 1.0f); }

		HSVA to_hsva(const ToAlphaConversion& conv) const;
		RGB to_rgb() const;
		HSL to_hsl() const;
		//CMY to_cmy() const;
	};

	/// H[0,360] S[0,1] V[0,1] A[0,1]
	struct HSVA
	{
		float h, s, v, a;

		HSVA() = default;
		HSVA(float h, float s, float v, float a) : h(h), s(s), v(v), a(a) {}
		operator glm::vec4() const { return { h, s, v, a }; }

		void validate() { h = std::clamp(h, 0.0f, 360.0f); s = std::clamp(s, 0.0f, 1.0f); v = std::clamp(v, 0.0f, 1.0f); a = std::clamp(a, 0.0f, 1.0f); }

		HSV to_hsv() const;
		RGBA to_rgba() const;
		HSLA to_hsla() const;
		//CMYK to_cmyk() const;
	};

	/// H[0,360] S[0,1] L[0,1]
	struct HSL
	{
		float h, s, l;

		HSL() = default;
		HSL(float h, float s, float l) : h(h), s(s), l(l) {}
		operator glm::vec3() const { return { h, s, l }; }

		void validate() { h = std::clamp(h, 0.0f, 360.0f); s = std::clamp(s, 0.0f, 1.0f); l = std::clamp(l, 0.0f, 1.0f); }

		HSLA to_hsla(const ToAlphaConversion& conv) const;
		RGB to_rgb() const;
		HSV to_hsv() const;
		//CMY to_cmy() const;
	};

	/// H[0,360] S[0,1] L[0,1] A[0,1]
	struct HSLA
	{
		float h, s, l, a;

		HSLA() = default;
		HSLA(float h, float s, float l, float a) : h(h), s(s), l(l), a(a) {}
		operator glm::vec4() const { return { h, s, l, a }; }

		void validate() { h = std::clamp(h, 0.0f, 360.0f); s = std::clamp(s, 0.0f, 1.0f); l = std::clamp(l, 0.0f, 1.0f); a = std::clamp(a, 0.0f, 1.0f); }

		HSL to_hsl() const;
		RGBA to_rgba() const;
		HSVA to_hsva() const;
		//CMYK to_cmyk() const;
	};

	///// C[0,1] M[0,1] Y[0,1]
	//struct CMY
	//{
	//	float c, m, y;

	//	CMY() = default;
	//	CMY(float c, float m, float y) : c(c), m(m), y(y) {}
	//	operator glm::vec3() const { return { c, m, y }; }

	//	void validate() { c = std::clamp(c, 0.0f, 1.0f); m = std::clamp(m, 0.0f, 1.0f); y = std::clamp(y, 0.0f, 1.0f); }

	//	CMYK to_cmyk(const ToAlphaConversion& conv) const;
	//	RGB to_rgb() const;
	//	HSV to_hsv() const;
	//	HSL to_hsl() const;
	//};

	///// C[0,1] M[0,1] Y[0,1] K[0,1]
	//struct CMYK
	//{
	//	float c, m, y, k;

	//	CMYK() = default;
	//	CMYK(float c, float m, float y, float k) : c(c), m(m), y(y), k(k) {}
	//	operator glm::vec4() const { return { c, m, y, k }; }

	//	void validate() { c = std::clamp(c, 0.0f, 1.0f); m = std::clamp(m, 0.0f, 1.0f); y = std::clamp(y, 0.0f, 1.0f); k = std::clamp(k, 0.0f, 1.0f); }

	//	CMY to_cmy() const;
	//	RGBA to_rgba() const;
	//	HSVA to_hsva() const;
	//	HSLA to_hsla() const;
	//};
}
