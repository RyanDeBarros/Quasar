#include "ColorPicker.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "edit/Color.h"
#include "user/Machine.h"
#include "user/GUI.h"

enum class GradientIndex : GLint
{
	BLACK,
	WHITE,
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

static void send_gradient_color_uniform(Shader& shader, GradientIndex index, ColorFrame color)
{
	bind_shader(shader);
	QUASAR_GL(glUniform4fv(shader.uniform_locations["u_GradientColors[0]"] + int(index), 1, glm::value_ptr(color.rgba_as_vec())));
}

// LATER use UMR when possible
enum
{
	// LATER ? use one CURSORS UMR, and implement visibility for subshapes in UMR.
	ALPHA_SLIDER,					// quad
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
	_CPWC_COUNT
};

ColorPicker::ColorPicker()
	: quad_shader(FileSystem::resources_path("gradients/quad.vert"), FileSystem::resources_path("gradients/quad.frag.tmpl"),
		{ {"$MAX_GRADIENT_COLORS", std::to_string((int)GradientIndex::_MAX_GRADIENT_COLORS) } }),
	linear_hue_shader(FileSystem::resources_path("gradients/linear_hue.vert"), FileSystem::resources_path("gradients/linear_hue.frag")),
	hue_wheel_w_shader(FileSystem::resources_path("gradients/hue_wheel_w.vert"), FileSystem::resources_path("gradients/hue_wheel_w.frag")),
	linear_lightness_shader(FileSystem::resources_path("gradients/linear_lightness.vert"), FileSystem::resources_path("gradients/linear_lightness.frag")),
	circle_cursor_shader(FileSystem::resources_path("circle_cursor.vert"), FileSystem::resources_path("circle_cursor.frag")),
	widget(_CPWC_COUNT)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::BLACK, ColorFrame(RGB::precise_from_hsv(0.0f, 0.0f, 0.0f)));
	send_gradient_color_uniform(quad_shader, GradientIndex::WHITE, ColorFrame(RGB::precise_from_hsv(0.0f, 0.0f, 1.0f)));
	initialize_widget();
	connect_mouse_handlers();
	set_color(ColorFrame());
}

ColorPicker::~ColorPicker()
{
	Machine.main_window->clbk_mouse_button.remove_at_owner(this);
	Machine.main_window->clbk_mouse_button_down.remove_at_owner(this);
}

void ColorPicker::render()
{
	cp_render_gui();
	switch (state)
	{
	case State::GRAPHIC_QUAD:
		ur_wget(widget, GRAPHIC_QUAD).draw();
		ur_wget(widget, GRAPHIC_QUAD_CURSOR).draw();
		ur_wget(widget, GRAPHIC_HUE_SLIDER).draw();
		ur_wget(widget, GRAPHIC_HUE_SLIDER_CURSOR).draw();
		break;
	case State::GRAPHIC_WHEEL:
		ur_wget(widget, GRAPHIC_HUE_WHEEL).draw();
		ur_wget(widget, GRAPHIC_HUE_WHEEL_CURSOR).draw();
		ur_wget(widget, GRAPHIC_VALUE_SLIDER).draw();
		ur_wget(widget, GRAPHIC_VALUE_SLIDER_CURSOR).draw();
		break;
	case State::SLIDER_RGB:
		ur_wget(widget, RGB_R_SLIDER).draw();
		ur_wget(widget, RGB_R_SLIDER_CURSOR).draw();
		ur_wget(widget, RGB_G_SLIDER).draw();
		ur_wget(widget, RGB_G_SLIDER_CURSOR).draw();
		ur_wget(widget, RGB_B_SLIDER).draw();
		ur_wget(widget, RGB_B_SLIDER_CURSOR).draw();
		break;
	case State::SLIDER_HSV:
		ur_wget(widget, HSV_H_SLIDER).draw();
		ur_wget(widget, HSV_H_SLIDER_CURSOR).draw();
		ur_wget(widget, HSV_S_SLIDER).draw();
		ur_wget(widget, HSV_S_SLIDER_CURSOR).draw();
		ur_wget(widget, HSV_V_SLIDER).draw();
		ur_wget(widget, HSV_V_SLIDER_CURSOR).draw();
		break;
	case State::SLIDER_HSL:
		ur_wget(widget, HSL_H_SLIDER).draw();
		ur_wget(widget, HSL_H_SLIDER_CURSOR).draw();
		ur_wget(widget, HSL_S_SLIDER).draw();
		ur_wget(widget, HSL_S_SLIDER_CURSOR).draw();
		ur_wget(widget, HSL_L_SLIDER).draw();
		ur_wget(widget, HSL_L_SLIDER_CURSOR).draw();
		break;
	case State::HEX_RGB:
		break;
	case State::HEX_HSV:
		break;
	case State::HEX_HSL:
		break;
	}
}

void ColorPicker::cp_render_gui()
{
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::SetNextWindowBgAlpha(0);
	auto sz = size;
	ImGui::SetNextWindowSize(ImVec2(sz.x, sz.y));
	Position pos = center - Position{ 0.5f * sz.x, 0.8f * sz.y };
	ImGui::SetNextWindowPos({ pos.x, pos.y });

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration;
	if (ImGui::Begin("-", nullptr, window_flags))
	{
		State to_state = state;
		if (ImGui::BeginTabBar("cp-main-tb"))
		{
			cp_render_maintab_button(to_state, MainState::GRAPHIC, last_graphic_state, "GRAPHIC");
			cp_render_maintab_button(to_state, MainState::SLIDER, last_slider_state, "SLIDER");
			cp_render_maintab_button(to_state, MainState::HEX, last_hex_state, "HEX");
			ImGui::EndTabBar();
		}
		ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 32));
		if (ImGui::BeginTabBar("cp-sub-tb"))
		{
			if (is_main_state(MainState::GRAPHIC))
			{
				cp_render_subtab_button(to_state, State::GRAPHIC_QUAD, "QUAD");
				cp_render_subtab_button(to_state, State::GRAPHIC_WHEEL, "WHEEL");
			}
			else if (is_main_state(MainState::SLIDER))
			{
				cp_render_subtab_button(to_state, State::SLIDER_RGB, "RGB");
				cp_render_subtab_button(to_state, State::SLIDER_HSV, "HSV");
				cp_render_subtab_button(to_state, State::SLIDER_HSL, "HSL");
			}
			else if (is_main_state(MainState::HEX))
			{
				cp_render_subtab_button(to_state, State::HEX_RGB, "RGB");
				cp_render_subtab_button(to_state, State::HEX_HSV, "HSV");
				cp_render_subtab_button(to_state, State::HEX_HSL, "HSL");
			}
			ImGui::EndTabBar();
		}
		set_state(to_state);
		ImGui::End();
	}
}

