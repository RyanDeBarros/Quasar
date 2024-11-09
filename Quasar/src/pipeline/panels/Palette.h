#pragma once

#include "Panel.h"
#include "../render/FlatSprite.h"
#include "../render/Shader.h"
#include "../widgets/Widget.h"

struct Palette : public Panel
{
	GLfloat* varr = nullptr;
	SharedFlatSprite background;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader sprite_shader;
	glm::mat3 vp;

	Widget widget;

	Palette();
	Palette(const Palette&) = delete;
	Palette(Palette&&) noexcept = delete;
	~Palette();

	void subsend_background_vao() const;
	void _send_view() override;
	void draw() override;
	void render_widget();

private:
	void initialize_widget();

	enum : size_t
	{
		COLOR_PICKER,
		COLOR_PALETTE,
		_W_COUNT
	};
};
