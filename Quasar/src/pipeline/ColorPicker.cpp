#include "ColorPicker.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "variety/Geometry.h"
#include "edit/Color.h"
#include "user/Machine.h"

static void send_gradient_color_uniform(Shader& shader, GLint index, glm::vec4 color)
{
	bind_shader(shader);
	QUASAR_GL(glUniform4fv(shader.uniform_locations["u_GradientColors[0]"] + index, 1, glm::value_ptr(color)));
}

static void standard_set_vertex_positions(UnitRenderable& renderable, const VertexQuad& quad)
{
	renderable.set_attribute_single_vertex(0, 0, glm::value_ptr(quad.bl));
	renderable.set_attribute_single_vertex(1, 0, glm::value_ptr(quad.br));
	renderable.set_attribute_single_vertex(2, 0, glm::value_ptr(quad.tl));
	renderable.set_attribute_single_vertex(3, 0, glm::value_ptr(quad.tr));
}

static void standard_set_rect_uvs(UnitRenderable& renderable)
{
	renderable.set_attribute_single_vertex(0, 1, glm::value_ptr(glm::vec2{ 0.0f, 0.0f }));
	renderable.set_attribute_single_vertex(1, 1, glm::value_ptr(glm::vec2{ 1.0f, 0.0f }));
	renderable.set_attribute_single_vertex(2, 1, glm::value_ptr(glm::vec2{ 0.0f, 1.0f }));
	renderable.set_attribute_single_vertex(3, 1, glm::value_ptr(glm::vec2{ 1.0f, 1.0f }));
}

static void send_gradient_quad_whiteblack_colors(Shader& quad_shader)
{
	send_gradient_color_uniform(quad_shader, 0, ColorFrame(HSV(0.0f, 0.0f, 0.0f)).rgba_as_vec());
	send_gradient_color_uniform(quad_shader, 1, ColorFrame(HSV(0.0f, 0.0f, 1.0f)).rgba_as_vec());
}

static void standard_setup_quad_gradient_color(UnitRenderable& renderable, GLint gradient_index)
{
	glm::vec4 gradient_indexes{ 0, 0, 1, gradient_index };
	renderable.set_attribute(2, glm::value_ptr(gradient_indexes));
}

