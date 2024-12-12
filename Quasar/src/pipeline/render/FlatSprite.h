#pragma once

#include "variety/Geometry.h"
#include "edit/color/Color.h"
#include "edit/image/Image.h"
#include "../widgets/Widget.h"
#include "variety/History.h"

struct FlatSprite : public W_UnitRenderable
{
	std::shared_ptr<Image> image;
	
	FlatSprite(Shader* shader);
	FlatSprite(const FlatSprite&) = delete;
	FlatSprite(FlatSprite&&) noexcept = delete;

	void draw(GLuint texture_slot);
	
	const FlatSprite& update_transform() const;
	FlatSprite& update_transform();
	const FlatSprite& set_texture_slot(float texture_slot) const;
	FlatSprite& set_texture_slot(float texture_slot);
	glm::vec4 modulation() const;
	const FlatSprite& set_modulation(const glm::vec4& color) const;
	FlatSprite& set_modulation(const glm::vec4& color);
	ColorFrame modulation_color_frame() const;
	const FlatSprite& set_modulation(ColorFrame color) const;
	FlatSprite& set_modulation(ColorFrame color);
	const FlatSprite& set_uvs(const Bounds& bounds) const;
	FlatSprite& set_uvs(const Bounds& bounds);
};

inline const FlatSprite& fs_wget(const Widget& w, size_t i)
{
	return *w.get<FlatSprite>(i);
}

inline FlatSprite& fs_wget(Widget& w, size_t i)
{
	return *w.get<FlatSprite>(i);
}

struct SpriteMoveAction : public ActionBase
{
	FlatSprite* sprite;
	Position delta;
	SpriteMoveAction(FlatSprite* sprite, Position delta);
	virtual void forward() override;
	virtual void backward() override;
	QUASAR_ACTION_EQUALS_OVERRIDE(SpriteMoveAction)
};
