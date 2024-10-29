#include "ColorPicker.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "edit/Color.h"
#include "user/Machine.h"

static void send_gradient_color_uniform(Shader& shader, GLint index, glm::vec4 color)
{
	bind_shader(shader);
	QUASAR_GL(glUniform4fv(shader.uniform_locations["u_GradientColors[0]"] + index, 1, glm::value_ptr(color)));
}

static constexpr GLint ALPHA_SLIDER_GRADIENT_INDEX = 2;
static constexpr GLint RGB_QUAD_GRADIENT_INDEX = 3;
static constexpr GLint HSV_VALUE_SLIDER_GRADIENT_INDEX = 4;
static constexpr GLint RGB_R_SLIDER_GRADIENT_INDEX = 5;
static constexpr GLint RGB_G_SLIDER_GRADIENT_INDEX = 6;
static constexpr GLint RGB_B_SLIDER_GRADIENT_INDEX = 7;
static constexpr GLint HSV_H_SLIDER_GRADIENT_INDEX = 8;
static constexpr GLint HSV_S_SLIDER_GRADIENT_INDEX = 9;
static constexpr GLint HSV_V_SLIDER_GRADIENT_INDEX = 10;
static constexpr GLint HSL_H_SLIDER_GRADIENT_INDEX = 11;
static constexpr GLint HSL_S_SLIDER_GRADIENT_INDEX = 12;
static constexpr GLint HSL_L_SLIDER_GRADIENT_INDEX = 13;
static constexpr const char* MAX_GRADIENT_COLORS = "14";

// TODO use UMR when possible
enum
{
	// LATER ? use one CURSORS UMR, and implement visibility for subshapes in UMR.
	ALPHA_SLIDER,				// quad
	RGB_QUAD,					// quad
	RGB_QUAD_CURSOR,			// circle_cursor
	RGB_HUE_SLIDER,				// linear_hue
	RGB_HUE_SLIDER_CURSOR,		// circle_cursor
	HSV_WHEEL,					// hue_wheel
	HSV_WHEEL_CURSOR,			// circle_cursor
	HSV_VALUE_SLIDER,			// quad
	HSV_VALUE_SLIDER_CURSOR,	// circle_cursor
	RGB_R_SLIDER,				// quad
	RGB_R_SLIDER_CURSOR,		// circle_cursor
	RGB_G_SLIDER,				// quad
	RGB_G_SLIDER_CURSOR,		// circle_cursor
	RGB_B_SLIDER,				// quad
	RGB_B_SLIDER_CURSOR,		// circle_cursor
	HSV_H_SLIDER,				// quad
	HSV_H_SLIDER_CURSOR,		// circle_cursor
	HSV_S_SLIDER,				// quad
	HSV_S_SLIDER_CURSOR,		// circle_cursor
	HSV_V_SLIDER,				// quad
	HSV_V_SLIDER_CURSOR,		// circle_cursor
	HSL_H_SLIDER,				// quad
	HSL_H_SLIDER_CURSOR,		// circle_cursor
	HSL_S_SLIDER,				// quad
	HSL_S_SLIDER_CURSOR,		// circle_cursor
	HSL_L_SLIDER,				// quad
	HSL_L_SLIDER_CURSOR,		// circle_cursor
	_CPWC_COUNT
};

ColorPicker::ColorPicker()
	: quad_shader(FileSystem::resources_path("gradients/quad.vert"), FileSystem::resources_path("gradients/quad.frag.tmpl"), { {"$MAX_GRADIENT_COLORS", MAX_GRADIENT_COLORS } }),
	linear_hue_shader(FileSystem::resources_path("gradients/linear_hue.vert"), FileSystem::resources_path("gradients/linear_hue.frag")),
	hue_wheel_w_shader(FileSystem::resources_path("gradients/hue_wheel_w.vert"), FileSystem::resources_path("gradients/hue_wheel_w.frag")),
	circle_cursor_shader(FileSystem::resources_path("circle_cursor.vert"), FileSystem::resources_path("circle_cursor.frag")),
	widget(_CPWC_COUNT)
{
	send_gradient_color_uniform(quad_shader, 0, ColorFrame(HSV(0.0f, 0.0f, 0.0f)).rgba_as_vec());
	send_gradient_color_uniform(quad_shader, 1, ColorFrame(HSV(0.0f, 0.0f, 1.0f)).rgba_as_vec());
	initialize_widget();
	connect_mouse_handlers();
}

