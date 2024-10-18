#pragma once

#include "Macros.h"
#include "variety/Geometry.h"
#include "Shader.h"
#include "user/Platform.h"

struct QuasarSettings
{
	static unsigned short VERTEX_COUNT;
	static unsigned short INDEX_COUNT;
	static unsigned short TEXTURES_COUNT;

	static void load_settings(const char* filepath = "res/settings.asset");
};

struct ClippingRect
{
	GLint x, y;
	GLsizei screen_w, screen_h;

	std::function<glm::ivec4(int, int)> window_size_to_bounds;

	bool contains_point(const glm::vec2& point) const
	{
		return x <= point.x && point.x < x + screen_w && y <= point.y && point.y < y + screen_h;
	}
	void update_window_size(int width, int height)
	{
		if (!window_size_to_bounds) return;
		glm::ivec4 bnds = window_size_to_bounds(width, height);
		x = bnds[0];
		y = bnds[1];
		screen_w = bnds[2];
		screen_h = bnds[3];
	}
	glm::vec2 center_point() const
	{
		return { 0.5f * (x + screen_w), 0.5f * (y + screen_h) };
	}
};

class Renderer
{
	// Batches
	Window* window;
	GLfloat* const vertex_pool = nullptr;
	GLuint* const index_pool = nullptr;
	GLfloat* vertex_pos = nullptr;
	GLuint* index_pos = nullptr;
	GLuint* const texture_slots = nullptr;
	unsigned short num_vertices = 0;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader shader;
	unsigned char texture_slot_cap = 0;
	std::vector<struct Sprite*> _sprites;
	
	// View
	glm::mat3 projection;
	Transform view;
	Scale app_scale;
	ClippingRect clip{};

	void set_projection(float width, float height);
	void set_projection();

public:
	Renderer(Window* window, Shader&& shader);
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	~Renderer();

	void bind() const;
	void unbind() const;
	void prepare_for_sprite();
	void pool_over_varr(GLfloat* varr);
	void on_render();
	void flush() const;
	void reset();
	void frame_cycle() { bind(); on_render(); unbind(); }

	const Transform& get_view() const { return view; }
	void set_view(const Transform& view);
	void set_view_position(const Position& pos);
	void set_view_rotation(Rotation rot);
	void set_view_scale(const Scale& sca);
	glm::vec2 to_world_coordinates(const glm::vec2& screen_coordinates) const;
	glm::vec2 to_screen_coordinates(const glm::vec2& world_coordinates) const;

	void set_app_scale(float x = 1.0f, float y = 1.0f);
	glm::vec2 get_app_scale() const;
	unsigned short get_texture_slot(GLuint texture);

	std::vector<struct Sprite*>& sprites() { return _sprites; };
	const std::vector<struct Sprite*>& sprites() const { return _sprites; };
	ClippingRect& clipping_rect() { return clip; }
	const ClippingRect& clipping_rect() const { return clip; }
	bool cursor_in_clipping() const { return clip.contains_point(window->cursor_pos()); }
	float get_app_width() const { return clip.screen_w * app_scale.x; }
	float get_app_height() const { return clip.screen_h * app_scale.y; }
	glm::vec2 get_app_cursor_pos() const { return window->cursor_pos() * app_scale; }

	void set_window_resize_callback();
	Window* get_window() const { return window; }

private:
	void send_view();
};
