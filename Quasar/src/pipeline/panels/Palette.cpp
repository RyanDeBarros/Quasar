#include "Palette.h"

#include "user/Machine.h"
#include "variety/GLutility.h"
#include "../render/Uniforms.h"
#include "../widgets/ColorPicker.h"
#include "../widgets/ColorPalette.h"

static ColorPicker& color_picker(Palette* palette)
{
	return cpk_wget(palette->widget, Palette::COLOR_PICKER);
}

static const ColorPicker& color_picker(const Palette* palette)
{
	return cpk_wget(palette->widget, Palette::COLOR_PICKER);
}

static ColorPalette& color_palette(Palette* palette)
{
	return cpl_wget(palette->widget, Palette::COLOR_PALETTE);
}

static const ColorPalette& color_palette(const Palette* palette)
{
	return cpl_wget(palette->widget, Palette::COLOR_PALETTE);
}

Palette::Palette()
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) } }),
	widget(_W_COUNT)
{
	update_primary_color = [this](RGBA color) { color_picker(this).set_color(color); };
	get_picker_rgba = [this]() { return color_picker(this).get_color().rgba(); };
	initialize_widget();

	// TODO remove:
	std::vector<RGBA> colors;
	float num_colors = 10;
	for (int i = 0; i < num_colors; ++i)
		colors.push_back(HSVA(i / num_colors, 1.0f, 1.0f, 1.0f).to_rgba());
	color_palette(this).import_color_scheme(ColorScheme{ { std::make_shared<ColorSubscheme>(std::move(colors)) } });

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
	color_picker(this).process();
	color_palette(this).process();
	color_picker(this).draw();
	color_palette(this).draw();
}

void Palette::initialize_widget()
{
	assign_widget(&widget, COLOR_PICKER, new ColorPicker(&vp, Machine.palette_mb_handler, Machine.palette_key_handler)); // LATER initialize panels early and put mb_handlers as data members of panels?
	widget.wp_at(COLOR_PICKER).transform.scale = Scale(0.9f);

	assign_widget(&widget, COLOR_PALETTE, new ColorPalette(&vp, Machine.palette_mb_handler, Machine.palette_key_handler, Machine.palette_scroll_handler, &update_primary_color, &get_picker_rgba));
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
	color_picker(this).set_size(color_picker_size);
	widget.wp_at(COLOR_PICKER).transform.position = { 0, bounds.clip().screen_h * 0.5f * Machine.inv_app_scale().y - color_picker_size.y * widget.wp_at(COLOR_PICKER).transform.scale.y * 0.5f - 20 };
	color_picker(this).send_vp();

	Scale color_palette_size{ 240, 360 };
	color_palette(this).set_size(color_palette_size);
	widget.wp_at(COLOR_PALETTE).transform.position = { 0, -200 };
	color_palette(this).send_vp();
	unbind_shader();
}
