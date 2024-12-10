#include "FlatSprite.h"

#include <glm/gtc/type_ptr.inl>

#include "ImplUtility.h"
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
	Utils::set_vertex_pos_attributes(*ur, global_matrix(), 0, SHADER_POS_VERT_POS, false);
	return *this;
}

FlatSprite& FlatSprite::update_transform()
{
	Utils::set_vertex_pos_attributes(*ur, global_matrix(), 0, SHADER_POS_VERT_POS, false);
	return *this;
}

const FlatSprite& FlatSprite::set_texture_slot(float texture_slot) const
{
	ur->set_attribute(SHADER_POS_TEXTURE, &texture_slot);
	return *this;
}

FlatSprite& FlatSprite::set_texture_slot(float texture_slot)
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

FlatSprite& FlatSprite::set_modulation(const glm::vec4& color)
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

FlatSprite& FlatSprite::set_modulation(ColorFrame color)
{
	ur->set_attribute(SHADER_POS_MODULATE, glm::value_ptr(color.rgba().as_vec()));
	return *this;
}

const FlatSprite& FlatSprite::set_uvs(const Bounds& bounds) const
{
	Utils::set_uv_attributes(*ur, bounds, 0, SHADER_POS_UV, false);
	return *this;
}

FlatSprite& FlatSprite::set_uvs(const Bounds& bounds)
{
	Utils::set_uv_attributes(*ur, bounds, 0, SHADER_POS_UV, false);
	return *this;
}

SpriteMoveAction::SpriteMoveAction(FlatSprite* sprite, Position delta)
	: sprite(sprite), delta(delta)
{
	weight = sizeof(SpriteMoveAction);
}

void SpriteMoveAction::forward()
{
	sprite->self.transform.position += delta;
	sprite->update_transform().ur->send_buffer();
}

void SpriteMoveAction::backward()
{
	sprite->self.transform.position -= delta;
	sprite->update_transform().ur->send_buffer();
}
