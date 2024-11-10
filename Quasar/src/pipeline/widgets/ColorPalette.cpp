#include "ColorPalette.h"

#include <glm/gtc/type_ptr.inl>

#include "../render/Uniforms.h"
#include "RoundRect.h"
#include "user/Machine.h"

ColorSubpalette::ColorSubpalette(Shader* color_square_shader, Shader* outline_rect_shader)
	: Widget(_W_COUNT)
{
	assign_widget(this, SQUARES, new W_IndexedRenderable(color_square_shader));
	
	assign_widget(this, HOVER_SELECTOR, new W_UnitRenderable(outline_rect_shader));
	UnitRenderable& ur = ur_wget(*this, HOVER_SELECTOR);
	ur.set_attribute(1, glm::value_ptr(RGBA::WHITE.as_vec()));
	ur.set_attribute_single_vertex(0, 2, glm::value_ptr(glm::vec2{ 0, 0 }));
	ur.set_attribute_single_vertex(1, 2, glm::value_ptr(glm::vec2{ 1, 0 }));
	ur.set_attribute_single_vertex(2, 2, glm::value_ptr(glm::vec2{ 0, 1 }));
	ur.set_attribute_single_vertex(3, 2, glm::value_ptr(glm::vec2{ 1, 1 }));
	ur.send_buffer();
	// TODO setup HOVER_SELECTOR
}

void ColorSubpalette::reload_subscheme()
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	squares.varr.clear();
	squares.iarr.clear();
	squares.set_num_vertices(subscheme->get_colors().size() * 4);
	squares.push_back_quads(subscheme->get_colors().size());
	squares.send_both_buffers_resized();
	sync_with_palette();
}

void ColorSubpalette::draw()
{
	ir_wget(*this, SQUARES).draw(num_squares_visible() * size_t(6), first_square() * size_t(6));
}

void ColorSubpalette::draw_selectors()
{
	if (current_hover_index >= 0)
		ur_wget(*this, HOVER_SELECTOR).draw();
}

void ColorSubpalette::sync_hover_selector()
{
	const UnitRenderable& ur = ur_wget(*this, HOVER_SELECTOR);
	ur.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ hover_wp.left(), hover_wp.bottom() }));
	ur.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ hover_wp.right(), hover_wp.bottom() }));
	ur.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ hover_wp.left(), hover_wp.top() }));
	ur.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ hover_wp.right(), hover_wp.top() }));
	ur.send_buffer();
}

