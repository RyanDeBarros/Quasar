#pragma once

#include <array>

#include "Widgets.h"

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

	Shader quad_shader, linear_hue_shader, hue_wheel_w_shader;

	static constexpr const char* MAX_GRADIENT_COLORS = "4";
	static constexpr GLint RGB_QUAD_GRADIENT_INDEX = 2;

	Widget widget;

	ColorPicker();
	ColorPicker(const ColorPicker&) = delete;
	ColorPicker(ColorPicker&&) noexcept = delete;

	void render() const;
	void send_vp(const float* vp);
};
