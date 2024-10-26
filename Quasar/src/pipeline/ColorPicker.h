#pragma once

#include "UnitRenderable.h"

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

	UnitRenderable graphic_rgb_quad_gradient;
	UnitRenderable graphic_rgb_linear_hue;
	//UnitRenderable graphic_hsv_hue_wheel;
	//UnitRenderable graphic_hsv_value_gradient;
	//UnitMultiRenderable slider_rgb;
	//UnitMultiRenderable slider_hsv;
	//UnitMultiRenderable slider_hsl;
	//UnitMultiRenderable hex_rgb;
	//UnitMultiRenderable hex_hsv;
	//UnitMultiRenderable hex_hsl;

	ColorPicker();
	ColorPicker(const ColorPicker&) = delete;
	ColorPicker(ColorPicker&&) noexcept = delete;

	void render() const;
	void send_vp(const float* vp);
};
