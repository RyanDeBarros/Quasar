#pragma once

#include "Geometry.h"
#include "Referencers.h"
#include "Resources.h"

struct Sprite
{
	static constexpr unsigned short RECT_STRIDE = 14;

	// TODO just transition to registries and handles?
	BufferReferencer* buf = nullptr;
	ImageHandle image;
	Transform transform;

	Sprite(ImageHandle image);
	Sprite(const Sprite&);
	Sprite(Sprite&&) noexcept;
	Sprite& operator=(const Sprite&);
	Sprite& operator=(Sprite&&) noexcept;
	~Sprite();

	void set_buf_ref(BufferReferencer* buf);

	void on_draw(class Renderer*) const;
	void sync_transform() const;
	void sync_transform_p() const;
	void sync_transform_rs() const;
};
