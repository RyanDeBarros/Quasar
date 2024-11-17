#pragma once

#include "variety/Geometry.h"
#include "edit/color/Color.h"
#include "edit/image/Image.h"
#include "../widgets/Widget.h"

// LATER use renderables here

struct FlatSprite : public W_UnitRenderable
{
	std::shared_ptr<Image> image;

	FlatSprite(Shader* shader);
	FlatSprite(const FlatSprite&) = delete;
	FlatSprite(FlatSprite&&) noexcept = delete;
	
	void sync_transform() const;

	void set_image(std::shared_ptr<Image>&& img, Dim v_width = -1, Dim v_height = -1) { image = std::move(img); set_image_dimensions(v_width, v_height); }
	void set_image(const std::shared_ptr<Image>& img, Dim v_width = -1, Dim v_height = -1) { image = img; set_image_dimensions(v_width, v_height); }
	void set_image_dimensions(Dim v_width = -1, Dim v_height = -1) const;
	void set_texture_slot(float texture_slot) const;

	glm::vec4 modulation() const;
	void set_modulation(const glm::vec4& color) const;
	ColorFrame modulation_color_frame() const;
	void set_modulation(ColorFrame color) const;

	void set_uvs(const Bounds& bounds) const;
};
