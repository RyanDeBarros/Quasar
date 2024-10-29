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
		GRAPHIC_QUAD,
		GRAPHIC_WHEEL,
		SLIDER_RGB,
		SLIDER_HSV,
		SLIDER_HSL,
		HEX_RGB,
		HEX_HSV,
		HEX_HSL
	} state = State::GRAPHIC_WHEEL;

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
	ColorFrame get_color() const;

private:
	void initialize_widget();
	void connect_mouse_handlers();

	void mouse_handler_graphic_quad(Position local_cursor_pos);
	void mouse_handler_graphic_hue_slider(Position local_cursor_pos);
	void mouse_handler_graphic_hue_wheel(Position local_cursor_pos);
	void mouse_handler_graphic_value_slider(Position local_cursor_pos);

	void send_graphic_quad_hue_to_uniform(float hue);
	void orient_graphic_hue_slider(Cardinal i) const;
	glm::vec2 get_graphic_quad_sat_and_value() const;
	float get_graphic_hue_slider_hue() const;

	void send_graphic_wheel_value_to_uniform(float value);
	glm::vec2 get_graphic_wheel_hue_and_sat() const;
	float get_graphic_value_slider_value() const;

	void setup_vertex_positions(size_t control) const;
	void setup_rect_uvs(size_t control) const;
	void setup_gradient(size_t control, GLint g1, GLint g2, GLint g3, GLint g4) const;
	void sync_cp_widget_transforms() const;
	void sync_single_cp_widget_transform(size_t control) const;
	void set_circle_cursor_thickness(size_t cursor, float thickness);
	void set_circle_cursor_value(size_t cursor, float value);
	float get_circle_cursor_value(size_t cursor);
	void setup_circle_cursor(size_t cursor);
};
