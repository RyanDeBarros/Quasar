#include "RoundRect.h"

#include <glm/gtc/type_ptr.inl>

RoundRect::RoundRect(Shader* round_rect_shader)
	: WP_UnitRenderable(round_rect_shader)
{
}

void RoundRect::draw() const
{
	ur->draw();
}

const RoundRect& RoundRect::update_transform() const
{
	auto gm = global_matrix();
	float left = wp_left(gm);
	float right = wp_right(gm);
	float bottom = wp_bottom(gm);
	float top = wp_top(gm);
	ur->set_attribute_single_vertex(0, VERTEX_POS, glm::value_ptr(glm::vec2{ left, bottom }));
	ur->set_attribute_single_vertex(1, VERTEX_POS, glm::value_ptr(glm::vec2{ right, bottom }));
	ur->set_attribute_single_vertex(2, VERTEX_POS, glm::value_ptr(glm::vec2{ left, top }));
	ur->set_attribute_single_vertex(3, VERTEX_POS, glm::value_ptr(glm::vec2{ right, top }));
	ur->set_attribute(BOTTOM_LEFT, glm::value_ptr(glm::vec2{ left, bottom }));
	ur->set_attribute(RECT_SIZE, glm::value_ptr(glm::vec2{ right - left, top - bottom }));
	return *this;
}

const RoundRect& RoundRect::update_border_color() const
{
	ur->set_attribute(BORDER_COLOR, glm::value_ptr(border_color.as_vec()));
	return *this;
}

const RoundRect& RoundRect::update_fill_color() const
{
	ur->set_attribute(FILL_COLOR, glm::value_ptr(fill_color.as_vec()));
	return *this;
}

const RoundRect& RoundRect::update_corner_radius() const
{
	ur->set_attribute(CORNER_RADIUS, &corner_radius);
	return *this;
}

const RoundRect& RoundRect::update_thickness() const
{
	ur->set_attribute(THICKNESS, &thickness);
	return *this;
}

const RoundRect& RoundRect::update_all() const
{
	return update_transform().update_border_color().update_fill_color().update_corner_radius().update_thickness();
}

void RoundRect::send_buffer() const
{
	ur->send_buffer();
}