ColorPicker::~ColorPicker()
{
	Machine.main_window->clbk_mouse_button.remove_at_owner(this);
	Machine.main_window->clbk_mouse_button_down.remove_at_owner(this);
}

void ColorPicker::render() const
{
	switch (state)
	{
	case State::GRAPHIC_RGB:
		ur_wget(widget, RGB_QUAD).draw();
		ur_wget(widget, RGB_QUAD_CURSOR).draw();
		ur_wget(widget, RGB_HUE_SLIDER).draw();
		ur_wget(widget, RGB_HUE_SLIDER_CURSOR).draw();
		break;
	case State::GRAPHIC_HSV:
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
}

void ColorPicker::initialize_widget()
{
	widget.hobjs[RGB_QUAD] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[RGB_QUAD_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[RGB_HUE_SLIDER] = new WP_UnitRenderable(linear_hue_shader);
	widget.hobjs[RGB_HUE_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);

	setup_rect_uvs(RGB_QUAD);
	setup_quad_gradient_color(RGB_QUAD, RGB_QUAD_GRADIENT_INDEX);
	send_rgb_quad_hue_to_uniform(0.0f);
	setup_circle_cursor(RGB_QUAD_CURSOR);
	orient_rgb_hue_slider(Cardinal::RIGHT);
	setup_circle_cursor(RGB_HUE_SLIDER_CURSOR);

	widget.wp_at(RGB_QUAD).transform.position = { 0, -100 };
	widget.wp_at(RGB_QUAD).transform.scale = { 200, 200 };
	widget.wp_at(RGB_QUAD).pivot.y = 0;
	widget.wp_at(RGB_QUAD_CURSOR).transform.position = { widget.wp_at(RGB_QUAD).top(), widget.wp_at(RGB_QUAD).right() };
	widget.wp_at(RGB_HUE_SLIDER).transform.position = { 0, -115 };
	widget.wp_at(RGB_HUE_SLIDER).transform.scale = { 200, 15 };
	widget.wp_at(RGB_HUE_SLIDER).pivot.y = 1;
	widget.wp_at(RGB_HUE_SLIDER_CURSOR).transform.position = { widget.wp_at(RGB_HUE_SLIDER).left(), widget.wp_at(RGB_HUE_SLIDER).center_y() };
	widget.parent.position.y = 150;
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
				if (state == State::GRAPHIC_RGB)
				{
					if (widget.wp_at(RGB_QUAD).contains_point(local_cursor_pos))
					{
						current_widget_control = RGB_QUAD_CURSOR;
						mouse_handler_rgb_quad(local_cursor_pos);
					}
					else if (widget.wp_at(RGB_HUE_SLIDER).contains_point(local_cursor_pos))
					{
						current_widget_control = RGB_HUE_SLIDER_CURSOR;
						mouse_handler_rgb_hue_slider(local_cursor_pos);
					}
				}
			}
		}
		else if (mb.action == IAction::RELEASE)
		{
			if (current_widget_control == RGB_QUAD_CURSOR || current_widget_control == RGB_HUE_SLIDER_CURSOR)
				current_widget_control = -1;
		}
		};
	clbk_mb_down = [this](const Callback::MouseButton& mb) {
		Position local_cursor_pos = widget.parent.get_relative_pos(Machine.palette_cursor_world_pos());
		if (current_widget_control == RGB_QUAD_CURSOR)
			mouse_handler_rgb_quad(local_cursor_pos);
		else if (current_widget_control == RGB_HUE_SLIDER_CURSOR)
			mouse_handler_rgb_hue_slider(local_cursor_pos);
		};
	Machine.main_window->clbk_mouse_button.push_back(clbk_mb, this);
	Machine.main_window->clbk_mouse_button_down.push_back(clbk_mb_down, this);
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

