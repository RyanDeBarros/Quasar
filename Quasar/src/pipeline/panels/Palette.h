#pragma once

#include "Panel.h"
#include "../render/FlatSprite.h"
#include "../render/Shader.h"
#include "../widgets/ColorPicker.h"

struct Palette : public Panel
{
	GLfloat* varr = nullptr;
	SharedFlatSprite background;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader sprite_shader;
	glm::mat3 vp;
	ColorPicker color_picker;

	Palette();
	Palette(const Palette&) = delete;
	Palette(Palette&&) noexcept = delete;
	~Palette();

	void subsend_background_vao() const;

	void _send_view() override;

	void draw() override;
};
