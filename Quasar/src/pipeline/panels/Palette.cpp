#include "Palette.h"

#include "user/Machine.h"
#include "variety/GLutility.h"
#include "../Uniforms.h"

Palette::Palette()
	: sprite_shader(FileSystem::resources_path("flatsprite.vert"), FileSystem::resources_path("flatsprite.frag")),
	color_picker(Machine.palette_mb_handler, Machine.palette_key_handler) // LATER initialize panels early and put mb_handlers as data members of panels?
{
	static constexpr size_t num_quads = 1;

	varr = new GLfloat[num_quads * FlatSprite::NUM_VERTICES * FlatSprite::STRIDE];
	background.varr = varr;
	background.initialize_varr();

	GLuint IARR[num_quads * 6]{
		0, 1, 2, 2, 3, 0
	};
	gen_dynamic_vao(vao, vb, ib, num_quads * FlatSprite::NUM_VERTICES, sprite_shader.stride, sizeof(IARR) / sizeof(*IARR), varr, IARR, sprite_shader.attributes);
	
	background.sync_texture_slot(-1.0f);
	background.set_image(nullptr, 1, 1);
	background.set_modulation(ColorFrame(RGB(0.2f, 0.1f, 0.3f), 0.1f));
}

Palette::~Palette()
{
	delete_vao_buffers(vao, vb, ib);
	delete[] varr;
}

void Palette::draw()
{
	// bind
	bind_shader(sprite_shader);
	// background
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
	// color picker
	color_picker.render();
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

void Palette::_send_view()
{
	background.transform.scale = get_app_size();
	background.sync_transform();
	subsend_background_vao();
	glm::mat3 cameraVP = vp_matrix();
	Uniforms::send_matrix3(sprite_shader, "u_VP", cameraVP);

	color_picker.set_size(Scale{240, 420} * Machine.get_app_scale());
	Position pos_rel{ 0.0f, 0.275f };
	Position ppos = pos_rel * glm::vec2{ bounds.clip().screen_w, bounds.clip().screen_h } * Machine.inv_app_scale();
	auto test = bounds.clip().screen_w;
	color_picker.set_position(ppos, to_screen_coordinates(ppos));
	color_picker.send_vp(cameraVP);
	unbind_shader();
}
