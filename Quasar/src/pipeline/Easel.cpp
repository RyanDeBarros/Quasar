#include "Easel.h"

#include "variety/GLutility.h"
#include "user/Machine.h"

Gridlines::Gridlines()
	: shader(FileSystem::resources_path("gridlines.vert"), FileSystem::resources_path("gridlines.frag"),
		{ 2 }, { "u_VP", "u_FlatTransform", "u_Color" })
{
	gen_dynamic_vao(vao, vb, 0, shader.stride, varr, shader.attributes);
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
	if (2.0f * lwx >= scale.x * line_spacing - self_intersection_threshold || 2.0f * lwy >= scale.y * line_spacing - self_intersection_threshold)
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
		float delta = i * line_spacing;
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
		float delta = i * line_spacing;
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
		bind_shader(shader.rid);
		bind_vao_buffers(vao, vb);
		QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, arrays_firsts, arrays_counts, num_quads()));
	}
}

unsigned short Gridlines::num_cols() const
{
	return unsigned short(std::ceil(width / line_spacing)) + 1_US;
}

unsigned short Gridlines::num_rows() const
{
	return unsigned short(std::ceil(height / line_spacing)) + 1_US;
}

void Gridlines::set_color(ColorFrame color)
{
	bind_shader(shader.rid);
	QUASAR_GL(glUniform4fv(shader.uniform_locations["u_Color"], 1, &color.rgba_as_vec()[0]));
	unbind_shader();
}

void Canvas::create_checkerboard_image()
{
	Image img;
	img.buf.width = 2;
	img.buf.height = 2;
	img.buf.chpp = 4;
	img.buf.pxnew();
	img.gen_texture();
	checkerboard.set_image(Images.add(std::move(img)));
	set_checkerboard_uv_size(0, 0);
}

void Canvas::sync_checkerboard_colors() const
{
	Image* img = Images.get(checkerboard.image);
	for (size_t i = 0; i < 2; ++i)
	{
		img->buf.pixels[0 + 12 * i] = checker1.rgb.r;
		img->buf.pixels[1 + 12 * i] = checker1.rgb.g;
		img->buf.pixels[2 + 12 * i] = checker1.rgb.b;
		img->buf.pixels[3 + 12 * i] = checker1.alpha;
	}
	for (size_t i = 0; i < 2; ++i)
	{
		img->buf.pixels[4 + 4 * i] = checker2.rgb.r;
		img->buf.pixels[5 + 4 * i] = checker2.rgb.g;
		img->buf.pixels[6 + 4 * i] = checker2.rgb.b;
		img->buf.pixels[7 + 4 * i] = checker2.alpha;
	}
	sync_checkerboard_texture();
}

void Canvas::sync_checkerboard_texture() const
{
	Image* img = Images.get(checkerboard.image);
	img->resend_texture();
	TextureParams tparams;
	tparams.wrap_s = TextureWrap::Repeat;
	tparams.wrap_t = TextureWrap::Repeat;
	img->update_texture_params(tparams);
}

void Canvas::set_checkerboard_uv_size(float width, float height) const
{
	checkerboard.set_uvs(Bounds{ 0.0f, width, 0.0f, height });
}

void Canvas::set_image(ImageHandle img)
{
	sprite.set_image(img);
	image = Images.get(img);
	if (image)
	{
		set_checkerboard_uv_size(0.5f * image->buf.width * checker_size_inv.x, 0.5f * image->buf.height * checker_size_inv.y);
		checkerboard.set_modulation(ColorFrame());
	}
	else
		checkerboard.set_modulation(ColorFrame(0));
}

void Canvas::sync_transform()
{
	sprite.sync_transform();
	checkerboard.transform = sprite.transform;
	if (image)
		checkerboard.transform.scale *= glm::vec2{ image->buf.width * 0.5f, image->buf.height * 0.5f };
	checkerboard.sync_transform();
}

