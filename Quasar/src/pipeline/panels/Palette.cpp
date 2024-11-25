#include "Palette.h"

#include <glm/gtc/type_ptr.inl>

#include "ImplUtility.h"
#include "user/Machine.h"
#include "variety/GLutility.h"
#include "Easel.h"
#include "../render/Uniforms.h"
#include "../widgets/ColorPicker.h"
#include "../widgets/ColorPalette.h"

static ColorPicker& color_picker(PalettePanel* palette)
{
	return cpk_wget(palette->widget, PalettePanel::COLOR_PICKER);
}

static const ColorPicker& color_picker(const PalettePanel* palette)
{
	return cpk_wget(palette->widget, PalettePanel::COLOR_PICKER);
}

static ColorPalette& color_palette(PalettePanel* palette)
{
	return cpl_wget(palette->widget, PalettePanel::COLOR_PALETTE);
}

static const ColorPalette& color_palette(const PalettePanel* palette)
{
	return cpl_wget(palette->widget, PalettePanel::COLOR_PALETTE);
}

const Scale color_palette_scale = Scale(0.9f);
const Scale color_picker_scale = Scale(0.9f);
const float padding = 20;

PalettePanel::PalettePanel()
	: bkg_shader(FileSystem::shader_path("color_square.vert"), FileSystem::shader_path("color_square.frag")),
	widget(_W_COUNT)
{
	update_pri_color = [this](RGBA color) { color_picker(this).set_pri_color(color, false); };
	update_alt_color = [this](RGBA color) { color_picker(this).set_alt_color(color, false); };
	get_picker_pri_rgba = [this]() { return color_picker(this).get_pri_color().rgba(); };
	get_picker_alt_rgba = [this]() { return color_picker(this).get_alt_color().rgba(); };
	use_primary = [this]() { return color_picker(this).get_editing_color() == ColorPicker::EditingColor::PRIMARY; };
	use_alternate = [this]() { return color_picker(this).get_editing_color() == ColorPicker::EditingColor::ALTERNATE; };
	swap_picker_colors = [this]() { color_picker(this).swap_picker_colors(); };
	emit_modified_primary = [this](RGBA rgba) {	MEasel->canvas().set_primary_color(rgba); };
	emit_modified_alternate = [this](RGBA rgba) { MEasel->canvas().set_alternate_color(rgba); };
}

void PalettePanel::initialize()
{
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
}

void PalettePanel::draw()
{
	ur_wget(widget, BACKGROUND).draw();
	render_widget();
}

void PalettePanel::process()
{
	color_picker(this).process();
	color_palette(this).process();
}

void PalettePanel::render_widget()
{
	color_picker(this).draw();
	color_palette(this).draw();
}

Scale PalettePanel::minimum_screen_display() const
{
	Scale cpk = color_picker_scale * color_picker(this).minimum_display();
	Scale cpl = color_palette_scale * color_palette(this).minimum_display();
	return to_screen_size({ std::max(cpk.x, cpl.x) + 2 * padding, cpk.y + cpl.y + 3 * padding });
}

void PalettePanel::new_color()
{
	bool adjacent = !MainWindow->is_shift_pressed();
	bool update_selector = MainWindow->is_ctrl_pressed();
	if (MainWindow->is_alt_pressed())
		color_palette(this).current_subpalette().new_color_from_alternate(color_picker(this).get_alt_color().rgba(), adjacent, update_selector, true);
	else
		color_palette(this).current_subpalette().new_color_from_primary(color_picker(this).get_pri_color().rgba(), adjacent, update_selector, true);
}

void PalettePanel::overwrite_color()
{
	if (MainWindow->is_alt_pressed())
		color_palette(this).subpalette_overwrite_alternate(true);
	else
		color_palette(this).subpalette_overwrite_primary(true);
}

void PalettePanel::delete_color()
{
	ColorSubpalette& subpalette = color_palette(this).current_subpalette();
	if (MainWindow->is_alt_pressed())
		subpalette.remove_square_under_index(subpalette.alternate_index, true, true);
	else
		subpalette.remove_square_under_index(subpalette.primary_index, true, true);
}

void PalettePanel::new_subpalette()
{
	color_palette(this).new_subpalette(true);
}

void PalettePanel::rename_subpalette()
{
	color_palette(this).rename_subpalette();
}

void PalettePanel::delete_subpalette()
{
	color_palette(this).delete_current_subpalette(true);
}

void PalettePanel::set_pri_color(RGBA color)
{
	color_picker(this).set_pri_color(color, false);
}

void PalettePanel::set_alt_color(RGBA color)
{
	color_picker(this).set_alt_color(color, false);
}

void PalettePanel::initialize_widget()
{
	assign_widget(&widget, BACKGROUND, std::make_shared<W_UnitRenderable>(&bkg_shader));
	ur_wget(widget, BACKGROUND).set_attribute(1, glm::value_ptr(RGBA(0.2f, 0.1f, 0.3f, 0.1f).as_vec())).send_buffer();

	assign_widget(&widget, COLOR_PICKER, std::make_shared<ColorPicker>(&vp, mb_handler, key_handler, ColorPicker::Reflection(&emit_modified_primary, &emit_modified_alternate)));
	widget.wp_at(COLOR_PICKER).transform.scale = Scale(color_picker_scale);

	assign_widget(&widget, COLOR_PALETTE, std::make_shared<ColorPalette>(&vp, mb_handler, key_handler, scroll_handler,
		ColorPalette::Reflection(&update_pri_color, &update_alt_color, &get_picker_pri_rgba, &get_picker_alt_rgba, &use_primary, &use_alternate, &swap_picker_colors)));
	widget.wp_at(COLOR_PALETTE).transform.scale = Scale(color_palette_scale);
}

void PalettePanel::sync_widget()
{
	widget.wp_at(BACKGROUND).transform.scale = get_app_size();
	Utils::set_vertex_pos_attributes(ur_wget(widget, BACKGROUND), widget.wp_at(BACKGROUND).relative_to(widget.self.transform));

	float view_height = to_view_size({ 0, bounds.clip().screen_h }).y;
	const float free_space_y = view_height - 3 * padding;
	const float color_picker_height = 420;
	const float color_palette_height = free_space_y - color_picker_height * color_picker_scale.y;

	Scale color_picker_size{ 240, color_picker_height };
	color_picker(this).set_size(color_picker_size, false);
	widget.wp_at(COLOR_PICKER).transform.position = { 0, 0.5f * view_height - color_picker_size.y * widget.wp_at(COLOR_PICKER).transform.scale.y * 0.5f - padding };
	color_picker(this).send_vp();

	Scale color_palette_size{ 240, color_palette_height / color_palette_scale.y };
	color_palette(this).set_size(color_palette_size, false);
	widget.wp_at(COLOR_PALETTE).transform.position = { 0, -0.5f * view_height + color_palette_size.y * widget.wp_at(COLOR_PALETTE).transform.scale.y * 0.5f + padding };
	color_palette(this).send_vp();
}

void PalettePanel::_send_view()
{
	vp = vp_matrix();
	Uniforms::send_matrix3(bkg_shader, "u_VP", vp);
	unbind_shader();
	sync_widget();
}
