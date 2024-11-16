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

const Scale color_palette_scale = Scale(0.9f);
const Scale color_picker_scale = Scale(0.9f);
const float padding = 20;

Palette::Palette()
	: sprite_shader(FileSystem::shader_path("flatsprite.vert"), FileSystem::shader_path("flatsprite.frag.tmpl"), { { "$NUM_TEXTURE_SLOTS", std::to_string(GLC.max_texture_image_units) } }),
	widget(_W_COUNT)
{
	update_pri_color = [this](RGBA color) { color_picker(this).set_pri_color(color, false); };
	update_alt_color = [this](RGBA color) { color_picker(this).set_alt_color(color, false); };
	get_picker_pri_rgba = [this]() { return color_picker(this).get_pri_color().rgba(); };
	get_picker_alt_rgba = [this]() { return color_picker(this).get_alt_color().rgba(); };
	use_primary = [this]() { return color_picker(this).get_editing_color() == ColorPicker::EditingColor::PRIMARY; };
	use_alternate = [this]() { return color_picker(this).get_editing_color() == ColorPicker::EditingColor::ALTERNATE; };
	initialize_widget();

	// ##########################################################
	// LATER replace with default palette found in settings file:
	float num_colors = 250;
	std::vector<RGBA> colors0;
	for (int i = 0; i < num_colors; ++i)
		colors0.push_back(HSVA(i / num_colors, 1.0f, 1.0f, 1.0f).to_rgba());

	num_colors = 100;
	std::vector<RGBA> colors1;
	for (int i = 0; i < num_colors; ++i)
		colors1.push_back(HSVA(i / num_colors, 1.0f, 1.0f, 1.0f).to_rgba());

	num_colors = 10;
	std::vector<RGBA> colors2;
	for (int i = 0; i < num_colors; ++i)
		colors2.push_back(HSVA(i / num_colors, 1.0f, 1.0f, 1.0f).to_rgba());

	color_palette(this).import_color_scheme(std::make_shared<ColorScheme>(std::vector<std::shared_ptr<ColorSubscheme>>{
		std::make_shared<ColorSubscheme>("test#0", std::move(colors0)),
		std::make_shared<ColorSubscheme>("test#1", std::move(colors1)),
		std::make_shared<ColorSubscheme>("test#2", std::move(colors2))
		}), false); // LATER this would be true when importing a new scheme, but false for default color scheme upon opening application/new file
	// ##########################################################

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

Scale Palette::minimum_screen_display() const
{
	return to_screen_coordinates(color_picker_scale * color_picker(this).minimum_display() + color_palette_scale * color_palette(this).minimum_display() + Scale{ 2 * padding, 3 * padding }) - to_screen_coordinates({});
}

void Palette::initialize_widget()
{
	assign_widget(&widget, COLOR_PICKER, std::make_shared<ColorPicker>(&vp, Machine.palette_mb_handler, Machine.palette_key_handler)); // LATER initialize panels early and put mb_handlers as data members of panels?
	widget.wp_at(COLOR_PICKER).transform.scale = Scale(color_picker_scale);

	assign_widget(&widget, COLOR_PALETTE, std::make_shared<ColorPalette>(&vp, Machine.palette_mb_handler, Machine.palette_key_handler,
		Machine.palette_scroll_handler, ColorPalette::Reflection(&update_pri_color, &update_alt_color, &get_picker_pri_rgba, &get_picker_alt_rgba, &use_primary, &use_alternate)));
	widget.wp_at(COLOR_PALETTE).transform.scale = Scale(color_palette_scale);
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

	float view_height = (to_view_coordinates({}) - to_view_coordinates({ 0, bounds.clip().screen_h })).y;
	const float free_space_y = view_height - 3 * padding;
	const float color_picker_height = 420; // LATER palette min height is at least 420 + 3 * padding + min color palette height, or else scale Palette view down.
	const float color_palette_height = free_space_y - color_picker_height * color_picker_scale.y;

	Scale color_picker_size{ 240, color_picker_height };
	color_picker(this).set_size(color_picker_size, false);
	widget.wp_at(COLOR_PICKER).transform.position = { 0, 0.5f * view_height - color_picker_size.y * widget.wp_at(COLOR_PICKER).transform.scale.y * 0.5f - padding };
	color_picker(this).send_vp();

	Scale color_palette_size{ 240, color_palette_height / color_palette_scale.y };
	color_palette(this).set_size(color_palette_size, false);
	widget.wp_at(COLOR_PALETTE).transform.position = { 0, -0.5f * view_height + color_palette_size.y * widget.wp_at(COLOR_PALETTE).transform.scale.y * 0.5f + padding };
	color_palette(this).send_vp();
	unbind_shader();
}
