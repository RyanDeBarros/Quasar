#include "RoundRect.h"

#include <glm/gtc/type_ptr.inl>

RoundRect::RoundRect(Shader* round_rect_shader)
	: WP_UnitRenderable(round_rect_shader)
{
}

void RoundRect::update_transform() const
{
	glm::vec2 bl = { held.left(), held.bottom() };
	ur->set_attribute_single_vertex(0, VERTEX_POS, glm::value_ptr(bl));
	ur->set_attribute_single_vertex(1, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ held.transform.scale.x, 0 }));
	ur->set_attribute_single_vertex(2, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ 0, held.transform.scale.y }));
	ur->set_attribute_single_vertex(3, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ held.transform.scale.x, held.transform.scale.y }));
	ur->set_attribute(BOTTOM_LEFT, glm::value_ptr(bl));
	ur->set_attribute(RECT_SIZE, glm::value_ptr(held.transform.scale));
}

void RoundRect::send_transform_under_parent(FlatTransform parent) const
{
	WidgetPlacement wp = held.relative_to(parent);
	glm::vec2 bl = { wp.left(), wp.bottom() };
	ur->set_attribute_single_vertex(0, VERTEX_POS, glm::value_ptr(bl));
	ur->set_attribute_single_vertex(1, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ wp.transform.scale.x, 0 }));
	ur->set_attribute_single_vertex(2, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ 0, wp.transform.scale.y }));
	ur->set_attribute_single_vertex(3, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ wp.transform.scale.x, wp.transform.scale.y }));
	ur->set_attribute(BOTTOM_LEFT, glm::value_ptr(bl));
	ur->set_attribute(RECT_SIZE, glm::value_ptr(wp.transform.scale));
	ur->send_buffer();
}

void RoundRect::update_border_color() const
{
	ur->set_attribute(BORDER_COLOR, glm::value_ptr(border_color.as_vec()));
}

void RoundRect::update_fill_color() const
{
	ur->set_attribute(FILL_COLOR, glm::value_ptr(fill_color.as_vec()));
}

void RoundRect::update_corner_radius() const
{
	ur->set_attribute(CORNER_RADIUS, &corner_radius);
}

void RoundRect::update_thickness() const
{
	ur->set_attribute(THICKNESS, &thickness);
}

void RoundRect::update_all() const
{
	update_transform();
	update_border_color();
	update_fill_color();
	update_corner_radius();
	update_thickness();
}
