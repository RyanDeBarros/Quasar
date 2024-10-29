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
	HSV_H_SLIDER,
	HSV_S_SLIDER,
	HSV_V_SLIDER,
	HSL_H_SLIDER,
	HSL_S_SLIDER,
	HSL_L_SLIDER,
	_MAX_GRADIENT_COLORS
};

static void send_gradient_color_uniform(Shader& shader, GradientIndex index, ColorFrame color)
{
	bind_shader(shader);
	QUASAR_GL(glUniform4fv(shader.uniform_locations["u_GradientColors[0]"] + int(index), 1, glm::value_ptr(color.rgba_as_vec())));
}

// TODO use UMR when possible
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
	HSL_L_SLIDER,					// quad
	HSL_L_SLIDER_CURSOR,			// circle_cursor
	_CPWC_COUNT
};

ColorPicker::ColorPicker()
	: quad_shader(FileSystem::resources_path("gradients/quad.vert"), FileSystem::resources_path("gradients/quad.frag.tmpl"),
		{ {"$MAX_GRADIENT_COLORS", std::to_string((int)GradientIndex::_MAX_GRADIENT_COLORS) } }),
	linear_hue_shader(FileSystem::resources_path("gradients/linear_hue.vert"), FileSystem::resources_path("gradients/linear_hue.frag")),
	hue_wheel_w_shader(FileSystem::resources_path("gradients/hue_wheel_w.vert"), FileSystem::resources_path("gradients/hue_wheel_w.frag")),
	circle_cursor_shader(FileSystem::resources_path("circle_cursor.vert"), FileSystem::resources_path("circle_cursor.frag")),
	widget(_CPWC_COUNT)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::BLACK, ColorFrame(HSV(0.0f, 0.0f, 0.0f)));
	send_gradient_color_uniform(quad_shader, GradientIndex::WHITE, ColorFrame(HSV(0.0f, 0.0f, 1.0f)));
	initialize_widget();
	connect_mouse_handlers();
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
		break;
	case State::SLIDER_HSV:
		break;
	case State::SLIDER_HSL:
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

	ImGuiWindowFlags invisible_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration;
	if (ImGui::Begin("-", nullptr, invisible_flags))
	{
		if (ImGui::BeginTabBar("bar"))
		{
			if (ImGui::TabItemButton("QUAD"))
				set_state(State::GRAPHIC_QUAD);
			if (ImGui::TabItemButton("WHEEL"))
				set_state(State::GRAPHIC_WHEEL);
			ImGui::EndTabBar();
		}
		ImGui::End();
	}
}

void ColorPicker::set_state(State _state)
{
	if (_state != state)
	{
		current_widget_control = -1; // release mouse
		state = _state;
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
	setup_gradient(GRAPHIC_QUAD, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::WHITE, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER);
	send_graphic_quad_hue_to_uniform(0.0f);
	setup_circle_cursor(GRAPHIC_QUAD_CURSOR);
	orient_graphic_hue_slider(Cardinal::RIGHT);
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
	setup_gradient(GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER, (GLint)GradientIndex::BLACK, (GLint)GradientIndex::GRAPHIC_VALUE_SLIDER); // TODO also put in orient method
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
						current_widget_control = GRAPHIC_QUAD_CURSOR;
						mouse_handler_graphic_quad(local_cursor_pos);
					}
					else if (widget.wp_at(GRAPHIC_HUE_SLIDER).contains_point(local_cursor_pos))
					{
						current_widget_control = GRAPHIC_HUE_SLIDER_CURSOR;
						mouse_handler_graphic_hue_slider(local_cursor_pos);
					}
				}
				else if (state == State::GRAPHIC_WHEEL)
				{
					if (widget.wp_at(GRAPHIC_HUE_WHEEL).contains_point(local_cursor_pos))
					{
						current_widget_control = GRAPHIC_HUE_WHEEL_CURSOR;
						mouse_handler_graphic_hue_wheel(local_cursor_pos);
					}
					else if (widget.wp_at(GRAPHIC_VALUE_SLIDER).contains_point(local_cursor_pos))
					{
						current_widget_control = GRAPHIC_VALUE_SLIDER_CURSOR;
						mouse_handler_graphic_value_slider(local_cursor_pos);
					}
				}
			}
		}
		else if (mb.action == IAction::RELEASE)
		{
			current_widget_control = -1;
		}
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
		};
	Machine.main_window->clbk_mouse_button.push_back(clbk_mb, this);
	Machine.main_window->clbk_mouse_button_down.push_back(clbk_mb_down, this);
}

ColorFrame ColorPicker::get_color() const
{
	if (state == State::GRAPHIC_QUAD)
	{
		// TODO add alpha
		glm::vec2 sv = get_graphic_quad_sat_and_value();
		return ColorFrame(HSV(get_graphic_hue_slider_hue(), sv[0], sv[1]));
	}
	else if (state == State::GRAPHIC_WHEEL)
	{
		// TODO add alpha
		glm::vec2 hs = get_graphic_wheel_hue_and_sat();
		return ColorFrame(HSV(hs[0], hs[1], get_graphic_value_slider_value()));
	}
	return ColorFrame();
}

