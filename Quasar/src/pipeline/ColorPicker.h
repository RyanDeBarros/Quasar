#pragma once

#include <array>

#include "user/Platform.h"
#include "Widgets.h"
#include "edit/Color.h"

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

	Shader quad_shader, linear_hue_shader, hue_wheel_w_shader, circle_cursor_shader;

	Widget widget;
	std::function<void(const Callback::MouseButton&)> clbk_mb;
	std::function<void(const Callback::MouseButton&)> clbk_mb_down;
	int current_widget_control = -1;

	ColorPicker();
	ColorPicker(const ColorPicker&) = delete;
	ColorPicker(ColorPicker&&) noexcept = delete;
	~ColorPicker();

	void render() const;
	void send_vp(const float* vp);

private:
	void sync_cp_widget_transforms();
	void sync_single_cp_widget_transform(size_t i);
	glm::vec2 get_rgb_quad_sat_and_value() const;
	float get_rgb_hue_slider_hue() const;

	void mouse_handler_rgb_quad(Position local_cursor_pos);
	void mouse_handler_rgb_hue_slider(Position local_cursor_pos);

public:
	ColorFrame get_color() const;
};
