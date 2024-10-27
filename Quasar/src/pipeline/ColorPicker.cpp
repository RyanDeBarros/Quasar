#include "ColorPicker.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"
#include "variety/Geometry.h"
#include "edit/Color.h"

static void send_gradient_color_uniform(Shader& shader, GLint index, glm::vec4 color)
{
	bind_shader(shader);
	QUASAR_GL(glUniform4fv(shader.uniform_locations["u_GradientColors[0]"] + index, 1, glm::value_ptr(color)));
}

static void standard_set_vertex_positions(UnitRenderable& renderable, glm::vec2 bl, glm::vec2 br, glm::vec2 tl, glm::vec2 tr)
{
	renderable.set_attribute_single_vertex(0, 0, glm::value_ptr(bl));
	renderable.set_attribute_single_vertex(1, 0, glm::value_ptr(br));
	renderable.set_attribute_single_vertex(2, 0, glm::value_ptr(tl));
	renderable.set_attribute_single_vertex(3, 0, glm::value_ptr(tr));
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

ColorPicker::ColorPicker()
	: quad_shader(FileSystem::resources_path("gradients/quad.vert"), FileSystem::resources_path("gradients/quad.frag.tmpl"), { {"$MAX_GRADIENT_COLORS", "4" } }),
	linear_hue_shader(FileSystem::resources_path("gradients/linear_hue.vert"), FileSystem::resources_path("gradients/linear_hue.frag")),
	hue_wheel_w_shader(FileSystem::resources_path("gradients/hue_wheel_w.vert"), FileSystem::resources_path("gradients/hue_wheel_w.frag")),
	graphic_rgb_quad_gradient(quad_shader),
	graphic_rgb_linear_hue(linear_hue_shader)
{
	send_gradient_quad_whiteblack_colors(quad_shader);

	static constexpr GLint RGB_QUAD_GRADIENT_INDEX = 2;

	standard_set_vertex_positions(graphic_rgb_quad_gradient, { -100, -100 }, { 100, -100 }, { -100, 100 }, { 100, 100 });
	standard_set_rect_uvs(graphic_rgb_quad_gradient);
	standard_setup_quad_gradient_color(graphic_rgb_quad_gradient, RGB_QUAD_GRADIENT_INDEX);
	graphic_rgb_quad_gradient.send_buffer();
	send_gradient_color_uniform(graphic_rgb_quad_gradient.shader, RGB_QUAD_GRADIENT_INDEX, ColorFrame(HSV(0.4f, 1.0f, 1.0f)).rgba_as_vec());

	standard_set_vertex_positions(graphic_rgb_linear_hue, { -100, -130 }, { 100, -130 }, { -100, -115 }, { 100, -115 });
	orient_hue_progress(graphic_rgb_linear_hue, Cardinal::RIGHT);
	graphic_rgb_linear_hue.send_buffer();
}

void ColorPicker::render() const
{
	switch (state)
	{
	case State::GRAPHIC_RGB:
		graphic_rgb_quad_gradient.draw();
		graphic_rgb_linear_hue.draw();
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
}
