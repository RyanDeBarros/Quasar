#pragma once

#include "user/Platform.h"
#include "FlatSprite.h"

struct Gridlines
{
	unsigned short width = 0, height = 0;
	GLuint vao = 0, vb = 0;
	Shader shader;
	GLfloat* varr = nullptr;
	float line_spacing = 1.0f;
	float line_width = 1.0f;
	float self_intersection_threshold = 1.0f;

	GLint* arrays_firsts = nullptr;
	GLsizei* arrays_counts = nullptr;

private:
	mutable bool _visible = true;
public:

	Gridlines();
	Gridlines(const Gridlines&) = delete;
	Gridlines(Gridlines&&) noexcept = delete;
	~Gridlines();

	void resize_grid(Scale scale);
	void update_scale(Scale scale) const;
	void draw() const;

	unsigned short num_cols() const;
	unsigned short num_rows() const;
	GLsizei num_quads() const { return num_rows() + num_cols(); }
	GLsizei num_vertices() const { return num_quads() * 4; }

	void set_color(ColorFrame color);
};

struct Canvas
{
	Image* image = nullptr;
	SharedFlatSprite sprite;
	SharedFlatSprite checkerboard;
	RGBA checker1, checker2;
private:
	float checker_size_inv = 1.0f / 16.0f; // LATER settings
public:
	unsigned short get_checker_size() const { return static_cast<unsigned short>(roundf(1.0f / checker_size_inv)); }
	void set_checker_size(unsigned short checker_size) { checker_size_inv = 1.0f / checker_size; }

	void set_image(ImageHandle img);

	FlatTransform& transform() { return sprite.transform; }
	const FlatTransform& transform() const { return sprite.transform; }
	Position& position() { return sprite.transform.position; }
	const Position& position() const { return sprite.transform.position; }
	Scale& scale() { return sprite.transform.scale; }
	const Scale& scale() const { return sprite.transform.scale; }

	void sync_transform();

	void create_checkerboard_image();
	void sync_checkerboard_colors() const;
	void sync_checkerboard_texture() const;
	void set_checkerboard_uv_size(float width, float height) const;
};

struct Easel
{
	constexpr static float BACKGROUND_TSLOT = -1.0f;
	constexpr static float CHECKERBOARD_TSLOT = 0.0f;
	constexpr static float CANVAS_SPRITE_TSLOT = 1.0f;

	Window* window;
	GLfloat* varr = nullptr;
	Canvas canvas;
	GLuint vao = 0, vb = 0, ib = 0;
	SharedFlatSprite background;
	Shader sprite_shader;

	bool canvas_visible = false;
	
	Gridlines minor_gridlines;
	Gridlines major_gridlines;

private:
	bool minor_gridlines_visible = false;
	bool _buffer_minor_gridlines_send_flat_transform = false;
	bool _buffer_minor_gridlines_sync_with_image = false;
	bool major_gridlines_visible = false;
	bool _buffer_major_gridlines_send_flat_transform = false;
	bool _buffer_major_gridlines_sync_with_image = false;
public:

	// View
	glm::mat3 projection{};
	FlatTransform view{};
	float view_scale = 1.0f;
private:
	float app_scale;
public:
	ClippingRect clip;

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
	void send_gridlines_vao(const Gridlines& gridlines) const;
	
	void send_view();
	glm::mat3 vp_matrix() const;
	
	void sync_canvas_transform();
	
	void resize_gridlines(Gridlines& gridlines) const;
	void update_gridlines_scale(const Gridlines& gridlines) const;
	void gridlines_send_flat_transform(Gridlines& gridlines) const;
	void gridlines_sync_with_image(Gridlines& gridlines) const;
	
	void set_canvas_image(ImageHandle img);

	bool minor_gridlines_are_visible() const { return minor_gridlines_visible; }
	void set_minor_gridlines_visibility(bool visible);
	bool major_gridlines_are_visible() const { return major_gridlines_visible; }
	void set_major_gridlines_visibility(bool visible);

	glm::vec2 to_world_coordinates(const glm::vec2& screen_coordinates) const;
	glm::vec2 to_screen_coordinates(const glm::vec2& world_coordinates) const;

	void set_app_scale(float sc = 1.0f);
	float get_app_scale() const;
	
	bool cursor_in_clipping() const { return clip.contains_point(window->cursor_pos()); }
	float get_app_width() const { return clip.screen_w * app_scale; }
	float get_app_height() const { return clip.screen_h * app_scale; }
	glm::vec2 get_app_cursor_pos() const { return window->cursor_pos() * app_scale; }
};
