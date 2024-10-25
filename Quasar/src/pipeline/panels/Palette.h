#pragma once

#include "../FlatSprite.h"
#include "../Shader.h"

// TODO put common methods of Easel and Palette into (abstract?) Renderer class.
struct Palette
{
	GLfloat* varr = nullptr;
	SharedFlatSprite background;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader sprite_shader;

	// View
	glm::mat3 projection{};
	FlatTransform view{};
	float view_scale = 1.0f;
private:
	Scale app_scale;
public:

	Palette();
	Palette(const Palette&) = delete;
	Palette(Palette&&) noexcept = delete;
	~Palette();

	void set_projection(float width, float height);
	void set_projection();

	void subsend_background_vao() const;

	void send_view();
	glm::mat3 vp_matrix() const;

	void render() const;
};
