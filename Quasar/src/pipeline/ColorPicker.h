#pragma once

#include <array>

#include "user/Platform.h"
#include "Widgets.h"
#include "edit/Color.h"
#include "variety/Geometry.h"

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
	void initialize_widget();
	void connect_mouse_handlers();

	void setup_vertex_positions(size_t control) const;
	void setup_rect_uvs(size_t control) const;
	void setup_quad_gradient_color(size_t control, GLint gradient_index) const;
	void sync_cp_widget_transforms() const;
	void sync_single_cp_widget_transform(size_t control) const;
	void set_circle_cursor_thickness(size_t cursor, float thickness);
	void set_circle_cursor_value(size_t cursor, float value);
	float get_circle_cursor_value(size_t cursor);
	void setup_circle_cursor(size_t cursor);

	void send_rgb_quad_hue_to_uniform(float hue);
	void orient_rgb_hue_slider(Cardinal i) const;
	glm::vec2 get_rgb_quad_sat_and_value() const;
	float get_rgb_hue_slider_hue() const;

	void mouse_handler_rgb_quad(Position local_cursor_pos);
	void mouse_handler_rgb_hue_slider(Position local_cursor_pos);

public:
	ColorFrame get_color() const;
};
