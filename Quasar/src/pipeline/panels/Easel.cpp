#include "Easel.h"

#include "variety/GLutility.h"
#include "user/Machine.h"
#include "../render/Uniforms.h"

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
		_visible = false;
		return;
	}
	else
		_visible = true;

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
	if (_visible)
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

void Canvas::create_checkerboard_image()
{
	Image* img = new Image();
	img->buf.width = 2;
	img->buf.height = 2;
	img->buf.chpp = 4;
	img->buf.pxnew();
	img->gen_texture();
	checkerboard.set_image(std::shared_ptr<Image>(img));
	set_checkerboard_uv_size(0, 0);
}

void Canvas::sync_checkerboard_colors() const
{
	if (checkerboard.image)
	{
		for (size_t i = 0; i < 2; ++i)
		{
			checkerboard.image->buf.pixels[0 + 12 * i] = checker1.get_pixel_r();
			checkerboard.image->buf.pixels[1 + 12 * i] = checker1.get_pixel_g();
			checkerboard.image->buf.pixels[2 + 12 * i] = checker1.get_pixel_b();
			checkerboard.image->buf.pixels[3 + 12 * i] = checker1.get_pixel_a();
		}
		for (size_t i = 0; i < 2; ++i)
		{
			checkerboard.image->buf.pixels[4 + 4 * i] = checker2.get_pixel_r();
			checkerboard.image->buf.pixels[5 + 4 * i] = checker2.get_pixel_g();
			checkerboard.image->buf.pixels[6 + 4 * i] = checker2.get_pixel_b();
			checkerboard.image->buf.pixels[7 + 4 * i] = checker2.get_pixel_a();
		}
	}
	sync_checkerboard_texture();
}

void Canvas::sync_checkerboard_texture() const
{
	if (checkerboard.image)
	{
		checkerboard.image->resend_texture();
		TextureParams tparams;
		tparams.wrap_s = TextureWrap::Repeat;
		tparams.wrap_t = TextureWrap::Repeat;
		checkerboard.image->update_texture_params(tparams);
	}
}

void Canvas::set_checkerboard_uv_size(float width, float height) const
{
	checkerboard.set_uvs(Bounds{ 0.0f, width, 0.0f, height });
}

void Canvas::set_checker_size(glm::ivec2 checker_size)
{
	checker_size_inv = { 1.0f / checker_size.x, 1.0f / checker_size.y };
	major_gridlines.line_spacing = checker_size;
}

void Canvas::set_image(const std::shared_ptr<Image>& img)
{
	sprite.set_image(img);
	if (sprite.image)
	{
		set_checkerboard_uv_size(0.5f * sprite.image->buf.width * checker_size_inv.x, 0.5f * sprite.image->buf.height * checker_size_inv.y);
		checkerboard.set_modulation(ColorFrame());
	}
	else
		checkerboard.set_modulation(ColorFrame(0));
}

void Canvas::set_image(std::shared_ptr<Image>&& img)
{
	sprite.set_image(std::move(img));
	if (sprite.image)
	{
		set_checkerboard_uv_size(0.5f * sprite.image->buf.width * checker_size_inv.x, 0.5f * sprite.image->buf.height * checker_size_inv.y);
		checkerboard.set_modulation(ColorFrame());
	}
	else
		checkerboard.set_modulation(ColorFrame(0));
}

void Canvas::sync_transform()
{
	sprite.sync_transform();
	checkerboard.transform = sprite.transform;
	if (sprite.image)
		checkerboard.transform.scale *= glm::vec2{ sprite.image->buf.width * 0.5f, sprite.image->buf.height * 0.5f };
	checkerboard.sync_transform();
}

Easel::Easel()
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", "32" } }) // TODO actually query the number of texture slots supported.
{
	static constexpr size_t num_quads = 3;

	varr = new GLfloat[num_quads * FlatSprite::NUM_VERTICES * FlatSprite::STRIDE];
	background.varr = varr;
	canvas.checkerboard.varr = background.varr + FlatSprite::NUM_VERTICES * FlatSprite::STRIDE;
	canvas.sprite.varr = canvas.checkerboard.varr + FlatSprite::NUM_VERTICES * FlatSprite::STRIDE;
	background.initialize_varr();
	canvas.checkerboard.initialize_varr();
	canvas.sprite.initialize_varr();
	canvas.create_checkerboard_image();
	canvas.set_image(nullptr);

	canvas.checker1 = Machine.preferences.checker1;
	canvas.checker2 = Machine.preferences.checker2;
	canvas.set_checker_size(Machine.preferences.checker_size);
	canvas.sync_checkerboard_colors();

	GLuint IARR[num_quads * 6]{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8
	};
	initialize_dynamic_vao(vao, vb, ib, num_quads * FlatSprite::NUM_VERTICES, sprite_shader.stride, sizeof(IARR) / sizeof(*IARR), varr, IARR, sprite_shader.attributes);

	background.sync_texture_slot(-1.0f);
	canvas.checkerboard.sync_texture_slot(CHECKERBOARD_TSLOT);
	canvas.sprite.sync_texture_slot(CANVAS_SPRITE_TSLOT);

	background.set_image(nullptr, 1, 1);
	background.set_modulation(ColorFrame(HSV(0.5f, 0.15f, 0.15f), 0.5f));
}

Easel::~Easel()
{
	delete_vao_buffers(vao, vb, ib);
	delete[] varr;
}

