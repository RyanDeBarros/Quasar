#include "Palette.h"

#include "user/Machine.h"
#include "variety/GLutility.h"
#include "../render/Uniforms.h"
#include "../widgets/ColorPicker.h"
#include "../widgets/ColorPalette.h"

Palette::Palette()
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) } }),
	widget(_W_COUNT)
{
	initialize_widget();

	static constexpr size_t num_quads = 1;
	varr = new GLfloat[num_quads * FlatSprite::NUM_VERTICES * FlatSprite::STRIDE];
	background.varr = varr;
	background.initialize_varr();

	GLuint IARR[num_quads * 6]{
		0, 1, 2, 2, 3, 0
	};
	initialize_dynamic_vao(vao, vb, ib, num_quads * FlatSprite::NUM_VERTICES, sprite_shader.stride, sizeof(IARR) / sizeof(*IARR), varr, IARR, sprite_shader.attributes);
	
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
	// widget
	render_widget();
	// unbind
	unbind_vao_buffers();
	unbind_shader();
}

void Palette::render_widget()
{
	cpk_wget(widget, COLOR_PICKER).draw();
	cpl_wget(widget, COLOR_PALETTE).draw();
}

void Palette::initialize_widget()
{
	assign_widget(&widget, COLOR_PICKER, new ColorPicker(&vp, Machine.palette_mb_handler, Machine.palette_key_handler)); // LATER initialize panels early and put mb_handlers as data members of panels?
	widget.wp_at(COLOR_PICKER).transform.scale = Scale(0.9f);

	assign_widget(&widget, COLOR_PALETTE, new ColorPalette(&vp, Machine.palette_mb_handler, Machine.palette_key_handler));
	widget.wp_at(COLOR_PALETTE).transform.scale = Scale(0.9f);
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
	vp = vp_matrix();
	Uniforms::send_matrix3(sprite_shader, "u_VP", vp);

	Scale color_picker_size{ 240, 420 };
	cpk_wget(widget, COLOR_PICKER).set_size(color_picker_size);
	widget.wp_at(COLOR_PICKER).transform.position = { 0, bounds.clip().screen_h * 0.5f * Machine.inv_app_scale().y - color_picker_size.y * widget.wp_at(COLOR_PICKER).transform.scale.y * 0.5f - 20 };
	cpk_wget(widget, COLOR_PICKER).send_vp();

	//widget.wp_at(COLOR_PALETTE).transform.position = { 0, -bounds.clip().screen_h * 0.5f * Machine.inv_app_scale().y * 0.5f };
	widget.wp_at(COLOR_PALETTE).transform.position = { 0, -200 };
	cpl_wget(widget, COLOR_PALETTE).send_vp();
	unbind_shader();
}
