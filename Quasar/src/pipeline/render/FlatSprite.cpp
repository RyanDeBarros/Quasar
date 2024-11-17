#include "FlatSprite.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"

constexpr size_t SHADER_POS_TEXTURE = 0;
constexpr size_t SHADER_POS_VERT_POS = 1;
constexpr size_t SHADER_POS_UV = 2;
constexpr size_t SHADER_POS_PACKED = 3;
constexpr size_t SHADER_POS_MODULATE = 4;

FlatSprite::FlatSprite(Shader* shader)
	: W_UnitRenderable(shader)
{
	set_texture_slot(0);
	set_image_dimensions();
	set_uvs(Bounds{ 0, 1, 0, 1 });
	sync_transform();
	set_modulation(glm::vec4{ 1, 1, 1, 1 });
}

void FlatSprite::sync_transform() const
{
	// TODO use vertex positions instead, as they can be calculated here. eventually don't even use packed() honestly.
	ur->set_attribute(SHADER_POS_PACKED, glm::value_ptr(self.transform.packed()));
}

void FlatSprite::set_image_dimensions(Dim v_width, Dim v_height) const
{
	Dim w = v_width >= 0 ? v_width : (image ? image->buf.width : 0);
	Dim h = v_height >= 0 ? v_height : (image ? image->buf.height : 0);

	ur->set_attribute_single_vertex(0, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ -0.5f * w, -0.5f * h }));
	ur->set_attribute_single_vertex(1, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ 0.5f * w, -0.5f * h}));
	ur->set_attribute_single_vertex(2, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ -0.5f * w, 0.5f * h}));
	ur->set_attribute_single_vertex(3, SHADER_POS_VERT_POS, glm::value_ptr(glm::vec2{ 0.5f * w, 0.5f * h}));
}

void FlatSprite::set_texture_slot(float texture_slot) const
{
	ur->set_attribute(SHADER_POS_TEXTURE, &texture_slot);
}

glm::vec4 FlatSprite::modulation() const
{
	glm::vec4 v;
	ur->get_attribute(0, SHADER_POS_MODULATE, glm::value_ptr(v));
	return v;
}

void FlatSprite::set_modulation(const glm::vec4& color) const
{
	ur->set_attribute(SHADER_POS_MODULATE, glm::value_ptr(color));
}

ColorFrame FlatSprite::modulation_color_frame() const
{
	glm::vec4 v;
	ur->get_attribute(0, SHADER_POS_MODULATE, glm::value_ptr(v));
	return ColorFrame(RGB(v.r, v.g, v.b), v.a);
}

void FlatSprite::set_modulation(ColorFrame color) const
{
	ur->set_attribute(SHADER_POS_MODULATE, glm::value_ptr(color.rgba().as_vec()));
}

void FlatSprite::set_uvs(const Bounds& bounds) const
{
	ur->set_attribute_single_vertex(0, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x1, bounds.y1 }));
	ur->set_attribute_single_vertex(1, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x2, bounds.y1 }));
	ur->set_attribute_single_vertex(2, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x1, bounds.y2 }));
	ur->set_attribute_single_vertex(3, SHADER_POS_UV, glm::value_ptr(glm::vec2{ bounds.x2, bounds.y2 }));
}
