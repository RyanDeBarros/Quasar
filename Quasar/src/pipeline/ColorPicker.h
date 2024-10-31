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

	Shader quad_shader, linear_hue_shader, hue_wheel_w_shader, linear_lightness_shader, circle_cursor_shader;

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
	void send_vp(const glm::mat3& vp) const;
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
	void take_over_cursor() const;
	void release_cursor();

	void mouse_handler_alpha_slider(Position local_cursor_pos);

	void mouse_handler_graphic_quad(Position local_cursor_pos);
	void mouse_handler_graphic_hue_slider(Position local_cursor_pos);
	
	void mouse_handler_graphic_hue_wheel(Position local_cursor_pos);
	void mouse_handler_graphic_value_slider(Position local_cursor_pos);

	void mouse_handler_slider_rgb_r(Position local_cursor_pos);
	void mouse_handler_slider_rgb_g(Position local_cursor_pos);
	void mouse_handler_slider_rgb_b(Position local_cursor_pos);

	void mouse_handler_slider_hsv_h(Position local_cursor_pos);
	void mouse_handler_slider_hsv_s(Position local_cursor_pos);
	void mouse_handler_slider_hsv_v(Position local_cursor_pos);

	void mouse_handler_slider_hsl_h(Position local_cursor_pos);
	void mouse_handler_slider_hsl_s(Position local_cursor_pos);
	void mouse_handler_slider_hsl_l(Position local_cursor_pos);

	void move_slider_cursor_x_absolute(size_t control, size_t cursor, float absolute);
	void move_slider_cursor_x_relative(size_t control, size_t cursor, float relative);

	void update_preview() const;

	void enact_alpha_slider_cursor_position();

	void enact_graphic_quad_cursor_position(float hue, float sat, float val) const;
	void enact_graphic_hue_slider_cursor_position(float hue) const;
	void enact_graphic_quad_and_hue_slider_cursor_positions(Position local_cursor_pos) const;

	void enact_graphic_hue_wheel_cursor_position(float hue, float sat) const;
	void enact_graphic_value_slider_cursor_position(float hue, float value) const;
	void enact_graphic_hue_wheel_and_value_slider_cursor_positions(Position local_cursor_pos) const;

	void enact_slider_rgb_cursor_positions() const;
	void enact_slider_hsv_cursor_positions() const;
	void enact_slider_hsl_cursor_positions() const;

	void orient_progress_slider(size_t control, Cardinal i) const;
	
	void send_graphic_quad_hue_to_uniform(float hue) const;
	glm::vec2 get_graphic_quad_sat_and_value() const;

	void send_graphic_wheel_value_to_uniform(float value) const;
	void send_graphic_value_slider_hue_and_sat_to_uniform(float hue, float sat) const;
	glm::vec2 get_graphic_wheel_hue_and_sat() const;

	void send_slider_hsv_hue_and_value_to_uniform(float hue, float value) const;
	void send_slider_hsl_hue_and_lightness_to_uniform(float hue, float lightness) const;

	float slider_normal_x(size_t control, size_t cursor) const;
	void setup_vertex_positions(size_t control) const;
	void setup_rect_uvs(size_t control) const;
	void setup_gradient(size_t control, GLint g1, GLint g2, GLint g3, GLint g4) const;
	void sync_cp_widget_transforms() const;
	void sync_single_cp_widget_transform(size_t control) const;
	void set_circle_cursor_thickness(size_t cursor, float thickness) const;
	void set_circle_cursor_value(size_t cursor, float value) const;
	float get_circle_cursor_value(size_t cursor) const;
	void setup_circle_cursor(size_t cursor);
};
