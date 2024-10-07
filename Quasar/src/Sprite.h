#pragma once

#include "Geometry.h"
#include "Resources.h"

struct Sprite
{
	static constexpr unsigned char NUM_VERTICES = 4;
	static constexpr unsigned char NUM_INDICES = 6;
	static constexpr unsigned char STRIDE = 14;
	static constexpr unsigned char ILEN_BYTES = NUM_INDICES * sizeof(GLuint);
	static constexpr unsigned char VLEN_BYTES = size_t(NUM_VERTICES) * STRIDE * sizeof(GLfloat);
	static constexpr GLuint IARR[6]{ 0, 1, 2, 2, 3, 0 };
	
	GLfloat* varr = nullptr;
	Transform transform;
	ImageHandle image;

	Sprite(ImageHandle image);
	Sprite(const Sprite&);
	Sprite(Sprite&&) noexcept;
	Sprite& operator=(const Sprite&);
	Sprite& operator=(Sprite&&) noexcept;
	~Sprite();

	void on_draw(class Renderer*) const;
	void sync_transform() const;
	void sync_transform_p() const;
	void sync_transform_rs() const;
};
