#pragma once

#include "variety/GLutility.h"
#include "variety/Geometry.h"
#include "Shader.h"
#include "edit/color/Color.h"

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
	bool _visible = false;
	mutable bool _nonobstructing = true;
	mutable bool _send_flat_transform = false;
	mutable bool _sync_with_image = false;

public:
	Gridlines();
	Gridlines(const Gridlines&) = delete;
	Gridlines(Gridlines&&) noexcept = delete;
	~Gridlines();

	void resize_grid(Scale scale);
	void update_scale(Scale scale) const;
	void draw() const;

	bool visible() const { return _visible; }
	unsigned short num_cols() const;
	unsigned short num_rows() const;
	GLsizei num_quads() const { return num_rows() + num_cols(); }
	GLsizei num_vertices() const { return num_quads() * 4; }

	void set_color(ColorFrame color) const;

	void send_buffer() const;
	void send_flat_transform(FlatTransform canvas_transform) const;
	void sync_with_image(int w, int h, Scale canvas_scale);

	void set_visible(bool visible, FlatTransform transform, int w, int h);
};
