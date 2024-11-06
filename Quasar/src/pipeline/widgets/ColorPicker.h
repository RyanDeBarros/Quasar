#pragma once

#include <array>

#include <imgui/imgui.h>

#include "user/Platform.h"
#include "Widgets.h"
#include "edit/Color.h"
#include "variety/Geometry.h"

struct ColorPicker : public Widget
{
	enum class State
	{
		GRAPHIC_QUAD,
		GRAPHIC_WHEEL,
		SLIDER_RGB,
		SLIDER_HSV,
		SLIDER_HSL,
	};

private:
	State state = State::GRAPHIC_QUAD;
public:
	State get_state() const { return state; }
	void set_state(State state);

	Shader quad_shader, linear_hue_shader, hue_wheel_w_shader, linear_lightness_shader, circle_cursor_shader, round_rect_shader;

private:
	Position center;
	glm::mat3 vp;
	State last_graphic_state = State::GRAPHIC_QUAD;
	enum class TextFieldMode
	{
		NUMBER,
		PERCENT
	} txtfld_mode = TextFieldMode::NUMBER;
	static const size_t rgb_hex_size = 7;
	char rgb_hex_prev[rgb_hex_size] = "FFFFFF";
	char rgb_hex[rgb_hex_size] = "FFFFFF";

	MouseButtonHandler& parent_mb_handler;
	MouseButtonHandler mb_handler;
	MouseButtonHandler imgui_mb_handler;
	KeyHandler& parent_key_handler;
	KeyHandler key_handler;
	
	bool imgui_takeover_mb = false;
	bool imgui_takeover_key = false;
	
	int current_widget_control = -1;

public:
	ColorPicker(MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler);
	ColorPicker(const ColorPicker&) = delete;
	ColorPicker(ColorPicker&&) noexcept = delete;
	~ColorPicker();

	void render();
	void send_vp(const glm::mat3& vp);
	ColorFrame get_color() const;
	void set_color(ColorFrame);
	void set_size(Scale size, bool sync = false);
	void set_position(Position world_pos, Position screen_pos);

private:
	void process_mb_down_events();

	void cp_render_gui_back();
	void cp_render_gui_front();
	void cp_render_tab_button(State& to_state, State state, bool disable, const char* display) const;
	void update_rgb_hex();

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
	void move_slider_cursor_y_absolute(size_t control, size_t cursor, float absolute);
	void move_slider_cursor_y_relative(size_t control, size_t cursor, float relative);

	void update_display_colors();

	void orient_progress_slider(size_t control, Cardinal i) const;
	
	void send_graphic_quad_hue_to_uniform(float hue) const;
	glm::vec2 get_graphic_quad_sat_and_value() const;

	void send_graphic_wheel_value_to_uniform(float value) const;
	void send_graphic_value_slider_hue_and_sat_to_uniform(float hue, float sat) const;
	glm::vec2 get_graphic_wheel_hue_and_sat() const;

	void send_slider_hsv_hue_and_value_to_uniform(float hue, float value) const;
	void send_slider_hsl_hue_and_lightness_to_uniform(float hue, float lightness) const;

	float slider_normal_x(size_t control, size_t cursor) const;
	float slider_normal_y(size_t control, size_t cursor) const;

	void setup_vertex_positions(size_t control) const;
	void setup_rect_uvs(size_t control) const;
	void setup_gradient(size_t control, GLint g1, GLint g2, GLint g3, GLint g4) const;
	void sync_cp_widget_with_vp();
	void sync_single_cp_widget_transform_ur(size_t control, bool send_buffer = true) const;
	void send_cpwc_buffer(size_t control) const;
	void set_circle_cursor_thickness(size_t cursor, float thickness) const;
	void set_circle_cursor_value(size_t cursor, float value) const;
	float get_circle_cursor_value(size_t cursor) const;
	void setup_circle_cursor(size_t cursor);

private:
	// LATER use UMR when possible
	enum
	{
		// LATER ? use one CURSORS UMR, and implement visibility for subshapes in UMR.
		PREVIEW,						// quad
		ALPHA_SLIDER,					// quad
		ALPHA_SLIDER_CURSOR,			// circle_cursor
		GRAPHIC_QUAD,					// quad
		GRAPHIC_QUAD_CURSOR,			// circle_cursor
		GRAPHIC_HUE_SLIDER,				// linear_hue
		GRAPHIC_HUE_SLIDER_CURSOR,		// circle_cursor
		GRAPHIC_HUE_WHEEL,				// hue_wheel
		GRAPHIC_HUE_WHEEL_CURSOR,		// circle_cursor
		GRAPHIC_VALUE_SLIDER,			// quad
		GRAPHIC_VALUE_SLIDER_CURSOR,	// circle_cursor
		RGB_R_SLIDER,					// quad
		RGB_R_SLIDER_CURSOR,			// circle_cursor
		RGB_G_SLIDER,					// quad
		RGB_G_SLIDER_CURSOR,			// circle_cursor
		RGB_B_SLIDER,					// quad
		RGB_B_SLIDER_CURSOR,			// circle_cursor
		HSV_H_SLIDER,					// quad
		HSV_H_SLIDER_CURSOR,			// circle_cursor
		HSV_S_SLIDER,					// quad
		HSV_S_SLIDER_CURSOR,			// circle_cursor
		HSV_V_SLIDER,					// quad
		HSV_V_SLIDER_CURSOR,			// circle_cursor
		HSL_H_SLIDER,					// quad
		HSL_H_SLIDER_CURSOR,			// circle_cursor
		HSL_S_SLIDER,					// quad
		HSL_S_SLIDER_CURSOR,			// circle_cursor
		HSL_L_SLIDER,					// linear_lightness
		HSL_L_SLIDER_CURSOR,			// circle_cursor
		BACKGROUND,						// separate widget
		TEXT_ALPHA,						// separate widget
		TEXT_RED,						// separate widget
		TEXT_GREEN,						// separate widget
		TEXT_BLUE,						// separate widget
		TEXT_HUE,						// separate widget
		TEXT_SAT,						// separate widget
		TEXT_VALUE,						// separate widget
		TEXT_LIGHT,						// separate widget
		BUTTON_SWITCH_TXTFLD_MODE,		// separate widget
		_W_COUNT
	};

	enum class GradientIndex : GLint
	{
		BLACK,
		WHITE,
		TRANSPARENT,
		PREVIEW,
		ALPHA_SLIDER,
		GRAPHIC_QUAD,
		GRAPHIC_VALUE_SLIDER,
		RGB_R_SLIDER,
		RGB_G_SLIDER,
		RGB_B_SLIDER,
		HSV_S_SLIDER_ZERO,
		HSV_S_SLIDER_ONE,
		HSV_V_SLIDER,
		HSL_S_SLIDER_ZERO,
		HSL_S_SLIDER_ONE,
		_MAX_GRADIENT_COLORS
	};

	static void send_gradient_color_uniform(const Shader& shader, GradientIndex index, ColorFrame color);
};
