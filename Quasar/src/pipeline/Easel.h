#pragma once

#include "user/Platform.h"
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

struct Gridlines
{
	unsigned short width = 0, height = 0;
	GLuint vao = 0, vb = 0;
	Shader shader;
	GLfloat* varr = nullptr;
	float line_spacing = 1.0f;
	float line_width_scale = 0.05f;

	Gridlines();
	Gridlines(const Gridlines&) = delete;
	Gridlines(Gridlines&&) noexcept = delete;
	~Gridlines();

	void sync_grid();
	void draw(float canvas_scale) const;

	unsigned short num_cols() const { return unsigned short(width / line_spacing) + 1_US; }
	unsigned short num_rows() const { return unsigned short(height / line_spacing) + 1_US; }
	GLsizei num_vertices() const { return (num_rows() + num_cols()) * 2; }

	void set_color(ColorFrame color);
};

struct Easel
{
	constexpr static float BACKGROUND_TSLOT = -1.0f;
	constexpr static float CHECKERBOARD_TSLOT = 0.0f;
	constexpr static float CANVAS_SPRITE_TSLOT = 1.0f;

	Window* window;
	Canvas canvas;
	GLuint canvas_sprite_VAO = 0, canvas_sprite_VB = 0, canvas_sprite_IB = 0;
	GLuint checkerboard_VAO = 0, checkerboard_VB = 0, checkerboard_IB = 0; // TODO combine along with background into one VAO/VB/IB.
	Sprite background;
	GLuint background_VAO = 0, background_VB = 0, background_IB = 0;
	Shader sprite_shader;

	bool canvas_visible = false;
	
	Gridlines minor_gridlines;
	Gridlines major_gridlines;

	bool minor_gridlines_visible = false;
	bool major_gridlines_visible = false;
	
	// View
	glm::mat3 projection{};
	Transform view;
private:
	Scale app_scale;
public:
	ClippingRect clip{};

	Easel(Window* window);
	Easel(const Easel&) = delete;
	Easel(Easel&&) noexcept = delete;
	~Easel();

	void set_projection(float width, float height);
	void set_projection();

	void render() const;

	void subsend_background_vao() const;
	void subsend_checkerboard_vao() const;
	void subsend_canvas_sprite_vao() const;
	void send_minor_gridlines_vao() const;
	void send_major_gridlines_vao() const;

	void send_view();
	
	void sync_canvas_transform();
	void sync_canvas_transform_p();
	void sync_canvas_transform_rs();

	void set_canvas_image(ImageHandle img);

	glm::vec2 to_world_coordinates(const glm::vec2& screen_coordinates) const;
	glm::vec2 to_screen_coordinates(const glm::vec2& world_coordinates) const;

	void set_app_scale(float x = 1.0f, float y = 1.0f);
	glm::vec2 get_app_scale() const;
	
	bool cursor_in_clipping() const { return clip.contains_point(window->cursor_pos()); }
	float get_app_width() const { return clip.screen_w * app_scale.x; }
	float get_app_height() const { return clip.screen_h * app_scale.y; }
	glm::vec2 get_app_cursor_pos() const { return window->cursor_pos() * app_scale; }
};