Easel::Easel(Window* w)
	: window(w), sprite_shader(FileSystem::resources_path("flatsprite.vert"), FileSystem::resources_path("flatsprite.frag"),
		{ 1, 2, 2, 4, 4 }, { "u_VP" }), clip(0, 0, window->width(), window->height())
{
	varr = new GLfloat[3 * SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE];
	background.varr = varr;
	canvas.checkerboard.varr = background.varr + SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE;
	canvas.sprite.varr = canvas.checkerboard.varr + SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE;
	background.initialize_varr();
	canvas.checkerboard.initialize_varr();
	canvas.sprite.initialize_varr();
	canvas.create_checkerboard_image();
	canvas.set_image(ImageHandle(0));

	canvas.checker1 = Machine.preferences.checker1;
	canvas.checker2 = Machine.preferences.checker2;
	canvas.set_checker_size(Machine.preferences.checker_size);
	canvas.sync_checkerboard_colors();

	GLuint IARR[3*6]{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8
	};
	gen_dynamic_vao(vao, vb, ib, size_t(3) * SharedFlatSprite::NUM_VERTICES, sprite_shader.stride, sizeof(IARR) / sizeof(*IARR), varr, IARR, sprite_shader.attributes);

	set_projection();
	send_view();
	clip.window_size_to_bounds = [](int width, int height) -> glm::ivec4 { return { 0, 0, width, height }; };

	background.sync_texture_slot(BACKGROUND_TSLOT);
	canvas.checkerboard.sync_texture_slot(CHECKERBOARD_TSLOT);
	canvas.sprite.sync_texture_slot(CANVAS_SPRITE_TSLOT);

	background.set_image(ImageHandle(0), 1, 1);
	background.set_modulation(ColorFrame(HSV(0.5f, 0.15f, 0.15f), 0.5f));
	background.transform.scale = { float(window->width()), float(window->height()) };
	background.sync_transform();
	subsend_background_vao();
	window->clbk_window_size.push_back([this](const Callback::WindowSize& ws) {
		QUASAR_GL(glViewport(0, 0, ws.width, ws.height));
		clip.update_window_size(ws.width, ws.height);
		set_projection(float(ws.width), float(ws.height));
		send_view();
		Machine.on_render();
		});

	major_gridlines.line_spacing = 16.0f;
}

Easel::~Easel()
{
	delete_vao_buffers(vao, vb, ib);
	delete[] varr;
}

void Easel::set_projection(float width, float height)
{
	projection = glm::ortho<float>(0.0f, width * app_scale, 0.0f, height * app_scale);
}

void Easel::set_projection()
{
	projection = glm::ortho<float>(0.0f, window->width() * app_scale, 0.0f, window->height() * app_scale);
}

void Easel::render() const
{
	// bind
	bind_shader(sprite_shader.rid);
	QUASAR_GL(glScissor(clip.x, clip.y, clip.screen_w, clip.screen_h));
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
		Image* img = Images.get(canvas.checkerboard.image); // LATER honestly, since there aren't many images/shaders, not much point in registries. This isn't a game engine.
		bind_texture(img->tid, GLuint(CHECKERBOARD_TSLOT));
		if (!canvas.sprite.image)
		{
			QUASAR_GL(glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0));
		}
		else
		{
			// canvas sprite
			img = Images.get(canvas.sprite.image);
			bind_texture(img->tid, GLuint(CANVAS_SPRITE_TSLOT));
			QUASAR_GL(glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0));
		}
		// minor gridlines
		if (minor_gridlines_visible)
			minor_gridlines.draw();
		// major gridlines
		if (major_gridlines_visible)
			major_gridlines.draw();
	}
	// unbind
	unbind_vao_buffers();
	unbind_shader();
	QUASAR_GL(glScissor(0, 0, window->width(), window->height()));
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
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE * sizeof(GLfloat), 4 * sprite_shader.stride * sizeof(GLfloat), canvas.checkerboard.varr));
	unbind_vao_buffers();
}

void Easel::subsend_canvas_sprite_vao() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 2 * SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE * sizeof(GLfloat), 4 * sprite_shader.stride * sizeof(GLfloat), canvas.sprite.varr));
	unbind_vao_buffers();
}

