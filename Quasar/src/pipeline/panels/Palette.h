#pragma once

#include "Panel.h"
#include "../FlatSprite.h"
#include "../Shader.h"
#include "../ColorPicker.h"

// TODO put common methods of Easel and Palette into (abstract?) Renderer class.
struct Palette : public Panel
{
	GLfloat* varr = nullptr;
	SharedFlatSprite background;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader sprite_shader;

	ColorPicker color_picker; // use internal vao

	Palette();
	Palette(const Palette&) = delete;
	Palette(Palette&&) noexcept = delete;
	~Palette();

	void subsend_background_vao() const;

	void _send_view() override;

	void draw() override;
};
