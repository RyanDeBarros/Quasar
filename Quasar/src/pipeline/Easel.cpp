#include "Easel.h"

#include "variety/GLutility.h"
#include "user/Machine.h"

Checkerboard::Checkerboard(RGBA c1, RGBA c2)
	: c1(c1), c2(c2)
{
}

void Checkerboard::create_image()
{
	Image img;
	img.width = 2;
	img.height = 2;
	img.chpp = 4;
	img.pixels = new Image::Byte[img.area()];
	img.gen_texture();
	set_image(Images.add(std::move(img)));
	sync_colors();
	set_uv_size(0, 0);
}

void Checkerboard::sync_colors() const
{
	Image* img = Images.get(image);
	for (size_t i = 0; i < 2; ++i)
	{
		img->pixels[0 + 12 * i] = c1.rgb.r;
		img->pixels[1 + 12 * i] = c1.rgb.g;
		img->pixels[2 + 12 * i] = c1.rgb.b;
		img->pixels[3 + 12 * i] = c1.alpha;
	}
	for (size_t i = 0; i < 2; ++i)
	{
		img->pixels[4 + 4 * i] = c2.rgb.r;
		img->pixels[5 + 4 * i] = c2.rgb.g;
		img->pixels[6 + 4 * i] = c2.rgb.b;
		img->pixels[7 + 4 * i] = c2.alpha;
	}
	sync_texture();
}

void Checkerboard::sync_texture() const
{
	Image* img = Images.get(image);
	img->resend_texture();
	TextureParams tparams;
	tparams.wrap_s = TextureWrap::Repeat;
	tparams.wrap_t = TextureWrap::Repeat;
	img->update_texture_params(tparams);
}

void Checkerboard::set_uv_size(float width, float height) const
{
	set_uvs(Bounds{ 0.0f, width, 0.0f, height });
	sync_texture();
}

Canvas::Canvas(RGBA c1, RGBA c2)
	: checkerboard(c1, c2)
{
}

void Canvas::set_image(ImageHandle img)
{
	sprite.set_image(img);
	image = Images.get(img);
	if (image)
	{
		checkerboard.set_uv_size(0.5f * image->width / checker_size, 0.5f * image->height / checker_size);
		checkerboard.set_modulation(ColorFrame());
	}
	else
	{
		checkerboard.set_modulation(ColorFrame(0));
	}
}

void Canvas::sync_transform()
{
	sprite.sync_transform();
	checkerboard.transform = sprite.transform;
	if (image)
		checkerboard.transform.scale *= glm::vec2{ image->width * 0.5f, image->height * 0.5f };
	checkerboard.sync_transform();
}

Gridlines::Gridlines()
	: shader("res/gridlines.vert", "res/gridlines.frag", { 2 }, { "u_VP", "u_FlatTransform", "u_Color" })
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

void Gridlines::resize_grid(const Scale& scale)
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

void Gridlines::update_scale(const Scale& scale) const
{
	if (!varr) return;
	GLfloat* setter = varr;

#pragma warning(push)
#pragma warning(disable : 6386)
	float lwx = 0.5f * line_width / scale.x;
	float lwy = 0.5f * line_width / scale.y;
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
	bind_shader(shader.rid);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, arrays_firsts, arrays_counts, num_quads()));
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

