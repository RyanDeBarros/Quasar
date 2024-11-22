#include "FlatSprite.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"

constexpr size_t SHADER_POS_TEXTURE = 0;
constexpr size_t SHADER_POS_VERT_POS = 1;
constexpr size_t SHADER_POS_UV = 2;
constexpr size_t SHADER_POS_MODULATE = 3;

FlatSprite::FlatSprite(Shader* shader)
	: W_UnitRenderable(shader)
{
	set_texture_slot(0);
	set_uvs(Bounds{ 0, 1, 0, 1 });
	update_transform();
	set_modulation(glm::vec4{ 1, 1, 1, 1 });
}

void FlatSprite::draw(GLuint texture_slot)
{
	if (image)
	{
		bind_texture(image->tid, texture_slot);
		W_UnitRenderable::draw();
	}
}

const FlatSprite& FlatSprite::update_transform() const
{
	glm::mat3 m = global_matrix();
	ur->set_attribute_single_vertex(0, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ wp_left(m),  wp_bottom(m) }))
		.set_attribute_single_vertex(1, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ wp_right(m), wp_bottom(m) }))
		.set_attribute_single_vertex(2, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ wp_left(m),  wp_top(m) }))
		.set_attribute_single_vertex(3, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ wp_right(m), wp_top(m) }));
	return *this;
}

const FlatSprite& FlatSprite::set_texture_slot(float texture_slot) const
{
	ur->set_attribute(SHADER_POS_TEXTURE, &texture_slot);
	return *this;
}

glm::vec4 FlatSprite::modulation() const
{
	glm::vec4 v;
	ur->get_attribute(0, SHADER_POS_MODULATE, glm::value_ptr(v));
	return v;
}

const FlatSprite& FlatSprite::set_modulation(const glm::vec4& color) const
{
	ur->set_attribute(SHADER_POS_MODULATE, glm::value_ptr(color));
	return *this;
}

ColorFrame FlatSprite::modulation_color_frame() const
{
	glm::vec4 v;
	ur->get_attribute(0, SHADER_POS_MODULATE, glm::value_ptr(v));
	return ColorFrame(RGB(v.r, v.g, v.b), v.a);
}

const FlatSprite& FlatSprite::set_modulation(ColorFrame color) const
{
	ur->set_attribute(SHADER_POS_MODULATE, glm::value_ptr(color.rgba().as_vec()));
	return *this;
}

const FlatSprite& FlatSprite::set_uvs(const Bounds& bounds) const
{
	ur->set_attribute_single_vertex(0, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x1, bounds.y1 }))
		.set_attribute_single_vertex(1, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x2, bounds.y1 }))
		.set_attribute_single_vertex(2, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x1, bounds.y2 }))
		.set_attribute_single_vertex(3, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x2, bounds.y2 }));
	return *this;
}
