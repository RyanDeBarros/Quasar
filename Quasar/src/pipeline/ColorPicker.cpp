#include "ColorPicker.h"

#include "variety/GLutility.h"

ColorPicker::ColorPicker()
	: quad_shader(FileSystem::resources_path("gradients/quad.vert"), FileSystem::resources_path("gradients/quad.frag"), { 2, 4 }, { "u_VP" }),
	linear_hue_shader(FileSystem::resources_path("gradients/linear_hue.vert"), FileSystem::resources_path("gradients/linear_hue.frag"), { 2, 1 }, { "u_VP" }),
	hue_wheel_w_shader(FileSystem::resources_path("gradients/hue_wheel_w.vert"), FileSystem::resources_path("gradients/hue_wheel_w.frag"), { 2, 2, 1 }, { "u_VP" }),
	graphic_rgb_quad_gradient(quad_shader),
	graphic_rgb_linear_hue(linear_hue_shader)
{
	glm::vec2 pos;
	pos = { -100, -100 };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(0, 0, &pos.x);
	pos = { 100, -100 };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(1, 0, &pos.x);
	pos = { -100, 100 };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(2, 0, &pos.x);
	pos = { 100, 100 };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(3, 0, &pos.x);
	glm::vec4 clr;
	clr = { 0.0f, 0.0f, 0.0f, 1.0f };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(0, 1, &clr.x);
	clr = { 0.0f, 0.0f, 0.0f, 1.0f };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(1, 1, &clr.x);
	clr = { 1.0f, 1.0f, 1.0f, 1.0f };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(2, 1, &clr.x);
	clr = { 1.0f, 0.0f, 0.0f, 1.0f };
	graphic_rgb_quad_gradient.set_attribute_single_vertex(3, 1, &clr.x);
	graphic_rgb_quad_gradient.send_buffer();
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
