#pragma once

#include "Panel.h"
#include "../FlatSprite.h"
#include "../Shader.h"

struct Gridlines
{
	unsigned short width = 0, height = 0;
	GLuint vao = 0, vb = 0;
	Shader shader;
	GLfloat* varr = nullptr;
	glm::vec2 line_spacing = { 1.0f, 1.0f };
	float line_width = 1.0f; // SETTINGS
	float self_intersection_threshold = 1.0f; // SETTINGS

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

	void set_color(ColorFrame color) const;
};

struct Canvas
{
	SharedFlatSprite sprite;
	SharedFlatSprite checkerboard;
	RGBA checker1, checker2;

	Gridlines minor_gridlines;
	Gridlines major_gridlines;
private:
	glm::vec2 checker_size_inv = glm::vec2(1.0f / 16.0f);
public:
	glm::ivec2 get_checker_size() const { return { roundf(1.0f / checker_size_inv.x), roundf(1.0f / checker_size_inv.y) }; }
	void set_checker_size(glm::ivec2 checker_size);

	void set_image(const std::shared_ptr<Image>& img);
	void set_image(std::shared_ptr<Image>&& img);

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

struct Easel : public Panel
{
	constexpr static float CHECKERBOARD_TSLOT = 0.0f;
	constexpr static float CANVAS_SPRITE_TSLOT = 1.0f;

	GLfloat* varr = nullptr;
	Canvas canvas;
	GLuint vao = 0, vb = 0, ib = 0;
	SharedFlatSprite background;
	Shader sprite_shader;

	bool canvas_visible = false;

private:
	bool minor_gridlines_visible = false;
	bool _buffer_minor_gridlines_send_flat_transform = false;
	bool _buffer_minor_gridlines_sync_with_image = false;
	bool major_gridlines_visible = false;
	bool _buffer_major_gridlines_send_flat_transform = false;
	bool _buffer_major_gridlines_sync_with_image = false;
public:

	Easel();
	Easel(const Easel&) = delete;
	Easel(Easel&&) noexcept = delete;
	~Easel();

	void draw() override;

	void subsend_background_vao() const;
	void subsend_checkerboard_vao() const;
	void subsend_canvas_sprite_vao() const;
	void send_gridlines_vao(const Gridlines& gridlines) const;
	
	void _send_view() override;
	
	void sync_canvas_transform();
	
	void resize_gridlines(Gridlines& gridlines) const;
	void update_gridlines_scale(const Gridlines& gridlines) const;
	void gridlines_send_flat_transform(Gridlines& gridlines) const;
	void gridlines_sync_with_image(Gridlines& gridlines) const;
	
	void set_canvas_image(const std::shared_ptr<Image>& image);
	void set_canvas_image(std::shared_ptr<Image>&& image);
	void update_canvas_image() { set_canvas_image(canvas.sprite.image); sync_canvas_transform(); }
	Image* canvas_image() const { return canvas.sprite.image.get(); }
	std::shared_ptr<Image> canvas_image_ref() const { return canvas.sprite.image; }

	bool minor_gridlines_are_visible() const { return minor_gridlines_visible; }
	void set_minor_gridlines_visibility(bool visible);
	bool major_gridlines_are_visible() const { return major_gridlines_visible; }
	void set_major_gridlines_visibility(bool visible);
};
