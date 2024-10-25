#include "Palette.h"

#include "user/Machine.h"
#include "variety/GLutility.h"

Palette::Palette()
	: sprite_shader(FileSystem::resources_path("flatsprite.vert"), FileSystem::resources_path("flatsprite.frag"),
		{ 1, 2, 2, 4, 4 }, { "u_VP" })
{
	varr = new GLfloat[3 * FlatSprite::NUM_VERTICES * FlatSprite::STRIDE];
	background.varr = varr;
	background.initialize_varr();

	static constexpr size_t num_quads = 1;
	GLuint IARR[num_quads * 6]{
		0, 1, 2, 2, 3, 0
	};
	gen_dynamic_vao(vao, vb, ib, num_quads * FlatSprite::NUM_VERTICES, sprite_shader.stride, sizeof(IARR) / sizeof(*IARR), varr, IARR, sprite_shader.attributes);
	
	set_projection();
	send_view();

	background.sync_texture_slot(-1.0f);

	background.set_image(nullptr, 1, 1);
	background.set_modulation(ColorFrame(RGB(0.8f, 0.6f, 0.3f), 0.9f));
	background.transform.scale = { float(Machine.main_window->width()), float(Machine.main_window->height()) };
	background.sync_transform();
	subsend_background_vao();
	Machine.main_window->clbk_window_size.push_back([this](const Callback::WindowSize& ws) {
		set_projection(float(ws.width), float(ws.height));
		send_view();
		});
}

Palette::~Palette()
{
	delete_vao_buffers(vao, vb, ib);
	delete[] varr;
}

void Palette::set_projection(float width, float height)
{
	projection = glm::ortho<float>(0.0f, width * app_scale.x, 0.0f, height * app_scale.y);
}

void Palette::set_projection()
{
	projection = glm::ortho<float>(0.0f, Machine.main_window->width() * app_scale.x, 0.0f, Machine.main_window->height() * app_scale.y);
}

void Palette::render() const
{
	Machine.palette_clip().scissor();
	// bind
	bind_shader(sprite_shader.rid);
	// background
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
	// unbind
	unbind_vao_buffers();
	unbind_shader();
}

void Palette::subsend_background_vao() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sprite_shader.stride * sizeof(GLfloat), background.varr));
	unbind_vao_buffers();
}

void Palette::send_view()
{
	glm::mat3 cameraVP = vp_matrix();
	bind_shader(sprite_shader.rid);
	QUASAR_GL(glUniformMatrix3fv(sprite_shader.uniform_locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
	unbind_shader();
}

glm::mat3 Palette::vp_matrix() const
{
	return projection * view.camera();
}
