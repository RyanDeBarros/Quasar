#pragma once

#include "Sprite.h"

struct Checkerboard : public Sprite // TODO migrate out of Sprite
{
	RGBA c1, c2;

	Checkerboard(RGBA c1, RGBA c2);
	void sync_colors() const;
	void sync_texture() const;
	void set_uv_size(float width, float height) const;
};

struct Canvas
{
	Image* image = nullptr;
	Sprite sprite;
	Checkerboard checkerboard;
	float checker_size = 16.0f; // LATER settings

	Canvas(RGBA c1, RGBA c2);

	void set_image(ImageHandle img);

	Transform& transform() { return sprite.transform; }
	Position& position() { return sprite.transform.position; }
	Rotation& rotation() { return sprite.transform.rotation; }
	Scale& scale() { return sprite.transform.scale; }

	void sync_transform();
	void sync_transform_p();
	void sync_transform_rs();
};