void Easel::send_gridlines_vao(const Gridlines& gridlines) const
{
	bind_vao_buffers(gridlines.vao, gridlines.vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, gridlines.num_vertices() * gridlines.shader.stride * sizeof(GLfloat), gridlines.varr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void Easel::send_view()
{
	glm::mat3 cameraVP = vp_matrix();
	bind_shader(sprite_shader.rid);
	QUASAR_GL(glUniformMatrix3fv(sprite_shader.uniform_locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
	bind_shader(minor_gridlines.shader.rid);
	QUASAR_GL(glUniformMatrix3fv(minor_gridlines.shader.uniform_locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
	bind_shader(major_gridlines.shader.rid);
	QUASAR_GL(glUniformMatrix3fv(major_gridlines.shader.uniform_locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
	unbind_shader();
}

glm::mat3 Easel::vp_matrix() const
{
	return projection * view.camera();
}

void Easel::sync_canvas_transform()
{
	canvas.sync_transform();
	subsend_canvas_sprite_vao();
	subsend_checkerboard_vao();
	if (minor_gridlines_visible)
		gridlines_send_flat_transform(minor_gridlines);
	else
		_buffer_minor_gridlines_send_flat_transform = true;
	if (major_gridlines_visible)
		gridlines_send_flat_transform(major_gridlines);
	else
		_buffer_major_gridlines_send_flat_transform = true;
	unbind_shader();
}

void Easel::gridlines_send_flat_transform(Gridlines& gridlines) const
{
	bind_shader(gridlines.shader.rid);
	QUASAR_GL(glUniform4fv(gridlines.shader.uniform_locations["u_FlatTransform"], 1, &canvas.transform().packed()[0]));
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
	gridlines.width = canvas.image->buf.width;
	gridlines.height = canvas.image->buf.height;
	resize_gridlines(gridlines);
	send_gridlines_vao(gridlines);
}

void Easel::set_canvas_image(ImageHandle img)
{
	canvas.set_image(img);
	canvas_visible = true;
	if (minor_gridlines_visible)
		gridlines_sync_with_image(minor_gridlines);
	else
		_buffer_minor_gridlines_sync_with_image = true;
	if (major_gridlines_visible)
		gridlines_sync_with_image(major_gridlines);
	else
		_buffer_major_gridlines_sync_with_image = true;
}

void Easel::set_minor_gridlines_visibility(bool visible)
{
	if (!minor_gridlines_visible && visible)
	{
		if (_buffer_minor_gridlines_send_flat_transform)
			gridlines_send_flat_transform(minor_gridlines);
		if (_buffer_minor_gridlines_sync_with_image)
			gridlines_sync_with_image(minor_gridlines);
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
			gridlines_send_flat_transform(major_gridlines);
		if (_buffer_major_gridlines_sync_with_image)
			gridlines_sync_with_image(major_gridlines);
	}
	else if (major_gridlines_visible && !visible)
	{
		_buffer_major_gridlines_send_flat_transform = false;
		_buffer_major_gridlines_sync_with_image = false;
	}
	major_gridlines_visible = visible;
}

glm::vec2 Easel::to_world_coordinates(const glm::vec2& screen_coordinates) const
{
	glm::vec3 ndc{};
	ndc.x = 1.0f - 2.0f * (screen_coordinates.x / window->width());
	ndc.y = 1.0f - 2.0f * (screen_coordinates.y / window->height());
	ndc.z = 1.0f;

	glm::mat3 invVP = glm::inverse(vp_matrix());
	glm::vec3 world_pos = invVP * ndc;

	if (world_pos.z != 0.0f)
		world_pos / world_pos.z;

	return glm::vec2{ -world_pos.x, -world_pos.y };
}

glm::vec2 Easel::to_screen_coordinates(const glm::vec2& world_coordinates) const
{
	glm::vec3 world_pos{ world_coordinates.x, -world_coordinates.y, 1.0f };
	glm::vec3 clip_space_pos = vp_matrix() * world_pos;
	glm::vec2 screen_coo{};
	screen_coo.x = (1.0f + clip_space_pos.x) * 0.5f * window->width();
	screen_coo.y = (1.0f + clip_space_pos.y) * 0.5f * window->height();
	return screen_coo;
}

void Easel::set_app_scale(float sc)
{
	app_scale = 1.0f / sc;
	set_projection();
	send_view();
	// LATER scale cursor? Have different discrete cursor sizes (only works for custom cursors).
}

float Easel::get_app_scale() const
{
	return 1.0f / app_scale;
}
