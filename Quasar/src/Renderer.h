#pragma once

#include "Macros.h"
#include "Resources.h"
#include "Geometry.h"
#include "Shader.h"
#include "Platform.h"

struct QuasarSettings
{
	static unsigned short VERTEX_COUNT;
	static unsigned short INDEX_COUNT;
	static unsigned short TEXTURES_COUNT;

	static void load_settings(const char* filepath = "res/settings.asset");
};

class Renderer
{
	GLfloat* const vertex_pool = nullptr;
	GLuint* const index_pool = nullptr;
	GLfloat* vertex_pos = nullptr;
	GLuint* index_pos = nullptr;
	Window* window;
	GLuint* const texture_slots = nullptr;
	unsigned short num_vertices = 0;
	glm::mat3 projection;
	Transform view;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader shader;
	unsigned char texture_slot_cap = 0;
	std::vector<struct Sprite*> _sprites;
	float app_scale_x = 1.0f;
	float app_scale_y = 1.0f;

	void set_projection(float width, float height);
	void set_projection();

public:
	Renderer(Window* window, Shader&& shader);
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	~Renderer();

	void bind() const;
	void prepare_for_sprite();
	void pool_over_varr(GLfloat* varr);
	void on_draw();
	void flush() const;
	void reset();
	const Transform& get_view() const { return view; }
	void set_view(const Transform& view);
	void set_app_scale(float x = 1.0f, float y = 1.0f);
	glm::vec2 get_app_scale() const { return { app_scale_x, app_scale_y }; }
	unsigned short get_texture_slot(GLuint texture);

	std::vector<struct Sprite*>& sprites() { return _sprites; };

	void set_window_callbacks();
	Window* get_window() const { return window; }
};
