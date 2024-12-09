#include "Gridlines.h"

#include "Uniforms.h"

// LATER use geometry shader for simplicity
Gridlines::Gridlines()
	: shader(FileSystem::shader_path("gridlines.vert"), FileSystem::shader_path("gridlines.frag"))
{
	initialize_dynamic_vao(vao, vb, 0, shader.stride, varr, shader.attributes);
}

Gridlines::~Gridlines()
{
	delete_vao_buffers(vao, vb);
	delete[] varr;
	delete[] arrays_firsts;
	delete[] arrays_counts;
}

void Gridlines::resize_grid(Scale scale)
{
	delete[] varr;
	varr = new GLfloat[num_vertices() * shader.stride];
	update_scale(scale);
#pragma warning(push)
#pragma warning(disable : 6386)
	delete[] arrays_firsts;
	delete[] arrays_counts;
	arrays_firsts = new GLint[num_quads()];
	arrays_counts = new GLsizei[num_quads()];
	for (unsigned short i = 0; i < num_quads(); ++i)
	{
		arrays_firsts[i] = 4 * i;
		arrays_counts[i] = 4;
	}
#pragma warning(pop)
}

void Gridlines::update_scale(Scale scale) const
{
	if (!varr) return;
	GLfloat* setter = varr;

#pragma warning(push)
#pragma warning(disable : 6386)
	float lwx = 0.5f * line_width;
	if (scale.x > 1.0f)
		lwx /= scale.x;
	float lwy = 0.5f * line_width;
	if (scale.y > 1.0f)
		lwy /= scale.y;
	if (2.0f * lwx >= scale.x * line_spacing.x - self_intersection_threshold || 2.0f * lwy >= scale.y * line_spacing.y - self_intersection_threshold)
	{
		_nonobstructing = false;
		return;
	}
	_nonobstructing = true;

	float x1 = -width * 0.5f - lwx;
	float y1 = -height * 0.5f - lwy;
	float x2 = -width * 0.5f + lwx;
	float y2 = height * 0.5f + lwy;

	for (unsigned short i = 0; i < num_cols() - 1; ++i)
	{
		float delta = i * line_spacing.x;
		setter[0] = x1 + delta;
		setter[1] = y1;
		setter += shader.stride;
		setter[0] = x2 + delta;
		setter[1] = y1;
		setter += shader.stride;
		setter[0] = x1 + delta;
		setter[1] = y2;
		setter += shader.stride;
		setter[0] = x2 + delta;
		setter[1] = y2;
		setter += shader.stride;
	}

	x1 = width * 0.5f - lwx;
	x2 = width * 0.5f + lwx;
	setter[0] = x1;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x1;
	setter[1] = y2;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y2;
	setter += shader.stride;

	x1 = -width * 0.5f - lwx;
	y1 = -height * 0.5f - lwy;
	x2 = width * 0.5f + lwx;
	y2 = -height * 0.5f + lwy;
	for (unsigned short i = 0; i < num_rows() - 1; ++i)
	{
		float delta = i * line_spacing.y;
		setter[0] = x1;
		setter[1] = y1 + delta;
		setter += shader.stride;
		setter[0] = x1;
		setter[1] = y2 + delta;
		setter += shader.stride;
		setter[0] = x2;
		setter[1] = y1 + delta;
		setter += shader.stride;
		setter[0] = x2;
		setter[1] = y2 + delta;
		setter += shader.stride;
	}

	y1 = height * 0.5f - lwy;
	y2 = height * 0.5f + lwy;
	setter[0] = x1;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x1;
	setter[1] = y2;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y1;
	setter += shader.stride;
	setter[0] = x2;
	setter[1] = y2;
	setter += shader.stride;
#pragma warning(pop)
}

void Gridlines::draw() const
{
	if (_visible && _nonobstructing)
	{
		bind_shader(shader);
		bind_vao_buffers(vao, vb);
		QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, arrays_firsts, arrays_counts, num_quads()));
	}
}

unsigned short Gridlines::num_cols() const
{
	return unsigned short(std::ceil(width / line_spacing.x)) + 1;
}

unsigned short Gridlines::num_rows() const
{
	return unsigned short(std::ceil(height / line_spacing.y)) + 1;
}

void Gridlines::set_color(ColorFrame color) const
{
	Uniforms::send_4(shader, "u_Color", color.rgba().as_vec(), 0, true);
}

void Gridlines::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, num_vertices() * shader.stride * sizeof(GLfloat), varr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void Gridlines::send_flat_transform(FlatTransform canvas_transform) const
{
	if (_visible)
	{
		Uniforms::send_4(shader, "u_FlatTransform", canvas_transform.packed());
		update_scale(canvas_transform.scale);
		send_buffer();
		_send_flat_transform = false;
	}
	else
		_send_flat_transform = true;
}

void Gridlines::sync_with_image(int w, int h, Scale canvas_scale)
{
	if (_visible)
	{
		width = w;
		height = h;
		resize_grid(canvas_scale);
		send_buffer();
		_sync_with_image = false;
	}
	else
		_sync_with_image = true;
}

void Gridlines::set_visible(bool visible, FlatTransform transform, int w, int h)
{
	if (!_visible && visible)
	{
		_visible = true;
		if (_send_flat_transform)
			send_flat_transform(transform);
		if (_sync_with_image)
			sync_with_image(w, h, transform.scale);
	}
	else if (_visible && !visible)
	{
		_visible = false;
		_send_flat_transform = false;
		_sync_with_image = false;
	}
}
