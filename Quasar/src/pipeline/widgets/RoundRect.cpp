#include "RoundRect.h"

#include <glm/gtc/type_ptr.inl>

RoundRect::RoundRect(Shader* round_rect_shader)
	: WP_UnitRenderable(round_rect_shader)
{
}

const RoundRect& RoundRect::update_transform() const
{
	if (parent)
		update_transform(self.relative_to(parent->self.transform)); // TODO relative_to should take in widget placement ?? also, this should be a global() call chain.
	else
		update_transform(self);
	return *this;
}

const RoundRect& RoundRect::update_transform(const WidgetPlacement& wp) const
{
	glm::vec2 bl = { wp.left(), wp.bottom() };
	ur->set_attribute_single_vertex(0, VERTEX_POS, glm::value_ptr(bl));
	ur->set_attribute_single_vertex(1, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ wp.transform.scale.x, 0 }));
	ur->set_attribute_single_vertex(2, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ 0, wp.transform.scale.y }));
	ur->set_attribute_single_vertex(3, VERTEX_POS, glm::value_ptr(bl + glm::vec2{ wp.transform.scale.x, wp.transform.scale.y }));
	ur->set_attribute(BOTTOM_LEFT, glm::value_ptr(bl));
	ur->set_attribute(RECT_SIZE, glm::value_ptr(wp.transform.scale));
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

void RoundRect::send_vp(const glm::mat3& vp) const
{

}
