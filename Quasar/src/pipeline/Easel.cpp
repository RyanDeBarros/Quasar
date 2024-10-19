#include "Easel.h"

#include "variety/GLutility.h"
#include "user/Machine.h"

Checkerboard::Checkerboard(RGBA c1, RGBA c2)
	: c1(c1), c2(c2)
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
	set_image(ImageHandle(0));
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

void Canvas::sync_transform_p()
{
	sprite.sync_transform_p();
	checkerboard.transform.position = sprite.transform.position;
	checkerboard.sync_transform_p();
}

void Canvas::sync_transform_rs()
{
	sprite.sync_transform_rs();
	checkerboard.transform.rotation = sprite.transform.rotation;
	if (image)
		checkerboard.transform.scale = sprite.transform.scale * glm::vec2{ image->width * 0.5f, image->height * 0.5f };
	checkerboard.sync_transform_rs();
}

Easel::Easel(Window* w)
	: window(w), canvas(RGBA(HSV(0.5f, 0.2f, 0.2f).to_rgb(), 0.5f), RGBA(HSV(0.5f, 0.3f, 0.3f).to_rgb(), 0.5f)),
	sprite_shader("res/sprite.vert", "res/sprite.frag", { 1, 2, 2, 2, 4, 4 }, { "u_VP" })
{
	gen_vao_dynamic_draw(&canvas_sprite_VAO, &canvas_sprite_VB, &canvas_sprite_IB, 4, sprite_shader.stride, 6, canvas.sprite.varr, Sprite::IARR);
	gen_vao_dynamic_draw(&checkerboard_VAO, &checkerboard_VB, &checkerboard_IB, 4, sprite_shader.stride, 6, canvas.checkerboard.varr, Sprite::IARR);
	gen_vao_dynamic_draw(&background_VAO, &background_VB, &background_IB, 4, sprite_shader.stride, 6, background.varr, Sprite::IARR);
	attrib_pointers(sprite_shader.attributes, sprite_shader.stride);
	set_projection();
	send_view();

	clip.x = 0;
	clip.y = 0;
	clip.screen_w = window->width();
	clip.screen_h = window->height();
	clip.window_size_to_bounds = [](int width, int height) -> glm::ivec4 { return { 0, 0, width, height }; };

	background.sync_texture_slot(BACKGROUND_TSLOT);
	canvas.checkerboard.sync_texture_slot(CHECKERBOARD_TSLOT);
	canvas.sprite.sync_texture_slot(CANVAS_SPRITE_TSLOT);

	background.set_modulation(ColorFrame(HSV(0.5f, 0.15f, 0.15f), 0.5f));
	background.transform.scale = { float(window->width()), float(window->height()) };
	background.sync_transform_rs();
	window->clbk_window_size.push_back([this](const Callback::WindowSize& ws) {
		QUASAR_GL(glViewport(0, 0, ws.width, ws.height));
		clip.update_window_size(ws.width, ws.height);
		set_projection(float(ws.width), float(ws.height));
		send_view();
		background.transform.scale = { float(ws.width), float(ws.height) };
		background.sync_transform_rs();
		Machine.on_render();
		});
}

Easel::~Easel()
{
	delete_vao_buffers(canvas_sprite_VAO, canvas_sprite_VB, canvas_sprite_IB);
	delete_vao_buffers(checkerboard_VAO, checkerboard_VB, checkerboard_IB);
	delete_vao_buffers(background_VAO, background_VB, background_IB);
}

void Easel::set_projection(float width, float height)
{
	projection = glm::ortho<float>(0.0f, width * app_scale.x, 0.0f, height * app_scale.y);
}

void Easel::set_projection()
{
	projection = glm::ortho<float>(0.0f, window->width() * app_scale.x, 0.0f, window->height() * app_scale.y);
}

void Easel::render() const
{
	// bind
	QUASAR_GL(glUseProgram(sprite_shader.rid));
	QUASAR_GL(glScissor(clip.x, clip.y, clip.screen_w, clip.screen_h));
	// background
	QUASAR_GL(glBindVertexArray(background_VAO));
	Image* img = Images.get(background.image); // LATER honestly, since there aren't many images/shaders, not much point in registries. This isn't a game engine.
	bind_texture(img->tid, BACKGROUND_TSLOT);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), background.varr)); // TODO only buffersubdata when modifying varr, not every draw call.
	QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
	// canvas
	if (canvas_visible)
	{
		// checkerboard
		QUASAR_GL(glBindVertexArray(checkerboard_VAO));
		img = Images.get(canvas.checkerboard.image);
		bind_texture(img->tid, CHECKERBOARD_TSLOT);
		QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), canvas.checkerboard.varr));
		QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
		// canvas sprite
		QUASAR_GL(glBindVertexArray(canvas_sprite_VAO));
		img = Images.get(canvas.sprite.image);
		bind_texture(img->tid, CANVAS_SPRITE_TSLOT);
		QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), canvas.sprite.varr));
		QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
	}
	// unbind
	QUASAR_GL(glBindVertexArray(0));
	QUASAR_GL(glUseProgram(0));
	QUASAR_GL(glScissor(0, 0, window->width(), window->height()));
}

void Easel::send_view()
{
	glm::mat3 cameraVP = projection * view.camera();
	GLint prev;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prev);
	if (sprite_shader.rid != prev)
	{
		QUASAR_GL(glUseProgram(sprite_shader.rid));
	}
	QUASAR_GL(glUniformMatrix3fv(sprite_shader.uniform_locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
	if (sprite_shader.rid != prev)
	{
		QUASAR_GL(glUseProgram(prev));
	}
}

glm::vec2 Easel::to_world_coordinates(const glm::vec2& screen_coordinates) const
{
	glm::vec3 ndc{};
	ndc.x = 1.0f - 2.0f * (screen_coordinates.x / window->width());
	ndc.y = 1.0f - 2.0f * (screen_coordinates.y / window->height());
	ndc.z = 1.0f;

	glm::mat3 invVP = glm::inverse(projection * view.camera());
	glm::vec3 world_pos = invVP * ndc;

	if (world_pos.z != 0.0f)
		world_pos / world_pos.z;

	return glm::vec2{ -world_pos.x, -world_pos.y };
}

glm::vec2 Easel::to_screen_coordinates(const glm::vec2& world_coordinates) const
{
	glm::vec3 world_pos{ world_coordinates.x, -world_coordinates.y, 1.0f };
	glm::mat3 VP = projection * view.camera();
	glm::vec3 clip_space_pos = VP * world_pos;
	glm::vec2 screen_coo{};
	screen_coo.x = (1.0f + clip_space_pos.x) * 0.5f * window->width();
	screen_coo.y = (1.0f + clip_space_pos.y) * 0.5f * window->height();
	return screen_coo;
}

void Easel::set_app_scale(float x, float y)
{
	app_scale.x = 1.0f / x;
	app_scale.y = 1.0f / y;
	set_projection();
	send_view();
	// LATER scale cursor? Have different discrete cursor sizes (only works for custom cursors).
}

glm::vec2 Easel::get_app_scale() const
{
	return 1.0f / app_scale;
}