ColorPicker::MainState ColorPicker::main_state_of(State state)
{
	if (state == State::SLIDER_RGB || state == State::SLIDER_HSV || state == State::SLIDER_HSL)
		return MainState::SLIDER;
	else if (state == State::HEX_RGB || state == State::HEX_HSV || state == State::HEX_HSL)
		return MainState::HEX;
	else
		return MainState::GRAPHIC;
}

bool ColorPicker::is_main_state(MainState main) const
{
	return (main == MainState::GRAPHIC && (state == State::GRAPHIC_QUAD || state == State::GRAPHIC_WHEEL))
		|| (main == MainState::SLIDER && (state == State::SLIDER_RGB || state == State::SLIDER_HSV || state == State::SLIDER_HSL))
		|| (main == MainState::HEX && (state == State::HEX_RGB || state == State::HEX_HSV || state == State::HEX_HSL));
}

bool ColorPicker::is_sub_state(SubState sub) const
{
	return (sub == SubState::QUAD && state == State::GRAPHIC_QUAD)
		|| (sub == SubState::WHEEL && state == State::GRAPHIC_WHEEL)
		|| (sub == SubState::RGB && (state == State::SLIDER_RGB || state == State::HEX_RGB))
		|| (sub == SubState::HSV && (state == State::SLIDER_HSV || state == State::HEX_HSV))
		|| (sub == SubState::HSL && (state == State::SLIDER_HSL || state == State::HEX_HSL));
}

void ColorPicker::cp_render_maintab_button(State& to_state, MainState main_state, State main_state_default, const char* display) const
{
	bool is_main = is_main_state(main_state);
	if (is_main)
		ImGui::BeginDisabled();
	if (ImGui::TabItemButton(display))
	{
		if (!is_main)
			to_state = main_state_default;
	}
	if (is_main)
		ImGui::EndDisabled();
}

void ColorPicker::cp_render_subtab_button(State& to_state, State compare, const char* display) const
{
	if (state == compare)
		ImGui::BeginDisabled();
	if (ImGui::TabItemButton(display))
		to_state = compare;
	if (state == compare)
		ImGui::EndDisabled();
}

void ColorPicker::set_state(State _state)
{
	if (_state != state)
	{
		if (main_state_of(state) == MainState::GRAPHIC)
		{
			if (main_state_of(_state) == MainState::GRAPHIC)
				last_graphic_state = _state;
			else
				last_graphic_state = state;
		}
		else if (main_state_of(state) == MainState::SLIDER)
		{
			if (main_state_of(_state) == MainState::SLIDER)
				last_slider_state = _state;
			else
				last_slider_state = state;
		}
		else if (main_state_of(state) == MainState::HEX)
		{
			if (main_state_of(_state) == MainState::HEX)
				last_hex_state = _state;
			else
				last_hex_state = state;
		}

		release_cursor();
		ColorFrame color = get_color();
		state = _state;
		set_color(color);
	}
}

void ColorPicker::send_vp(const float* vp)
{
	bind_shader(quad_shader);
	QUASAR_GL(glUniformMatrix3fv(quad_shader.uniform_locations["u_VP"], 1, GL_FALSE, vp));
	bind_shader(linear_hue_shader);
	QUASAR_GL(glUniformMatrix3fv(linear_hue_shader.uniform_locations["u_VP"], 1, GL_FALSE, vp));
	bind_shader(hue_wheel_w_shader);
	QUASAR_GL(glUniformMatrix3fv(hue_wheel_w_shader.uniform_locations["u_VP"], 1, GL_FALSE, vp));
	bind_shader(linear_lightness_shader);
	QUASAR_GL(glUniformMatrix3fv(linear_lightness_shader.uniform_locations["u_VP"], 1, GL_FALSE, vp));
	bind_shader(circle_cursor_shader);
	QUASAR_GL(glUniformMatrix3fv(circle_cursor_shader.uniform_locations["u_VP"], 1, GL_FALSE, vp));
	sync_cp_widget_transforms();
}