static void orient_hue_progress(UnitRenderable& renderable, Cardinal i)
{
	float zero = 0.0f;
	float one = 1.0f;
	renderable.set_attribute_single_vertex(0, 1, i == Cardinal::DOWN  || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(1, 1, i == Cardinal::RIGHT || i == Cardinal::DOWN ? &one : &zero);
	renderable.set_attribute_single_vertex(2, 1, i == Cardinal::UP    || i == Cardinal::LEFT ? &one : &zero);
	renderable.set_attribute_single_vertex(3, 1, i == Cardinal::RIGHT || i == Cardinal::UP   ? &one : &zero);
}

static void orient_vertical_hue_progress(UnitRenderable& renderable)
{
	float zero = 0.0f;
	float one = 1.0f;
	renderable.set_attribute_single_vertex(0, 1, &zero);
	renderable.set_attribute_single_vertex(1, 1, &zero);
	renderable.set_attribute_single_vertex(2, 1, &one);
	renderable.set_attribute_single_vertex(3, 1, &one);
}

static void circle_cursor_set_thickness(UnitRenderable& renderable, float thickness)
{
	renderable.set_attribute(2, &thickness);
}

static void circle_cursor_set_value(UnitRenderable& renderable, float value)
{
	renderable.set_attribute(3, &value);
}

static float circle_cursor_get_value(UnitRenderable& renderable)
{
	float value;
	renderable.get_attribute(0, 3, &value);
	return value;
}

static constexpr const char* MAX_GRADIENT_COLORS = "4";
static constexpr GLint RGB_QUAD_GRADIENT_INDEX = 2;

// TODO use UMR when possible
enum
{
	// LATER ? use one CURSORS UMR, and implement visibility for subshapes in UMR.
	ALPHA_SLIDER,			// quad
	RGB_QUAD,				// quad
	RGB_QUAD_CURSOR,		// circle_cursor
	RGB_HUE_SLIDER,			// linear_hue
	RGB_HUE_SLIDER_CURSOR,			// circle_cursor
	HSV_WHEEL,				// hue_wheel
	HSV_VALUE_SLIDER,		// quad
	RGB_R_SLIDER,			// quad
	RGB_G_SLIDER,			// quad
	RGB_B_SLIDER,			// quad
	HSV_R_SLIDER,			// quad
	HSV_G_SLIDER,			// quad
	HSV_B_SLIDER,			// quad
	HSL_R_SLIDER,			// quad
	HSL_G_SLIDER,			// quad
	HSL_B_SLIDER,			// quad
	_CPW_COUNT
};

ColorPicker::ColorPicker()
	: quad_shader(FileSystem::resources_path("gradients/quad.vert"), FileSystem::resources_path("gradients/quad.frag.tmpl"), { {"$MAX_GRADIENT_COLORS", MAX_GRADIENT_COLORS } }),
	linear_hue_shader(FileSystem::resources_path("gradients/linear_hue.vert"), FileSystem::resources_path("gradients/linear_hue.frag")),
	hue_wheel_w_shader(FileSystem::resources_path("gradients/hue_wheel_w.vert"), FileSystem::resources_path("gradients/hue_wheel_w.frag")),
	circle_cursor_shader(FileSystem::resources_path("circle_cursor.vert"), FileSystem::resources_path("circle_cursor.frag")),
	widget(_CPW_COUNT)
{
	send_gradient_quad_whiteblack_colors(quad_shader);

	widget.hobjs[RGB_QUAD] = new WP_UnitRenderable(quad_shader);
	widget.hobjs[RGB_QUAD_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	widget.hobjs[RGB_HUE_SLIDER] = new WP_UnitRenderable(linear_hue_shader);
	widget.hobjs[RGB_HUE_SLIDER_CURSOR] = new WP_UnitRenderable(circle_cursor_shader);
	
	standard_set_rect_uvs(ur_wget(widget, RGB_QUAD));
	standard_setup_quad_gradient_color(ur_wget(widget, RGB_QUAD), RGB_QUAD_GRADIENT_INDEX);
	send_gradient_color_uniform(ur_wget(widget, RGB_QUAD).shader, RGB_QUAD_GRADIENT_INDEX, ColorFrame(HSV(0.0f, 1.0f, 1.0f)).rgba_as_vec());

	standard_set_rect_uvs(ur_wget(widget, RGB_QUAD_CURSOR));
	circle_cursor_set_thickness(ur_wget(widget, RGB_QUAD_CURSOR), 0.4f);
	circle_cursor_set_value(ur_wget(widget, RGB_QUAD_CURSOR), 1.0f);
	standard_set_rect_uvs(ur_wget(widget, RGB_HUE_SLIDER_CURSOR));
	circle_cursor_set_thickness(ur_wget(widget, RGB_HUE_SLIDER_CURSOR), 0.4f);
	circle_cursor_set_value(ur_wget(widget, RGB_HUE_SLIDER_CURSOR), 1.0f);

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

	orient_hue_progress(ur_wget(widget, RGB_HUE_SLIDER), Cardinal::RIGHT);

	widget.wp_at(RGB_QUAD).transform.position = { 0, -100 };
	widget.wp_at(RGB_QUAD).transform.scale = { 200, 200 };
	widget.wp_at(RGB_QUAD).pivot.y = 0;
	widget.wp_at(RGB_QUAD_CURSOR).transform.position = { widget.wp_at(RGB_QUAD).top(), widget.wp_at(RGB_QUAD).right() };
	widget.wp_at(RGB_QUAD_CURSOR).transform.scale = { 8, 8 };
	widget.wp_at(RGB_HUE_SLIDER).transform.position = { 0, -115 };
	widget.wp_at(RGB_HUE_SLIDER).transform.scale = { 200, 15 };
	widget.wp_at(RGB_HUE_SLIDER).pivot.y = 1;
	widget.wp_at(RGB_HUE_SLIDER_CURSOR).transform.position = { widget.wp_at(RGB_HUE_SLIDER).left(), widget.wp_at(RGB_HUE_SLIDER).center_y() };
	widget.wp_at(RGB_HUE_SLIDER_CURSOR).transform.scale = { 8, 8 };
	widget.parent.position.y = 150;
	sync_cp_widget_transforms();
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

void ColorPicker::sync_cp_widget_transforms()
{
	for (size_t i = 0; i < widget.hobjs.size(); ++i)
		sync_single_cp_widget_transform(i);
}

void ColorPicker::sync_single_cp_widget_transform(size_t i)
{
	if (widget.hobjs[i])
	{
		standard_set_vertex_positions(ur_wget(widget, i), VertexQuad(widget.wp_at(i).relative_to(widget.parent)));
		ur_wget(widget, i).send_buffer();
	}
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
		circle_cursor_set_value(ur_wget(widget, RGB_HUE_SLIDER_CURSOR), 0.0f);
	else
		circle_cursor_set_value(ur_wget(widget, RGB_HUE_SLIDER_CURSOR), 1.0f);
	send_gradient_color_uniform(ur_wget(widget, RGB_QUAD).shader, RGB_QUAD_GRADIENT_INDEX, ColorFrame(HSV(hue, 1.0f, 1.0f)).rgba_as_vec());
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
