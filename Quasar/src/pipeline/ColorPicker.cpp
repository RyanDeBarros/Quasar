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

template<size_t n>
static void sync_cp_widget_transforms(CPWidget<n>& cpw)
{
	for (size_t i = 0; i < n; ++i)
	{
		standard_set_vertex_positions(cpw[i], VertexQuad(cpw.wps[i].relative_to(cpw.parent)));
		cpw[i].send_buffer();
	}
}

ColorPicker::ColorPicker()
	: quad_shader(FileSystem::resources_path("gradients/quad.vert"), FileSystem::resources_path("gradients/quad.frag.tmpl"), { {"$MAX_GRADIENT_COLORS", MAX_GRADIENT_COLORS } }),
	linear_hue_shader(FileSystem::resources_path("gradients/linear_hue.vert"), FileSystem::resources_path("gradients/linear_hue.frag")),
	hue_wheel_w_shader(FileSystem::resources_path("gradients/hue_wheel_w.vert"), FileSystem::resources_path("gradients/hue_wheel_w.frag")),
	graphic_rgb(quad_shader, linear_hue_shader)
{
	send_gradient_quad_whiteblack_colors(quad_shader);

	standard_set_rect_uvs(graphic_rgb[0]);
	standard_setup_quad_gradient_color(graphic_rgb[0], RGB_QUAD_GRADIENT_INDEX);
	send_gradient_color_uniform(graphic_rgb[0].shader, RGB_QUAD_GRADIENT_INDEX, ColorFrame(HSV(0.4f, 1.0f, 1.0f)).rgba_as_vec());

	orient_hue_progress(graphic_rgb[1], Cardinal::RIGHT);

	graphic_rgb.wps[0].transform.position = { 0, -100 };
	graphic_rgb.wps[0].size = { 200, 200 };
	graphic_rgb.wps[0].pivot.y = 0;
	graphic_rgb.wps[1].transform.position = { 0, -115 };
	graphic_rgb.wps[1].size = { 200, 15 };
	graphic_rgb.wps[1].pivot.y = 1;
	graphic_rgb.parent.position.y = 200;
	sync_cp_widget_transforms<>(graphic_rgb);
}

void ColorPicker::render() const
{
	switch (state)
	{
	case State::GRAPHIC_RGB:
		graphic_rgb[0].draw();
		graphic_rgb[1].draw();
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
