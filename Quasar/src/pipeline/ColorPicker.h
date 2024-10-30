#pragma once

#include <array>

#include <imgui/imgui.h>

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
	};

	enum class MainState
	{
		GRAPHIC,
		SLIDER,
		HEX
	};

	enum class SubState
	{
		QUAD,
		WHEEL,
		RGB,
		HSV,
		HSL
	};
private:
	State state = State::GRAPHIC_QUAD;
public:
	State get_state() const { return state; }
	void set_state(State state);

	Shader quad_shader, linear_hue_shader, hue_wheel_w_shader, circle_cursor_shader;

	Widget widget;
	Scale size;
private:
	Position center;
	State last_graphic_state = State::GRAPHIC_QUAD;
	State last_slider_state = State::SLIDER_RGB;
	State last_hex_state = State::HEX_RGB;
public:
	std::function<void(const Callback::MouseButton&)> clbk_mb;
	std::function<void(const Callback::MouseButton&)> clbk_mb_down;
	int current_widget_control = -1;

	ColorPicker();
	ColorPicker(const ColorPicker&) = delete;
	ColorPicker(ColorPicker&&) noexcept = delete;
	~ColorPicker();

	void render();
	void send_vp(const float* vp);
	ColorFrame get_color() const;
	void set_color(ColorFrame);
	void set_position(Position world_pos, Position screen_pos);

private:
	void cp_render_gui();
	static MainState main_state_of(State state);
	bool is_main_state(MainState main_state) const;
	bool is_sub_state(SubState sub_state) const;
	void cp_render_maintab_button(State& to_state, MainState main_state, State main_state_default, const char* display) const;
	void cp_render_subtab_button(State& to_state, State compare, const char* display) const;
	void initialize_widget();
	void connect_mouse_handlers();

	void mouse_handler_graphic_quad(Position local_cursor_pos);
	void mouse_handler_graphic_hue_slider(Position local_cursor_pos);
	void mouse_handler_graphic_hue_wheel(Position local_cursor_pos);
	void mouse_handler_graphic_value_slider(Position local_cursor_pos);

	void enact_graphic_quad_cursor_position(float hue, float sat, float val);
	void enact_graphic_hue_slider_cursor_position(float hue);
	void enact_graphic_quad_and_hue_slider_cursor_positions(Position local_cursor_pos);
	void enact_graphic_hue_wheel_cursor_position(float hue, float sat);
	void enact_graphic_value_slider_cursor_position(float hue, float value);
	void enact_graphic_hue_wheel_and_value_slider_cursor_positions(Position local_cursor_pos);

	void send_graphic_quad_hue_to_uniform(float hue);
	void orient_graphic_hue_slider(Cardinal i) const;
	glm::vec2 get_graphic_quad_sat_and_value() const;
	float get_graphic_hue_slider_hue() const;

	void send_graphic_wheel_value_to_uniform(float value);
	void send_graphic_value_slider_hue_and_sat_to_uniform(float hue, float sat);
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