void Easel::draw()
{
	// bind
	bind_shader(sprite_shader);
	// background
	bind_vao_buffers(vao, vb, ib);
	// canvas
	if (!canvas_visible)
	{
		QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
	}
	else
	{
		// checkerboard
		bind_texture(canvas.checkerboard.image->tid, GLuint(CHECKERBOARD_TSLOT));
		if (!canvas.sprite.image)
		{
			QUASAR_GL(glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0));
		}
		else
		{
			// canvas sprite
			bind_texture(canvas.sprite.image->tid, GLuint(CANVAS_SPRITE_TSLOT));
			QUASAR_GL(glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0));
		}
		// minor gridlines
		if (minor_gridlines_visible)
			canvas.minor_gridlines.draw();
		// major gridlines
		if (major_gridlines_visible)
			canvas.major_gridlines.draw();
	}
	// unbind
	unbind_vao_buffers();
	unbind_shader();
}

void Easel::subsend_background_vao() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), background.varr));
	unbind_vao_buffers();
}

void Easel::subsend_checkerboard_vao() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, FlatSprite::NUM_VERTICES * FlatSprite::STRIDE * sizeof(GLfloat), 4 * sprite_shader.stride * sizeof(GLfloat), canvas.checkerboard.varr));
	unbind_vao_buffers();
}

void Easel::subsend_canvas_sprite_vao() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 2 * FlatSprite::NUM_VERTICES * FlatSprite::STRIDE * sizeof(GLfloat), 4 * sprite_shader.stride * sizeof(GLfloat), canvas.sprite.varr));
	unbind_vao_buffers();
}

void Easel::send_gridlines_vao(const Gridlines& gridlines) const
{
	bind_vao_buffers(gridlines.vao, gridlines.vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, gridlines.num_vertices() * gridlines.shader.stride * sizeof(GLfloat), gridlines.varr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void Easel::_send_view()
{
	background.transform.scale = get_app_size();
	background.sync_transform();
	subsend_background_vao();
	glm::mat3 cameraVP = vp_matrix();
	Uniforms::send_matrix3(sprite_shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(canvas.minor_gridlines.shader, "u_VP", cameraVP);
	Uniforms::send_matrix3(canvas.major_gridlines.shader, "u_VP", cameraVP);
	unbind_shader();
}

void Easel::sync_canvas_transform()
{
	canvas.sync_transform();
	subsend_canvas_sprite_vao();
	subsend_checkerboard_vao();
	if (minor_gridlines_visible)
		gridlines_send_flat_transform(canvas.minor_gridlines);
	else
		_buffer_minor_gridlines_send_flat_transform = true;
	if (major_gridlines_visible)
		gridlines_send_flat_transform(canvas.major_gridlines);
	else
		_buffer_major_gridlines_send_flat_transform = true;
	unbind_shader();
}

void Easel::gridlines_send_flat_transform(Gridlines& gridlines) const
{
	Uniforms::send_4(gridlines.shader, "u_FlatTransform", canvas.transform().packed());
	update_gridlines_scale(gridlines);
}

void Easel::resize_gridlines(Gridlines& gridlines) const
{
	gridlines.resize_grid(canvas.scale());
	send_gridlines_vao(gridlines);
}

void Easel::update_gridlines_scale(const Gridlines& gridlines) const
{
	gridlines.update_scale(canvas.scale());
	send_gridlines_vao(gridlines);
}

void Easel::gridlines_sync_with_image(Gridlines& gridlines) const
{
	gridlines.width = canvas.sprite.image->buf.width;
	gridlines.height = canvas.sprite.image->buf.height;
	resize_gridlines(gridlines);
	send_gridlines_vao(gridlines);
}

void Easel::set_canvas_image(const std::shared_ptr<Image>& img)
{
	canvas.set_image(img);
	canvas_visible = true;
	if (minor_gridlines_visible)
		gridlines_sync_with_image(canvas.minor_gridlines);
	else
		_buffer_minor_gridlines_sync_with_image = true;
	if (major_gridlines_visible)
		gridlines_sync_with_image(canvas.major_gridlines);
	else
		_buffer_major_gridlines_sync_with_image = true;
}

void Easel::set_canvas_image(std::shared_ptr<Image>&& img)
{
	canvas.set_image(std::move(img));
	canvas_visible = true;
	if (minor_gridlines_visible)
		gridlines_sync_with_image(canvas.minor_gridlines);
	else
		_buffer_minor_gridlines_sync_with_image = true;
	if (major_gridlines_visible)
		gridlines_sync_with_image(canvas.major_gridlines);
	else
		_buffer_major_gridlines_sync_with_image = true;
}

void Easel::set_minor_gridlines_visibility(bool visible)
{
	if (!minor_gridlines_visible && visible)
	{
		if (_buffer_minor_gridlines_send_flat_transform)
			gridlines_send_flat_transform(canvas.minor_gridlines);
		if (_buffer_minor_gridlines_sync_with_image)
			gridlines_sync_with_image(canvas.minor_gridlines);
	}
	else if (minor_gridlines_visible && !visible)
	{
		_buffer_minor_gridlines_send_flat_transform = false;
		_buffer_minor_gridlines_sync_with_image = false;
	}
	minor_gridlines_visible = visible;
}

void Easel::set_major_gridlines_visibility(bool visible)
{
	if (!major_gridlines_visible && visible)
	{
		if (_buffer_major_gridlines_send_flat_transform)
			gridlines_send_flat_transform(canvas.major_gridlines);
		if (_buffer_major_gridlines_sync_with_image)
			gridlines_sync_with_image(canvas.major_gridlines);
	}
	else if (major_gridlines_visible && !visible)
	{
		_buffer_major_gridlines_send_flat_transform = false;
		_buffer_major_gridlines_sync_with_image = false;
	}
	major_gridlines_visible = visible;
}