// LATER when combining vertex buffers, create specialized shader for easel drawing. In it, no rotation is necessary, and scale is 1-dimensional.
// No transfom, and no packed(). Maybe not even position/scale. Just vertex pos.
Easel::Easel(Window* w)
	: window(w), canvas(RGBA(HSV(0.5f, 0.2f, 0.2f).to_rgb(), 0.5f), RGBA(HSV(0.5f, 0.3f, 0.3f).to_rgb(), 0.5f)),
	sprite_shader("res/flatsprite.vert", "res/flatsprite.frag", { 1, 2, 2, 4, 4 }, { "u_VP" }), clip(0, 0, window->width(), window->height())
{
	varr = new GLfloat[3 * SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE];
	background.varr = varr;
	canvas.checkerboard.varr = background.varr + SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE;
	canvas.sprite.varr = canvas.checkerboard.varr + SharedFlatSprite::NUM_VERTICES * SharedFlatSprite::STRIDE;
	background.initialize_varr();
	canvas.checkerboard.initialize_varr();
	canvas.sprite.initialize_varr();
	canvas.checkerboard.create_image();
	canvas.set_image(ImageHandle(0));

	gen_dynamic_vao(background_VAO, background_VB, background_IB, 4, sprite_shader.stride, 6, background.varr, FlatSprite::IARR, sprite_shader.attributes);
	gen_dynamic_vao(canvas_sprite_VAO, canvas_sprite_VB, canvas_sprite_IB, 4, sprite_shader.stride, 6, canvas.sprite.varr, FlatSprite::IARR, sprite_shader.attributes);
	gen_dynamic_vao(checkerboard_VAO, checkerboard_VB, checkerboard_IB, 4, sprite_shader.stride, 6, canvas.checkerboard.varr, FlatSprite::IARR, sprite_shader.attributes);

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
	delete_vao_buffers(canvas_sprite_VAO, canvas_sprite_VB, canvas_sprite_IB);
	delete_vao_buffers(checkerboard_VAO, checkerboard_VB, checkerboard_IB);
	delete_vao_buffers(background_VAO, background_VB, background_IB);
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
	bind_vao_buffers(background_VAO, background_VB, background_IB);
	QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
	// canvas
	if (canvas_visible)
	{
		// checkerboard
		Image* img = Images.get(canvas.checkerboard.image); // LATER honestly, since there aren't many images/shaders, not much point in registries. This isn't a game engine.
		bind_texture(img->tid, GLuint(CHECKERBOARD_TSLOT));
		bind_vao_buffers(checkerboard_VAO, checkerboard_VB, checkerboard_IB);
		QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
		// canvas sprite
		if (canvas.sprite.image)
		{
			img = Images.get(canvas.sprite.image);
			bind_texture(img->tid, GLuint(CANVAS_SPRITE_TSLOT));
			bind_vao_buffers(canvas_sprite_VAO, canvas_sprite_VB, canvas_sprite_IB);
			QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
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

// LATER When eventually combining vertex buffers, make use of second parameter in glBufferSubData to target the correct subbuffer.
void Easel::subsend_background_vao() const
{
	bind_vao_buffers(background_VAO, background_VB, background_IB);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), background.varr)); // TODO only buffersubdata when modifying varr, not every draw call.
	unbind_vao_buffers();
}

void Easel::subsend_checkerboard_vao() const
{
	bind_vao_buffers(checkerboard_VAO, checkerboard_VB, checkerboard_IB);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), canvas.checkerboard.varr));
	unbind_vao_buffers();
}

void Easel::subsend_canvas_sprite_vao() const
{
	bind_vao_buffers(canvas_sprite_VAO, canvas_sprite_VB, canvas_sprite_IB);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), canvas.sprite.varr));
	unbind_vao_buffers();
}

void Easel::send_minor_gridlines_vao() const
{
	bind_vao_buffers(minor_gridlines.vao, minor_gridlines.vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, minor_gridlines.num_vertices() * minor_gridlines.shader.stride * sizeof(GLfloat), minor_gridlines.varr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void Easel::send_major_gridlines_vao() const
{
	bind_vao_buffers(major_gridlines.vao, major_gridlines.vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, major_gridlines.num_vertices() * major_gridlines.shader.stride * sizeof(GLfloat), major_gridlines.varr, GL_DYNAMIC_DRAW));
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

// TODO buffer updates to gridlines when not they aren't visible
void Easel::sync_canvas_transform()
{
	canvas.sync_transform();
	subsend_canvas_sprite_vao();
	subsend_checkerboard_vao();
	bind_shader(minor_gridlines.shader.rid);
	QUASAR_GL(glUniform4fv(minor_gridlines.shader.uniform_locations["u_FlatTransform"], 1, &canvas.transform().packed()[0]));
	update_minor_gridlines();
	bind_shader(major_gridlines.shader.rid);
	QUASAR_GL(glUniform4fv(major_gridlines.shader.uniform_locations["u_FlatTransform"], 1, &canvas.transform().packed()[0]));
	update_major_gridlines();
	unbind_shader();
}

// TODO set line width in some way relative to zoom. As you zoom out, line width should approach 1.0 and at some limit drop straight to 0.0.
void Easel::resize_minor_gridlines()
{
	minor_gridlines.resize_grid({ canvas.sprite.transform.scale.x, canvas.sprite.transform.scale.y });
	send_minor_gridlines_vao();
}

void Easel::update_minor_gridlines() const
{
	minor_gridlines.update_scale({ canvas.sprite.transform.scale.x, canvas.sprite.transform.scale.y });
	send_minor_gridlines_vao();
}

void Easel::resize_major_gridlines()
{
	major_gridlines.resize_grid({ canvas.sprite.transform.scale.x, canvas.sprite.transform.scale.y });
	send_major_gridlines_vao();
}

void Easel::update_major_gridlines() const
{
	major_gridlines.update_scale({ canvas.sprite.transform.scale.x, canvas.sprite.transform.scale.y });
	send_major_gridlines_vao();
}

void Easel::set_canvas_image(ImageHandle img)
{
	canvas.set_image(img);
	canvas_visible = true;
	minor_gridlines.width = canvas.image->width;
	minor_gridlines.height = canvas.image->height;
	resize_minor_gridlines();
	send_minor_gridlines_vao();
	major_gridlines.width = canvas.image->width;
	major_gridlines.height = canvas.image->height;
	resize_major_gridlines();
	send_major_gridlines_vao();
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