void ColorPicker::set_position(Position world_pos, Position screen_pos) // TODO pass one Position. Add coordinate functions to Machine.
{
	widget.parent.position = world_pos;
	center = screen_pos;
}

void ColorPicker::mouse_handler_graphic_quad(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_QUAD_CURSOR).transform.position = widget.wp_at(GRAPHIC_QUAD).clamp_point(local_cursor_pos);
	// TODO set circle cursor value if contrast is weak
	sync_single_cp_widget_transform(GRAPHIC_QUAD_CURSOR);
}

void ColorPicker::mouse_handler_graphic_hue_slider(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_HUE_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_HUE_SLIDER).center_point().y;
	float hue = get_graphic_hue_slider_hue();
	if (on_interval(hue, 0.1f, 0.5f))
		set_circle_cursor_value(GRAPHIC_HUE_SLIDER_CURSOR, 0.0f);
	else
		set_circle_cursor_value(GRAPHIC_HUE_SLIDER_CURSOR, 1.0f);
	send_graphic_quad_hue_to_uniform(hue);
	sync_single_cp_widget_transform(GRAPHIC_HUE_SLIDER_CURSOR);
}

void ColorPicker::mouse_handler_graphic_hue_wheel(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position = widget.wp_at(GRAPHIC_HUE_WHEEL).clamp_point_in_ellipse(local_cursor_pos);
	// TODO set circle cursor value if contrast is weak
	sync_single_cp_widget_transform(GRAPHIC_HUE_WHEEL_CURSOR);
	glm::vec2 hs = get_graphic_wheel_hue_and_sat();
	send_graphic_value_slider_hue_and_sat_to_uniform(hs[0], hs[1]);
}

void ColorPicker::mouse_handler_graphic_value_slider(Position local_cursor_pos)
{
	widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position.x = widget.wp_at(GRAPHIC_VALUE_SLIDER).clamp_x(local_cursor_pos.x);
	widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position.y = widget.wp_at(GRAPHIC_VALUE_SLIDER).center_point().y;
	float value = get_graphic_value_slider_value();
	if (value > 0.5f)
		set_circle_cursor_value(GRAPHIC_VALUE_SLIDER_CURSOR, 0.0f);
	else
		set_circle_cursor_value(GRAPHIC_VALUE_SLIDER_CURSOR, 1.0f);
	send_graphic_wheel_value_to_uniform(value);
	sync_single_cp_widget_transform(GRAPHIC_VALUE_SLIDER_CURSOR);
}

glm::vec2 ColorPicker::get_graphic_quad_sat_and_value() const
{
	return widget.wp_at(GRAPHIC_QUAD).normalize(widget.wp_at(GRAPHIC_QUAD_CURSOR).transform.position);
}

float ColorPicker::get_graphic_hue_slider_hue() const
{
	return widget.wp_at(GRAPHIC_HUE_SLIDER).normalize_x(widget.wp_at(GRAPHIC_HUE_SLIDER_CURSOR).transform.position.x);
}

void ColorPicker::send_graphic_wheel_value_to_uniform(float value)
{
	bind_shader(hue_wheel_w_shader);
	QUASAR_GL(glUniform1fv(hue_wheel_w_shader.uniform_locations["u_Value"], 1, &value));
}

void ColorPicker::send_graphic_value_slider_hue_and_sat_to_uniform(float hue, float sat)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::GRAPHIC_VALUE_SLIDER, ColorFrame(HSV(hue, sat, 1.0f)));
}

glm::vec2 ColorPicker::get_graphic_wheel_hue_and_sat() const
{
	glm::vec2 normal = 2.0f * widget.wp_at(GRAPHIC_HUE_WHEEL).normalize(widget.wp_at(GRAPHIC_HUE_WHEEL_CURSOR).transform.position) - glm::vec2(1.0f);
	return { 0.5f + glm::atan(normal.y, normal.x) / glm::tau<float>(), glm::length(normal) };
}

float ColorPicker::get_graphic_value_slider_value() const
{
	return widget.wp_at(GRAPHIC_VALUE_SLIDER).normalize_x(widget.wp_at(GRAPHIC_VALUE_SLIDER_CURSOR).transform.position.x);
}

void ColorPicker::send_graphic_quad_hue_to_uniform(float hue)
{
	send_gradient_color_uniform(quad_shader, GradientIndex::GRAPHIC_QUAD, ColorFrame(HSV(hue, 1.0f, 1.0f)));
}

void ColorPicker::orient_graphic_hue_slider(Cardinal i) const
{
	const UnitRenderable& renderable = ur_wget(widget, GRAPHIC_HUE_SLIDER);
	float zero = 0.0f;
	float one = 1.0f;
	renderable.set_attribute_single_vertex(0, 1, i == Cardinal::DOWN || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(1, 1, i == Cardinal::RIGHT || i == Cardinal::DOWN ? &one : &zero);
	renderable.set_attribute_single_vertex(2, 1, i == Cardinal::UP || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(3, 1, i == Cardinal::RIGHT || i == Cardinal::UP ? &one : &zero);
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
