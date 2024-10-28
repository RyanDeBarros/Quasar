#pragma once

#include <array>

#include "UnitRenderable.h"
#include "variety/Geometry.h"

// TODO move away from templates. make more general (not specifically UnitRenderable wrapper).
template<size_t n>
struct CPWidget
{
	FlatTransform parent;
	std::array<WidgetPlacement, n> wps;
	std::array<UnitRenderable*, n> urs;

	template<typename... Shaders>
	CPWidget(Shaders&... shaders)
	{
		static_assert(sizeof...(shaders) == n, "Number of shaders in constructor list must match number of UnitRenderables");
		size_t i = 0;
		((urs[i++] = new UnitRenderable(shaders)), ...);
	}
	template<size_t m>
	CPWidget(const CPWidget<m>&) = delete;
	template<size_t m>
	CPWidget(CPWidget<m>&&) noexcept = delete;
	~CPWidget()
	{
		for (size_t i = 0; i < n; ++i)
			delete urs[i];
	}

	UnitRenderable& operator[](size_t i) { return *urs[i]; }
	const UnitRenderable& operator[](size_t i) const { return *urs[i]; }
};

struct ColorPicker
{
	enum class State
	{
		GRAPHIC_RGB,
		GRAPHIC_HSV,
		SLIDER_RGB,
		SLIDER_HSV,
		SLIDER_HSL,
		HEX_RGB,
		HEX_HSV,
		HEX_HSL
	} state = State::GRAPHIC_RGB;

	static constexpr const char* MAX_GRADIENT_COLORS = "4";
	static constexpr GLint RGB_QUAD_GRADIENT_INDEX = 2;

	Shader quad_shader, linear_hue_shader, hue_wheel_w_shader;

	CPWidget<2> graphic_rgb;
	//FlatTransform pftr_graphic_rgb{};
	//std::array<WidgetPlacement, 2> wp_graphic_rgb;
	//std::array<UnitRenderable, 2> graphic_rgb;
	
	//std::array<UnitRenderable, 2> graphic_hsv;
	//std::array<UnitMultiRenderable, 3> slider;
	//std::array<UnitMultiRenderable, 3> hex;

	ColorPicker();
	ColorPicker(const ColorPicker&) = delete;
	ColorPicker(ColorPicker&&) noexcept = delete;

	void render() const;
	void send_vp(const float* vp);
};
