#pragma once

#include "../FlatSprite.h"
#include "../Shader.h"
#include "../ColorPicker.h"

// TODO put common methods of Easel and Palette into (abstract?) Renderer class.
struct Palette
{
	GLfloat* varr = nullptr;
	SharedFlatSprite background;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader sprite_shader;

	ColorPicker color_picker; // use internal vao

	// View
	glm::mat3 projection{};
	FlatTransform view{};
private:
	Scale app_scale;
public:

	Palette();
	Palette(const Palette&) = delete;
	Palette(Palette&&) noexcept = delete;
	~Palette();

	void set_projection();

	void subsend_background_vao() const;

	void send_view();
	glm::mat3 vp_matrix() const;

	void render() const;

	void set_app_scale(Scale sc);
	Scale get_app_scale() const;

	bool cursor_in_clipping() const;
	float get_app_width() const;
	float get_app_height() const;
	glm::vec2 get_app_cursor_pos() const;
};