void ColorPicker::initialize_widget()
{
	// ---------- GRAPHIC QUAD ----------

	widget.hobjs[GRAPHIC_QUAD] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[GRAPHIC_QUAD_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[GRAPHIC_HUE_SLIDER] = new WP_UnitRenderable(linear_hue_shader);
	widget.hobjs[GRAPHIC_HUE_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);

	setup_rect_uvs(GRAPHIC_QUAD);
	setup_gradient(GRAPHIC_QUAD, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::WHITE, (GLint)GradientIndex::GRAPHIC_QUAD);
	send_graphic_quad_hue_to_uniform(0.0f);
	setup_circle_cursor(GRAPHIC_QUAD_CURSOR);
	orient_progress_slider(GRAPHIC_HUE_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(GRAPHIC_HUE_SLIDER_CURSOR);

	widget.wp_at(GRAPHIC_QUAD).transform.position = { 0, -100 };
	widget.wp_at(GRAPHIC_QUAD).transform.scale = { 200, 200 };
	widget.wp_at(GRAPHIC_QUAD).pivot.y = 0;
	widget.wp_at(GRAPHIC_QUAD_CURSOR).transform.position = { widget.wp_at(GRAPHIC_QUAD).top(), widget.wp_at(GRAPHIC_QUAD).right() };
	widget.wp_at(GRAPHIC_HUE_SLIDER).transform.position = { 0, -115 };
	widget.wp_at(GRAPHIC_HUE_SLIDER).transform.scale = { 200, 15 };
	widget.wp_at(GRAPHIC_HUE_SLIDER).pivot.y = 1;
	widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position = { widget.wp_at(GRAPHIC_HUE_SLIDER).left(), widget.wp_at(GRAPHIC_HUE_SLIDER).center_y() };

	// ---------- GRAPHIC WHEEL ----------

	widget.hobjs[GRAPHIC_HUE_WHEEL] = new WP_UnitRenderable(hue_wheel_w_shader);
	widget.hobjs[GRAPHIC_HUE_WHEEL_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[GRAPHIC_VALUE_SLIDER] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[GRAPHIC_VALUE_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);

	setup_rect_uvs(GRAPHIC_HUE_WHEEL);
	send_graphic_wheel_value_to_uniform(1.0f);
	setup_circle_cursor(GRAPHIC_HUE_WHEEL_CURSOR);
	setup_rect_uvs(GRAPHIC_VALUE_SLIDER);
	setup_gradient(GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER); // LATER also put in orient method
	send_graphic_value_slider_hue_and_sat_to_uniform(0.0f, 0.0f);
	setup_circle_cursor(GRAPHIC_VALUE_SLIDER_CURSOR);
	set_circle_cursor_value(GRAPHIC_VALUE_SLIDER_CURSOR, 0.0f);

	widget.wp_at(GRAPHIC_HUE_WHEEL).transform.position = { 0, -100 };
	widget.wp_at(GRAPHIC_HUE_WHEEL).transform.scale = { 200, 200 };
	widget.wp_at(GRAPHIC_HUE_WHEEL).pivot.y = 0;
	widget.wp_at(GRAPHIC_VALUE_SLIDER).transform.position = { 0, -115 };
	widget.wp_at(GRAPHIC_VALUE_SLIDER).transform.scale = { 200, 15 };
	widget.wp_at(GRAPHIC_VALUE_SLIDER).pivot.y = 1;
	widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position = { widget.wp_at(GRAPHIC_VALUE_SLIDER).right(), widget.wp_at(GRAPHIC_VALUE_SLIDER).center_y() };

	// ---------- RGB SLIDER ----------

	widget.hobjs[RGB_R_SLIDER] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[RGB_R_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[RGB_G_SLIDER] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[RGB_G_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[RGB_B_SLIDER] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[RGB_B_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);

	setup_rect_uvs(RGB_R_SLIDER);
	setup_circle_cursor(RGB_R_SLIDER_CURSOR);
	setup_gradient(RGB_R_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_R_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_R_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_R_SLIDER, RGB(0xFF0000));
	setup_rect_uvs(RGB_G_SLIDER);
	setup_circle_cursor(RGB_G_SLIDER_CURSOR);
	setup_gradient(RGB_G_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_G_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_G_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_G_SLIDER, RGB(0x00FF00));
	setup_rect_uvs(RGB_B_SLIDER);
	setup_circle_cursor(RGB_B_SLIDER_CURSOR);
	setup_gradient(RGB_B_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_B_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::RGB_B_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::RGB_B_SLIDER, RGB(0x0000FF));

	widget.wp_at(RGB_R_SLIDER).transform.position = { 0, 80 };
	widget.wp_at(RGB_R_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(RGB_R_SLIDER_CURSOR).transform.position = { widget.wp_at(RGB_R_SLIDER).right(), widget.wp_at(RGB_R_SLIDER).center_y() };
	widget.wp_at(RGB_G_SLIDER).transform.position = { 0, 0 };
	widget.wp_at(RGB_G_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(RGB_G_SLIDER_CURSOR).transform.position = { widget.wp_at(RGB_G_SLIDER).right(), widget.wp_at(RGB_G_SLIDER).center_y() };
	widget.wp_at(RGB_B_SLIDER).transform.position = { 0, -80 };
	widget.wp_at(RGB_B_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(RGB_B_SLIDER_CURSOR).transform.position = { widget.wp_at(RGB_B_SLIDER).right(), widget.wp_at(RGB_B_SLIDER).center_y() };

	// ---------- HSV SLIDER ----------

	widget.hobjs[HSV_H_SLIDER] = new WP_UnitRenderable(linear_hue_shader);
	widget.hobjs[HSV_H_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[HSV_S_SLIDER] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[HSV_S_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[HSV_V_SLIDER] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[HSV_V_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);

	orient_progress_slider(HSV_H_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSV_H_SLIDER_CURSOR);
	setup_rect_uvs(HSV_S_SLIDER);
	setup_circle_cursor(HSV_S_SLIDER_CURSOR);
	setup_gradient(HSV_S_SLIDER, (GLint)GradientIndex::HSV_S_SLIDER_ZERO, (GLint)GradientIndex::HSV_S_SLIDER_ONE, (GLint)GradientIndex::HSV_S_SLIDER_ZERO, (GLint)GradientIndex::HSV_S_SLIDER_ONE);
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ZERO, RGB::precise_from_hsv(0.0f, 0.0f, 1.0f));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ONE, RGB::precise_from_hsv(0.0f, 1.0f, 1.0f));
	setup_rect_uvs(HSV_V_SLIDER);
	setup_circle_cursor(HSV_V_SLIDER_CURSOR);
	setup_gradient(HSV_V_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::HSV_V_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::HSV_V_SLIDER);
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_V_SLIDER, RGB::precise_from_hsv(0.0f, 1.0f, 1.0f));

	widget.wp_at(HSV_H_SLIDER).transform.position = { 0, 80 };
	widget.wp_at(HSV_H_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(HSV_H_SLIDER_CURSOR).transform.position = { widget.wp_at(HSV_H_SLIDER).right(), widget.wp_at(HSV_H_SLIDER).center_y() };
	widget.wp_at(HSV_S_SLIDER).transform.position = { 0, 0 };
	widget.wp_at(HSV_S_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(HSV_S_SLIDER_CURSOR).transform.position = { widget.wp_at(HSV_S_SLIDER).right(), widget.wp_at(HSV_S_SLIDER).center_y() };
	widget.wp_at(HSV_V_SLIDER).transform.position = { 0, -80 };
	widget.wp_at(HSV_V_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(HSV_V_SLIDER_CURSOR).transform.position = { widget.wp_at(HSV_V_SLIDER).right(), widget.wp_at(HSV_V_SLIDER).center_y() };

	// ---------- HSL SLIDER ----------

	widget.hobjs[HSL_H_SLIDER] = new WP_UnitRenderable(linear_hue_shader);
	widget.hobjs[HSL_H_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[HSL_S_SLIDER] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[HSL_S_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[HSL_L_SLIDER] = new WP_UnitRenderable(linear_lightness_shader);
	widget.hobjs[HSL_L_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);

	orient_progress_slider(HSL_H_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSL_H_SLIDER_CURSOR);
	setup_rect_uvs(HSL_S_SLIDER);
	setup_circle_cursor(HSL_S_SLIDER_CURSOR);
	setup_gradient(HSL_S_SLIDER, (GLint)GradientIndex::HSL_S_SLIDER_ZERO, (GLint)GradientIndex::HSL_S_SLIDER_ONE, (GLint)GradientIndex::HSL_S_SLIDER_ZERO, (GLint)GradientIndex::HSL_S_SLIDER_ONE);
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ZERO, RGB::precise_from_hsv(0.0f, 0.0f, 0.5f));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ONE, RGB::precise_from_hsv(0.0f, 1.0f, 0.5f));
	orient_progress_slider(HSL_L_SLIDER, Cardinal::RIGHT);
	setup_circle_cursor(HSL_L_SLIDER_CURSOR);
	
	widget.wp_at(HSL_H_SLIDER).transform.position = { 0, 80 };
	widget.wp_at(HSL_H_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(HSL_H_SLIDER_CURSOR).transform.position = { widget.wp_at(HSL_H_SLIDER).right(), widget.wp_at(HSL_H_SLIDER).center_y() };
	widget.wp_at(HSL_S_SLIDER).transform.position = { 0, 0 };
	widget.wp_at(HSL_S_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(HSL_S_SLIDER_CURSOR).transform.position = { widget.wp_at(HSL_S_SLIDER).right(), widget.wp_at(HSL_S_SLIDER).center_y() };
	widget.wp_at(HSL_L_SLIDER).transform.position = { 0, -80 };
	widget.wp_at(HSL_L_SLIDER).transform.scale = { 200, 20 };
	widget.wp_at(HSL_L_SLIDER_CURSOR).transform.position = { widget.wp_at(HSL_L_SLIDER).right(), widget.wp_at(HSL_L_SLIDER).center_y() };

	// ---------- PARENT ----------

	sync_cp_widget_transforms();
}

void ColorPicker::connect_mouse_handlers()
{
	clbk_mb = [this](const Callback::MouseButton& mb) {
		if (mb.action == IAction::PRESS)
		{
			if (current_widget_control == -1)
			{
				Position local_cursor_pos = widget.parent.get_relative_pos(Machine.palette_cursor_world_pos());
				if (state == State::GRAPHIC_QUAD)
				{
					if (widget.wp_at(GRAPHIC_QUAD).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_QUAD_CURSOR;
						mouse_handler_graphic_quad(local_cursor_pos);
					}
					else if (widget.wp_at(GRAPHIC_HUE_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_HUE_SLIDER_CURSOR;
						mouse_handler_graphic_hue_slider(local_cursor_pos);
					}
				}
				else if (state == State::GRAPHIC_WHEEL)
				{
					if (widget.wp_at(GRAPHIC_HUE_WHEEL).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_HUE_WHEEL_CURSOR;
						mouse_handler_graphic_hue_wheel(local_cursor_pos);
					}
					else if (widget.wp_at(GRAPHIC_VALUE_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = GRAPHIC_VALUE_SLIDER_CURSOR;
						mouse_handler_graphic_value_slider(local_cursor_pos);
					}
				}
				else if (state == State::SLIDER_RGB)
				{
					if (widget.wp_at(RGB_R_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_R_SLIDER_CURSOR;
						mouse_handler_slider_rgb_r(local_cursor_pos);
					}
					else if (widget.wp_at(RGB_G_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_G_SLIDER_CURSOR;
						mouse_handler_slider_rgb_g(local_cursor_pos);
					}
					else if (widget.wp_at(RGB_B_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = RGB_B_SLIDER_CURSOR;
						mouse_handler_slider_rgb_b(local_cursor_pos);
					}
				}
				else if (state == State::SLIDER_HSV)
				{
					if (widget.wp_at(HSV_H_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_H_SLIDER_CURSOR;
						mouse_handler_slider_hsv_h(local_cursor_pos);
					}
					else if (widget.wp_at(HSV_S_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_S_SLIDER_CURSOR;
						mouse_handler_slider_hsv_s(local_cursor_pos);
					}
					else if (widget.wp_at(HSV_V_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSV_V_SLIDER_CURSOR;
						mouse_handler_slider_hsv_v(local_cursor_pos);
					}
				}
				else if (state == State::SLIDER_HSL)
				{
					if (widget.wp_at(HSL_H_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_H_SLIDER_CURSOR;
						mouse_handler_slider_hsl_h(local_cursor_pos);
					}
					else if (widget.wp_at(HSL_S_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_S_SLIDER_CURSOR;
						mouse_handler_slider_hsl_s(local_cursor_pos);
					}
					else if (widget.wp_at(HSL_L_SLIDER).contains_point(local_cursor_pos))
					{
						take_over_cursor();
						current_widget_control = HSL_L_SLIDER_CURSOR;
						mouse_handler_slider_hsl_l(local_cursor_pos);
					}
				}
			}
		}
		else if (mb.action == IAction::RELEASE)
			release_cursor();
		};
	clbk_mb_down = [this](const Callback::MouseButton& mb) {
		Position local_cursor_pos = widget.parent.get_relative_pos(Machine.palette_cursor_world_pos());
		if (current_widget_control == GRAPHIC_QUAD_CURSOR)
			mouse_handler_graphic_quad(local_cursor_pos);
		else if (current_widget_control == GRAPHIC_HUE_SLIDER_CURSOR)
			mouse_handler_graphic_hue_slider(local_cursor_pos);
		else if (current_widget_control == GRAPHIC_HUE_WHEEL_CURSOR)
			mouse_handler_graphic_hue_wheel(local_cursor_pos);
		else if (current_widget_control == GRAPHIC_VALUE_SLIDER_CURSOR)
			mouse_handler_graphic_value_slider(local_cursor_pos);
		else if (current_widget_control == RGB_R_SLIDER_CURSOR)
			mouse_handler_slider_rgb_r(local_cursor_pos);
		else if (current_widget_control == RGB_G_SLIDER_CURSOR)
			mouse_handler_slider_rgb_g(local_cursor_pos);
		else if (current_widget_control == RGB_B_SLIDER_CURSOR)
			mouse_handler_slider_rgb_b(local_cursor_pos);
		else if (current_widget_control == HSV_H_SLIDER_CURSOR)
			mouse_handler_slider_hsv_h(local_cursor_pos);
		else if (current_widget_control == HSV_S_SLIDER_CURSOR)
			mouse_handler_slider_hsv_s(local_cursor_pos);
		else if (current_widget_control == HSV_V_SLIDER_CURSOR)
			mouse_handler_slider_hsv_v(local_cursor_pos);
		else if (current_widget_control == HSL_H_SLIDER_CURSOR)
			mouse_handler_slider_hsl_h(local_cursor_pos);
		else if (current_widget_control == HSL_S_SLIDER_CURSOR)
			mouse_handler_slider_hsl_s(local_cursor_pos);
		else if (current_widget_control == HSL_L_SLIDER_CURSOR)
			mouse_handler_slider_hsl_l(local_cursor_pos);
		};
	Machine.main_window->clbk_mouse_button.push_back(clbk_mb, this);
	Machine.main_window->clbk_mouse_button_down.push_back(clbk_mb_down, this);
}

void ColorPicker::take_over_cursor() const
{
	//Machine.main_window->set_mouse_mode(MouseMode::VIRTUAL);
	Machine.main_window->override_gui_cursor_change(true);
	Machine.main_window->set_cursor(create_cursor(StandardCursor::CROSSHAIR));
}

void ColorPicker::release_cursor()
{
	if (current_widget_control != -1)
	{
		//Machine.main_window->set_mouse_mode(MouseMode::VISIBLE);
		Machine.main_window->override_gui_cursor_change(false);
		Machine.main_window->set_cursor(create_cursor(StandardCursor::ARROW));
		current_widget_control = -1;
	}
}

ColorFrame ColorPicker::get_color() const
{
	// LATER add alpha
	if (state == State::GRAPHIC_QUAD)
	{
		glm::vec2 sv = get_graphic_quad_sat_and_value();
		return RGB::precise_from_hsv(slider_normal_x(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR), sv[0], sv[1]);
	}
	else if (state == State::GRAPHIC_WHEEL)
	{
		glm::vec2 hs = get_graphic_wheel_hue_and_sat();
		return RGB::precise_from_hsv(hs[0], hs[1], slider_normal_x(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR));
	}
	else if (state == State::SLIDER_RGB)
	{
		float r = slider_normal_x(RGB_R_SLIDER, RGB_R_SLIDER_CURSOR);
		float g = slider_normal_x(RGB_G_SLIDER, RGB_G_SLIDER_CURSOR);
		float b = slider_normal_x(RGB_B_SLIDER, RGB_B_SLIDER_CURSOR);
		return RGB(r, g, b);
	}
	else if (state == State::SLIDER_HSV)
	{
		float h = slider_normal_x(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR);
		float s = slider_normal_x(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR);
		float v = slider_normal_x(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR);
		return RGB::precise_from_hsv(h, s, v);
	}
	else if (state == State::SLIDER_HSL)
	{
		float h = slider_normal_x(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR);
		float s = slider_normal_x(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR);
		float l = slider_normal_x(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR);
		return RGB::precise_from_hsl(h, s, l);
	}
	return ColorFrame();
}

void ColorPicker::set_color(ColorFrame color)
{
	if (state == State::GRAPHIC_QUAD)
	{
		glm::vec4 hsva = color.hsva_as_vec();
		float hue = hsva.x;
		float sat = hsva.y;
		float val = hsva.z;
		widget.wp_at(GRAPHIC_QUAD_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_QUAD).interp_x(sat);
		widget.wp_at(GRAPHIC_QUAD_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_QUAD).interp_y(val);
		widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_HUE_SLIDER).interp_x(hue);
		widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_HUE_SLIDER).center_y();
		enact_graphic_quad_cursor_position(hue, sat, val);
		enact_graphic_hue_slider_cursor_position(hue);
	}
	else if (state == State::GRAPHIC_WHEEL)
	{
		glm::vec4 hsva = color.hsva_as_vec();
		float hue = hsva.x;
		float sat = hsva.y;
		float val = hsva.z;
		float x = sat * glm::cos(glm::tau<float>() * hue);
		float y = -sat * glm::sin(glm::tau<float>() * hue);
		widget.wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_HUE_WHEEL).interp_x(0.5f * (x + 1));
		widget.wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_HUE_WHEEL).interp_y(0.5f * (y + 1));
		widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_VALUE_SLIDER).interp_x(val);
		widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_VALUE_SLIDER).center_y();
		enact_graphic_hue_wheel_cursor_position(hue, sat);
		enact_graphic_value_slider_cursor_position(hue, val);
	}
	else if (state == State::SLIDER_RGB)
	{
		glm::vec4 rgba = color.rgba_as_vec();
		widget.wp_at(RGB_R_SLIDER_CURSOR).transform.position.x = widget.wp_at(RGB_R_SLIDER).interp_x(rgba.r);
		widget.wp_at(RGB_R_SLIDER_CURSOR).transform.position.y = widget.wp_at(RGB_R_SLIDER).center_y();
		widget.wp_at(RGB_G_SLIDER_CURSOR).transform.position.x = widget.wp_at(RGB_G_SLIDER).interp_x(rgba.g);
		widget.wp_at(RGB_G_SLIDER_CURSOR).transform.position.y = widget.wp_at(RGB_G_SLIDER).center_y();
		widget.wp_at(RGB_B_SLIDER_CURSOR).transform.position.x = widget.wp_at(RGB_B_SLIDER).interp_x(rgba.b);
		widget.wp_at(RGB_B_SLIDER_CURSOR).transform.position.y = widget.wp_at(RGB_B_SLIDER).center_y();
		enact_slider_rgb_cursor_positions();
	}
	else if (state == State::SLIDER_HSV)
	{
		glm::vec4 hsva = color.hsva_as_vec();
		widget.wp_at(HSV_H_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSV_H_SLIDER).interp_x(hsva.x);
		widget.wp_at(HSV_H_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSV_H_SLIDER).center_y();
		widget.wp_at(HSV_S_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSV_S_SLIDER).interp_x(hsva.y);
		widget.wp_at(HSV_S_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSV_S_SLIDER).center_y();
		widget.wp_at(HSV_V_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSV_V_SLIDER).interp_x(hsva.z);
		widget.wp_at(HSV_V_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSV_V_SLIDER).center_y();
		enact_slider_hsv_cursor_positions();
	}
	else if (state == State::SLIDER_HSL)
	{
		glm::vec4 hsla = color.hsla_as_vec();
		widget.wp_at(HSL_H_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSL_H_SLIDER).interp_x(hsla.x);
		widget.wp_at(HSL_H_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSL_H_SLIDER).center_y();
		widget.wp_at(HSL_S_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSL_S_SLIDER).interp_x(hsla.y);
		widget.wp_at(HSL_S_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSL_S_SLIDER).center_y();
		widget.wp_at(HSL_L_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSL_L_SLIDER).interp_x(hsla.z);
		widget.wp_at(HSL_L_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSL_L_SLIDER).center_y();
		enact_slider_hsl_cursor_positions();
	}
}

void ColorPicker::set_position(Position world_pos, Position screen_pos) // LATER pass one Position. Add coordinate functions to Machine.
{
	widget.parent.position = world_pos;
	center = screen_pos;
}

void ColorPicker::mouse_handler_graphic_quad(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_QUAD_CURSOR).transform.position = widget.wp_at(GRAPHIC_QUAD).clamp_point(local_cursor_pos);
	enact_graphic_quad_and_hue_slider_cursor_positions(local_cursor_pos);
}

void ColorPicker::mouse_handler_graphic_hue_slider(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_HUE_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_HUE_SLIDER).center_y();
	enact_graphic_quad_and_hue_slider_cursor_positions(local_cursor_pos);
}

void ColorPicker::mouse_handler_graphic_hue_wheel(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position = widget.wp_at(GRAPHIC_HUE_WHEEL).clamp_point_in_ellipse(local_cursor_pos);
	enact_graphic_hue_wheel_and_value_slider_cursor_positions(local_cursor_pos);
}

void ColorPicker::mouse_handler_graphic_value_slider(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_VALUE_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_VALUE_SLIDER).center_y();
	enact_graphic_hue_wheel_and_value_slider_cursor_positions(local_cursor_pos);
}

void ColorPicker::mouse_handler_slider_rgb_r(Position local_cursor_pos)
{
	widget.wp_at(RGB_R_SLIDER_CURSOR).transform.position.x = widget.wp_at(RGB_R_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(RGB_R_SLIDER_CURSOR).transform.position.y = widget.wp_at(RGB_R_SLIDER).center_y();
	enact_slider_rgb_cursor_positions();
}

void ColorPicker::mouse_handler_slider_rgb_g(Position local_cursor_pos)
{
	widget.wp_at(RGB_G_SLIDER_CURSOR).transform.position.x = widget.wp_at(RGB_G_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(RGB_G_SLIDER_CURSOR).transform.position.y = widget.wp_at(RGB_G_SLIDER).center_y();
	enact_slider_rgb_cursor_positions();
}

void ColorPicker::mouse_handler_slider_rgb_b(Position local_cursor_pos)
{
	widget.wp_at(RGB_B_SLIDER_CURSOR).transform.position.x = widget.wp_at(RGB_B_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(RGB_B_SLIDER_CURSOR).transform.position.y = widget.wp_at(RGB_B_SLIDER).center_y();
	enact_slider_rgb_cursor_positions();
}

void ColorPicker::mouse_handler_slider_hsv_h(Position local_cursor_pos)
{
	widget.wp_at(HSV_H_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSV_H_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(HSV_H_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSV_H_SLIDER).center_y();
	enact_slider_hsv_cursor_positions();
}

void ColorPicker::mouse_handler_slider_hsv_s(Position local_cursor_pos)
{
	widget.wp_at(HSV_S_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSV_S_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(HSV_S_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSV_S_SLIDER).center_y();
	enact_slider_hsv_cursor_positions();
}

void ColorPicker::mouse_handler_slider_hsv_v(Position local_cursor_pos)
{
	widget.wp_at(HSV_V_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSV_V_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(HSV_V_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSV_V_SLIDER).center_y();
	enact_slider_hsv_cursor_positions();
}

void ColorPicker::mouse_handler_slider_hsl_h(Position local_cursor_pos)
{
	widget.wp_at(HSL_H_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSL_H_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(HSL_H_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSL_H_SLIDER).center_y();
	enact_slider_hsl_cursor_positions();
}

void ColorPicker::mouse_handler_slider_hsl_s(Position local_cursor_pos)
{
	widget.wp_at(HSL_S_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSL_S_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(HSL_S_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSL_S_SLIDER).center_y();
	enact_slider_hsl_cursor_positions();
}

void ColorPicker::mouse_handler_slider_hsl_l(Position local_cursor_pos)
{
	widget.wp_at(HSL_L_SLIDER_CURSOR).transform.position.x = widget.wp_at(HSL_L_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(HSL_L_SLIDER_CURSOR).transform.position.y = widget.wp_at(HSL_L_SLIDER).center_y();
	enact_slider_hsl_cursor_positions();
}

void ColorPicker::enact_graphic_quad_cursor_position(float hue, float sat, float val)
{
	set_circle_cursor_value(GRAPHIC_QUAD_CURSOR, contrast_wb_value_complex_hsv({ hue, sat, val }));
	sync_single_cp_widget_transform(GRAPHIC_QUAD_CURSOR);
}

void ColorPicker::enact_graphic_hue_slider_cursor_position(float hue)
{
	set_circle_cursor_value(GRAPHIC_HUE_SLIDER_CURSOR, contrast_wb_value_simple_hue(hue));
	send_graphic_quad_hue_to_uniform(hue);
	sync_single_cp_widget_transform(GRAPHIC_HUE_SLIDER_CURSOR);
}

void ColorPicker::enact_graphic_quad_and_hue_slider_cursor_positions(Position local_cursor_pos)
{
	glm::vec2 sv = get_graphic_quad_sat_and_value();
	float hue = slider_normal_x(GRAPHIC_HUE_SLIDER, GRAPHIC_HUE_SLIDER_CURSOR);
	enact_graphic_quad_cursor_position(hue, sv[0], sv[1]);
	enact_graphic_hue_slider_cursor_position(hue);
}

void ColorPicker::enact_graphic_hue_wheel_cursor_position(float hue, float sat)
{
	set_circle_cursor_value(GRAPHIC_HUE_WHEEL_CURSOR, contrast_wb_value_simple_hue_and_sat(hue, sat));
	sync_single_cp_widget_transform(GRAPHIC_HUE_WHEEL_CURSOR);
	send_graphic_value_slider_hue_and_sat_to_uniform(hue, sat);
}

void ColorPicker::enact_graphic_value_slider_cursor_position(float hue, float value)
{
	set_circle_cursor_value(GRAPHIC_VALUE_SLIDER_CURSOR, contrast_wb_value_simple_hue_and_value(hue, value));
	send_graphic_wheel_value_to_uniform(value);
	sync_single_cp_widget_transform(GRAPHIC_VALUE_SLIDER_CURSOR);
}

void ColorPicker::enact_graphic_hue_wheel_and_value_slider_cursor_positions(Position local_cursor_pos)
{
	glm::vec2 hs = get_graphic_wheel_hue_and_sat();
	float value = slider_normal_x(GRAPHIC_VALUE_SLIDER, GRAPHIC_VALUE_SLIDER_CURSOR);
	enact_graphic_hue_wheel_cursor_position(hs[0], hs[1]);
	enact_graphic_value_slider_cursor_position(hs[0], value);
}

void ColorPicker::enact_slider_rgb_cursor_positions()
{
	sync_single_cp_widget_transform(RGB_R_SLIDER_CURSOR);
	sync_single_cp_widget_transform(RGB_G_SLIDER_CURSOR);
	sync_single_cp_widget_transform(RGB_B_SLIDER_CURSOR);
}

void ColorPicker::enact_slider_hsv_cursor_positions()
{
	glm::vec3 hsv{
		slider_normal_x(HSV_H_SLIDER, HSV_H_SLIDER_CURSOR),
		slider_normal_x(HSV_S_SLIDER, HSV_S_SLIDER_CURSOR),
		slider_normal_x(HSV_V_SLIDER, HSV_V_SLIDER_CURSOR)
	};
	set_circle_cursor_value(HSV_H_SLIDER_CURSOR, contrast_wb_value_simple_hue(hsv.x));
	set_circle_cursor_value(HSV_S_SLIDER_CURSOR, contrast_wb_value_complex_hsv(hsv));
	set_circle_cursor_value(HSV_V_SLIDER_CURSOR, contrast_wb_value_simple_hue_and_value(hsv.x, hsv.z));
	send_slider_hsv_hue_and_value_to_uniform(hsv.x, hsv.z);
	sync_single_cp_widget_transform(HSV_H_SLIDER_CURSOR);
	sync_single_cp_widget_transform(HSV_S_SLIDER_CURSOR);
	sync_single_cp_widget_transform(HSV_V_SLIDER_CURSOR);
}

void ColorPicker::enact_slider_hsl_cursor_positions()
{
	glm::vec3 hsl{
		slider_normal_x(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR),
		slider_normal_x(HSL_S_SLIDER, HSL_S_SLIDER_CURSOR),
		slider_normal_x(HSL_L_SLIDER, HSL_L_SLIDER_CURSOR)
	};
	set_circle_cursor_value(HSL_H_SLIDER_CURSOR, contrast_wb_value_simple_hue(slider_normal_x(HSL_H_SLIDER, HSL_H_SLIDER_CURSOR)));
	set_circle_cursor_value(HSL_S_SLIDER_CURSOR, contrast_wb_value_complex_hsl(hsl));
	set_circle_cursor_value(HSL_L_SLIDER_CURSOR, contrast_wb_value_simple_hue_and_lightness(hsl.x, hsl.z));
	send_slider_hsl_hue_and_lightness_to_uniform(hsl.x, hsl.z);
	sync_single_cp_widget_transform(HSL_H_SLIDER_CURSOR);
	sync_single_cp_widget_transform(HSL_S_SLIDER_CURSOR);
	sync_single_cp_widget_transform(HSL_L_SLIDER_CURSOR);
}

void ColorPicker::orient_progress_slider(size_t control, Cardinal i) const
{
	const UnitRenderable& renderable = ur_wget(widget, control);
	float zero = 0.0f;
	float one = 1.0f;
	renderable.set_attribute_single_vertex(0, 1, i == Cardinal::DOWN || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(1, 1, i == Cardinal::RIGHT || i == Cardinal::DOWN ? &one : &zero);
	renderable.set_attribute_single_vertex(2, 1, i == Cardinal::UP || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(3, 1, i == Cardinal::RIGHT || i == Cardinal::UP ? &one : &zero);
}

void ColorPicker::send_graphic_quad_hue_to_uniform(float hue)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::GRAPHIC_QUAD, RGB::precise_from_hsv(hue, 1.0f, 1.0f));
}

glm::vec2 ColorPicker::get_graphic_quad_sat_and_value() const
{
	return widget.wp_at(GRAPHIC_QUAD).normalize(widget.wp_at(GRAPHIC_QUAD_CURSOR).transform.position);
}

void ColorPicker::send_graphic_wheel_value_to_uniform(float value)
{
	bind_shader(hue_wheel_w_shader);
	QUASAR_GL(glUniform1fv(hue_wheel_w_shader.uniform_locations["u_Value"], 1, &value));
}

void ColorPicker::send_graphic_value_slider_hue_and_sat_to_uniform(float hue, float sat)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::GRAPHIC_VALUE_SLIDER, RGB::precise_from_hsv(hue, sat, 1.0f));
}

glm::vec2 ColorPicker::get_graphic_wheel_hue_and_sat() const
{
	glm::vec2 normal = 2.0f * widget.wp_at(GRAPHIC_HUE_WHEEL).normalize(widget.wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position) - glm::vec2(1.0f);
	float hue = -glm::atan(normal.y, normal.x) / glm::tau<float>();
	if (hue < 0.0f)
		hue += 1.0f;
	return { hue, glm::length(normal) };
}

void ColorPicker::send_slider_hsv_hue_and_value_to_uniform(float hue, float value)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ZERO, RGB::precise_from_hsv(hue, 0.0f, value));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_S_SLIDER_ONE, RGB::precise_from_hsv(hue, 1.0f, value));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSV_V_SLIDER, HSV(hue, 1.0f, 1.0f));
}

void ColorPicker::send_slider_hsl_hue_and_lightness_to_uniform(float hue, float lightness)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ZERO, RGB::precise_from_hsl(hue, 0.0f, lightness));
	send_gradient_color_uniform(quad_shader, GradientIndex::HSL_S_SLIDER_ONE, RGB::precise_from_hsl(hue, 1.0f, lightness));
	ColorFrame c = RGB::precise_from_hsl(hue, 1.0f, lightness);
	bind_shader(linear_lightness_shader);
	QUASAR_GL(glUniform1fv(linear_lightness_shader.uniform_locations["u_Hue"], 1, &hue)); // TODO move these types of functions to Shader
}

float ColorPicker::slider_normal_x(size_t control, size_t cursor) const
{
	return widget.wp_at(control).normalize_x(widget.wp_at(cursor).transform.position.x);
}

void ColorPicker::setup_vertex_positions(size_t control) const
{
	const UnitRenderable& renderable = ur_wget(widget, control);
	WidgetPlacement wp = widget.wp_at(control).relative_to(widget.parent);
	renderable.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.bottom() }));
	renderable.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.bottom() }));
	renderable.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ wp.left(), wp.top() }));
	renderable.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ wp.right(), wp.top() }));
}

void ColorPicker::setup_rect_uvs(size_t control) const
{
	const UnitRenderable& renderable = ur_wget(widget, control);
	renderable.set_attribute_single_vertex(0, 1, glm::value_ptr(glm::vec2{ 0.0f, 0.0f }));
	renderable.set_attribute_single_vertex(1, 1, glm::value_ptr(glm::vec2{ 1.0f, 0.0f }));
	renderable.set_attribute_single_vertex(2, 1, glm::value_ptr(glm::vec2{ 0.0f, 1.0f }));
	renderable.set_attribute_single_vertex(3, 1, glm::value_ptr(glm::vec2{ 1.0f, 1.0f }));
}

void ColorPicker::setup_gradient(size_t control, GLint g1, GLint g2, GLint g3, GLint g4) const
{
	ur_wget(widget, control).set_attribute(2, glm::value_ptr(glm::vec4{ g1, g2, g3, g4 }));
}

void ColorPicker::sync_cp_widget_transforms() const
{
	for (size_t i = 0; i < widget.hobjs.size(); ++i)
		sync_single_cp_widget_transform(i);
}

void ColorPicker::sync_single_cp_widget_transform(size_t control) const
{
	if (widget.hobjs[control])
	{
		setup_vertex_positions(control);
		ur_wget(widget, control).send_buffer();
	}
}

void ColorPicker::set_circle_cursor_thickness(size_t cursor, float thickness)
{
	ur_wget(widget, cursor).set_attribute(2, &thickness);
}

void ColorPicker::set_circle_cursor_value(size_t cursor, float value)
{
	ur_wget(widget, cursor).set_attribute(3, &value);
}

float ColorPicker::get_circle_cursor_value(size_t cursor)
{
	float value;
	ur_wget(widget, cursor).get_attribute(0, 3, &value);
	return value;
}

void ColorPicker::setup_circle_cursor(size_t cursor)
{
	setup_rect_uvs(cursor);
	set_circle_cursor_thickness(cursor, 0.4f);
	set_circle_cursor_value(cursor, 1.0f);
	widget.wp_at(cursor).transform.scale = { 8, 8 };
}