void ColorSubpalette::sync_with_palette()
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	WidgetPlacement global;
	size_t num_colors = subscheme->get_colors().size();
	for (size_t i = 0; i < num_colors; ++i)
	{
		global = square_wp((int)i).relative_to(parent->self.transform);
		squares.set_attribute_single_vertex(i * 4 + 0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
		squares.set_attribute_single_vertex(i * 4 + 1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
		squares.set_attribute_single_vertex(i * 4 + 2, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
		squares.set_attribute_single_vertex(i * 4 + 3, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
		glm::vec4 color = subscheme->get_colors()[i].as_vec();
		squares.set_attribute_single_vertex(i * 4 + 0, 1, glm::value_ptr(color));
		squares.set_attribute_single_vertex(i * 4 + 1, 1, glm::value_ptr(color));
		squares.set_attribute_single_vertex(i * 4 + 2, 1, glm::value_ptr(color));
		squares.set_attribute_single_vertex(i * 4 + 3, 1, glm::value_ptr(color));
	}
	squares.send_vertex_buffer();
}

void ColorSubpalette::process()
{
	current_hover_index = -1;
	WidgetPlacement global;
	Position cursor_pos = Machine.to_world_coordinates(Machine.main_window->cursor_pos(), glm::inverse(*dynamic_cast<ColorPalette*>(parent)->vp));
	int end_square = first_square() + num_squares_visible();
	for (int i = first_square(); i < end_square; ++i)
	{
		global = square_wp(i).relative_to(parent->self.transform);
		if (global.contains_point(cursor_pos))
		{
			current_hover_index = i;
			if (hover_wp != global)
			{
				hover_wp = global;
				sync_hover_selector();
			}
		}
	}
}

void ColorSubpalette::scroll_by(int delta)
{
	scroll_offset = std::clamp(scroll_offset + delta, 0, std::max(0, ceil_divide((int)subscheme->get_colors().size(), ColorPalette::COL_COUNT) - ColorPalette::ROW_COUNT));
	sync_with_palette();
}

WidgetPlacement ColorSubpalette::square_wp(int i) const
{
	Position initial_pos{ -ColorPalette::SQUARE_SEP * (ColorPalette::COL_COUNT - 1) * 0.5f, ColorPalette::SQUARE_SEP * (ColorPalette::ROW_COUNT - 1) * 0.5f };
	Position delta_pos{ ColorPalette::SQUARE_SEP * (i % ColorPalette::COL_COUNT), -ColorPalette::SQUARE_SEP * (i / ColorPalette::ROW_COUNT - scroll_offset) };
	return WidgetPlacement{ { { initial_pos + delta_pos}, ColorPalette::SQUARE_SIZE } }.relative_to(self.transform);
}

int ColorSubpalette::first_square() const
{
	return scroll_offset * ColorPalette::COL_COUNT;
}

int ColorSubpalette::num_squares_visible() const
{
	int leftover = (int)subscheme->get_colors().size() - first_square();
	return std::clamp(leftover, 0, ColorPalette::COL_COUNT * ColorPalette::ROW_COUNT);
}

ColorSubpalette& ColorPalette::get_subpalette(size_t pos)
{
	return cspl_wget(*this, subpalette_index_in_widget(pos));
}

const ColorSubpalette& ColorPalette::get_subpalette(size_t pos) const
{
	return cspl_wget(*this, subpalette_index_in_widget(pos));
}

ColorSubpalette& ColorPalette::current_subpalette()
{
	return get_subpalette(current_subscheme);
}

const ColorSubpalette& ColorPalette::current_subpalette() const
{
	return get_subpalette(current_subscheme);
}

size_t ColorPalette::subpalette_index_in_widget(size_t pos) const
{
	return SUBPALETTE_START + pos;
}

// LATER use black outlined square IR instead of single-unit blackgrid shader?
ColorPalette::ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler, ScrollHandler& parent_scroll_handler)
	: Widget(SUBPALETTE_START), vp(vp), parent_mb_handler(parent_mb_handler), parent_key_handler(parent_key_handler), parent_scroll_handler(parent_scroll_handler),
	grid_shader(FileSystem::shader_path("palette/black_grid.vert"), FileSystem::shader_path("palette/black_grid.frag")),
	color_square_shader(FileSystem::shader_path("palette/color_square.vert"), FileSystem::shader_path("palette/color_square.frag")),
	outline_rect_shader(FileSystem::shader_path("palette/outline_rect.vert"), FileSystem::shader_path("palette/outline_rect.frag")),
	round_rect_shader(FileSystem::shader_path("round_rect.vert"), FileSystem::shader_path("round_rect.frag"))
{
	initialize_widget();
	connect_input_handlers();
	new_subpalette(); // always have at least one subpalette

	// TODO remove:
	float num_colors = 100;
	for (int i = 0; i < num_colors; ++i)
		current_subpalette().subscheme->insert(HSVA(i / num_colors, 1.0f, 1.0f, 1.0f).to_rgba());
	current_subpalette().reload_subscheme();
}

ColorPalette::~ColorPalette()
{
	parent_mb_handler.remove_child(&mb_handler);
	parent_key_handler.remove_child(&key_handler);
	parent_scroll_handler.remove_child(&scroll_handler);
}

void ColorPalette::draw()
{
	current_subpalette().process();
	// TODO render imgui
	rr_wget(*this, BACKGROUND).draw();
	current_subpalette().draw();
	ur_wget(*this, BLACK_GRID).draw();
	current_subpalette().draw_selectors();
}

void ColorPalette::send_vp()
{
	Uniforms::send_matrix3(color_square_shader, "u_VP", *vp);
	Uniforms::send_matrix3(grid_shader, "u_VP", *vp);
	Uniforms::send_matrix3(outline_rect_shader, "u_VP", *vp);
	Uniforms::send_matrix3(round_rect_shader, "u_VP", *vp);
	sync_widget_with_vp();
}

void ColorPalette::import_color_scheme(const ColorScheme& color_scheme)
{
	scheme = color_scheme;
	import_color_scheme();
}

void ColorPalette::import_color_scheme(ColorScheme&& color_scheme)
{
	scheme = std::move(color_scheme);
	import_color_scheme();
}

void ColorPalette::new_subpalette()
{
	attach_widget(this, new ColorSubpalette(&color_square_shader, &outline_rect_shader));
	scheme.subschemes.push_back(std::make_shared<ColorSubscheme>(std::vector<RGBA>{ RGBA::WHITE }));
	current_subscheme = num_subpalettes() - 1;
	current_subpalette().subscheme = scheme.subschemes[current_subscheme];
	current_subpalette().self.transform.position.y = GRID_OFFSET_Y;
	current_subpalette().reload_subscheme();
}

void ColorPalette::delete_subpalette(size_t pos)
{
	if (pos >= num_subpalettes())
		return;
	detach_widget(this, SUBPALETTE_START + pos);
	scheme.subschemes.erase(scheme.subschemes.begin() + pos);
	if (num_subpalettes() == 0)
		new_subpalette();
	if (current_subscheme >= num_subpalettes())
		current_subscheme = num_subpalettes() - 1;
}

size_t ColorPalette::num_subpalettes() const
{
	return children.size() - SUBPALETTE_START;
}

void ColorPalette::connect_input_handlers()
{
	parent_mb_handler.children.push_back(&mb_handler);
	mb_handler.callback = [](const MouseButtonEvent& mb) {
		// TODO
		};
	parent_key_handler.children.push_back(&key_handler);
	key_handler.callback = [](const KeyEvent& k) {
		// TODO
		};
	parent_scroll_handler.children.push_back(&scroll_handler);
	scroll_handler.callback = [this](const ScrollEvent& s) {
		if (children[BACKGROUND]->contains_screen_point(Machine.main_window->cursor_pos(), vp))
		{
			scroll_backlog -= s.yoff;
			float amount;
			scroll_backlog = modf(scroll_backlog, &amount);
			current_subpalette().scroll_by((int)amount);
		}
		};
}

void ColorPalette::initialize_widget()
{
	assign_widget(this, BACKGROUND, new RoundRect(&round_rect_shader));
	rr_wget(*this, BACKGROUND).thickness = 0.25f;
	rr_wget(*this, BACKGROUND).corner_radius = 10;
	rr_wget(*this, BACKGROUND).border_color = RGBA(HSV(0.7f, 0.5f, 0.5f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).update_all();

	assign_widget(this, BLACK_GRID, new W_UnitRenderable(&grid_shader));
}

void ColorPalette::import_color_scheme()
{
	if (scheme.subschemes.size() < num_subpalettes())
	{
		for (size_t i = 0; i < scheme.subschemes.size(); ++i)
		{
			get_subpalette(i).subscheme = scheme.subschemes[i];
			get_subpalette(i).reload_subscheme();
		}
		for (size_t i = scheme.subschemes.size(); i < num_subpalettes(); ++i)
			delete children[subpalette_index_in_widget(i)];
		children.erase(children.begin() + subpalette_index_in_widget(scheme.subschemes.size()), children.end());
	}
	else
	{
		for (size_t i = 0; i < num_subpalettes(); ++i)
		{
			get_subpalette(i).subscheme = scheme.subschemes[i];
			get_subpalette(i).reload_subscheme();
		}
		for (size_t i = num_subpalettes(); i < scheme.subschemes.size(); ++i)
		{
			new_subpalette();
			get_subpalette(i).subscheme = scheme.subschemes[i];
			get_subpalette(i).reload_subscheme();
		}
	}
}

void ColorPalette::sync_widget_with_vp()
{
	float sc = scale1d();
	if (cached_scale1d != sc)
	{
		cached_scale1d = sc;
		rr_wget(*this, BACKGROUND).update_corner_radius(sc).update_thickness(sc);
	}
	rr_wget(*this, BACKGROUND).update_transform().send_buffer();

	UnitRenderable& ur = ur_wget(*this, BLACK_GRID);
	WidgetPlacement global = GRID_WP.relative_to(self.transform);

	ur.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
	ur.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
	ur.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
	ur.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
	ur.set_attribute_single_vertex(0, 1, glm::value_ptr(glm::vec2{ 0, 0 }));
	ur.set_attribute_single_vertex(1, 1, glm::value_ptr(glm::vec2{ 1, 0 }));
	ur.set_attribute_single_vertex(2, 1, glm::value_ptr(glm::vec2{ 0, 1 }));
	ur.set_attribute_single_vertex(3, 1, glm::value_ptr(glm::vec2{ 1, 1 }));
	ur.send_buffer();

	for (size_t i = 0; i < num_subpalettes(); ++i)
		get_subpalette(i).sync_with_palette();
}

void ColorPalette::set_size(Scale size, bool sync)
{
	wp_at(BACKGROUND).transform.scale = size;
	rr_wget(*this, BACKGROUND).update_transform();
	if (sync)
		sync_widget_with_vp();
}