void ColorPicker::setup_quad_gradient_color(size_t control, GLint gradient_index) const
{
	const UnitRenderable& renderable = ur_wget(widget, control);
	glm::vec4 gradient_indexes{ 0, 0, 1, gradient_index };
	renderable.set_attribute(2, glm::value_ptr(gradient_indexes));
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
	widget.wp_at(RGB_QUAD_CURSOR).transform.scale = { 8, 8 };
}

void ColorPicker::send_rgb_quad_hue_to_uniform(float hue)
{
	send_gradient_color_uniform(quad_shader, RGB_QUAD_GRADIENT_INDEX, ColorFrame(HSV(hue, 1.0f, 1.0f)).rgba_as_vec());
}

void ColorPicker::orient_rgb_hue_slider(Cardinal i) const
{
	const UnitRenderable& renderable = ur_wget(widget, RGB_HUE_SLIDER);
	float zero = 0.0f;
	float one = 1.0f;
	renderable.set_attribute_single_vertex(0, 1, i == Cardinal::DOWN || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(1, 1, i == Cardinal::RIGHT || i == Cardinal::DOWN ? &one : &zero);
	renderable.set_attribute_single_vertex(2, 1, i == Cardinal::UP || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(3, 1, i == Cardinal::RIGHT || i == Cardinal::UP ? &one : &zero);
}

glm::vec2 ColorPicker::get_rgb_quad_sat_and_value() const
{
	return widget.wp_at(RGB_QUAD).normalize(widget.wp_at(RGB_QUAD_CURSOR).transform.position);
}

float ColorPicker::get_rgb_hue_slider_hue() const
{
	return widget.wp_at(RGB_HUE_SLIDER).normalize_x(widget.wp_at(RGB_HUE_SLIDER_CURSOR).transform.position.x);
}

void ColorPicker::mouse_handler_rgb_quad(Position local_cursor_pos)
{
	widget.wp_at(RGB_QUAD_CURSOR).transform.position = widget.wp_at(RGB_QUAD).clamp_point(local_cursor_pos);
	sync_single_cp_widget_transform(RGB_QUAD_CURSOR);
}

void ColorPicker::mouse_handler_rgb_hue_slider(Position local_cursor_pos)
{
	widget.wp_at(RGB_HUE_SLIDER_CURSOR).transform.position.x = widget.wp_at(RGB_HUE_SLIDER).clamp_point(local_cursor_pos).x;
	widget.wp_at(RGB_HUE_SLIDER_CURSOR).transform.position.y = widget.wp_at(RGB_HUE_SLIDER).center_point().y;
	float hue = get_rgb_hue_slider_hue();
	if (on_interval(hue, 0.1f, 0.5f))
		set_circle_cursor_value(RGB_HUE_SLIDER_CURSOR, 0.0f);
	else
		set_circle_cursor_value(RGB_HUE_SLIDER_CURSOR, 1.0f);
	send_rgb_quad_hue_to_uniform(hue);
	sync_single_cp_widget_transform(RGB_HUE_SLIDER_CURSOR);
}

ColorFrame ColorPicker::get_color() const
{
	if (state == State::GRAPHIC_RGB)
	{
		// TODO add alpha
		glm::vec2 sv = get_rgb_quad_sat_and_value();
		return ColorFrame(HSV(get_rgb_hue_slider_hue(), sv[0], sv[1]));
	}
	return ColorFrame();
}
