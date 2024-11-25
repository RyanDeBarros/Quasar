#include "RoundRect.h"

#include <glm/gtc/type_ptr.inl>

#include "ImplUtility.h"

RoundRect::RoundRect(Shader* round_rect_shader)
	: W_UnitRenderable(round_rect_shader)
{
}

void RoundRect::draw()
{
	ur->draw();
}

const RoundRect& RoundRect::update_transform() const
{
	auto gm = global_matrix();
	Utils::set_vertex_pos_attributes(*ur, gm, 0, VERTEX_POS, false);
	Utils::set_rect_dimension_attributes(*ur, gm, BOTTOM_LEFT, RECT_SIZE, false);
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

const RoundRect& RoundRect::update_corner_radius(float scale) const
{
	scale *= corner_radius;
	ur->set_attribute(CORNER_RADIUS, &scale);
	return *this;
}

const RoundRect& RoundRect::update_thickness(float scale) const
{
	scale *= thickness;
	ur->set_attribute(THICKNESS, &scale);
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
