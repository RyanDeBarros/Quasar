#include "ColorPalette.h"

#include <glm/gtc/type_ptr.inl>

#include "../render/Uniforms.h"
#include "RoundRect.h"
#include "user/Machine.h"

ColorSubpalette::ColorSubpalette(Shader* color_square_shader)
	: Widget(_W_COUNT)
{
	assign_widget(this, SQUARES, new W_IndexedRenderable(color_square_shader));
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
	int leftover = (int)subscheme->get_colors().size() - scroll_offset * ColorPalette::COL_COUNT;
	int num_to_draw = std::clamp(leftover, 0, ColorPalette::COL_COUNT * ColorPalette::ROW_COUNT);
	ir_wget(*this, SQUARES).draw(num_to_draw * 6, scroll_offset * ColorPalette::COL_COUNT * 6);
}

void ColorSubpalette::sync_with_palette()
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	WidgetPlacement global;
	for (int i = 0; i < squares.iarr.size() / 6; ++i)
	{
		global = square_wp(i).relative_to(parent->self.transform);
		squares.set_attribute_single_vertex(i * 4 + 0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
		squares.set_attribute_single_vertex(i * 4 + 1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
		squares.set_attribute_single_vertex(i * 4 + 2, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
		squares.set_attribute_single_vertex(i * 4 + 3, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
		glm::vec4 color = (i < subscheme->get_colors().size() ? subscheme->get_colors()[i] : RGBA::WHITE).as_vec(); // TODO don't use MAX_COLORS. have unlimited variable number of colors and scroll pane. Only COL_COUNT is a constant.
		squares.set_attribute_single_vertex(i * 4 + 0, 1, glm::value_ptr(color));
		squares.set_attribute_single_vertex(i * 4 + 1, 1, glm::value_ptr(color));
		squares.set_attribute_single_vertex(i * 4 + 2, 1, glm::value_ptr(color));
		squares.set_attribute_single_vertex(i * 4 + 3, 1, glm::value_ptr(color));
	}
	squares.send_vertex_buffer();
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

ColorSubpalette& ColorPalette::get_subpalette(size_t pos)
{
	return cspl_wget(*this, subpalette_index_in_widget(pos));
}

const ColorSubpalette& ColorPalette::get_subpalette(size_t pos) const
{
	return cspl_wget(*this, subpalette_index_in_widget(pos));
}

size_t ColorPalette::subpalette_index_in_widget(size_t pos) const
{
	return SUBPALETTE_START + pos;
}

ColorPalette::ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler, ScrollHandler& parent_scroll_handler)
	: Widget(SUBPALETTE_START), vp(vp), parent_mb_handler(parent_mb_handler), parent_key_handler(parent_key_handler), parent_scroll_handler(parent_scroll_handler),
	grid_shader(FileSystem::shader_path("palette/black_grid.vert"), FileSystem::shader_path("palette/black_grid.frag")),
	color_square_shader(FileSystem::shader_path("palette/color_square.vert"), FileSystem::shader_path("palette/color_square.frag")),
	round_rect_shader(FileSystem::shader_path("round_rect.vert"), FileSystem::shader_path("round_rect.frag"))
{
	initialize_widget();
	connect_input_handlers();
	new_subpalette(); // always have at least one subpalette

	// TODO remove:
	float num_colors = 100;
	for (int i = 0; i < num_colors; ++i)
		get_subpalette(current_subscheme).subscheme->insert(HSVA(i / num_colors, 1.0f, 1.0f, 1.0f).to_rgba());
	get_subpalette(current_subscheme).reload_subscheme();
}

ColorPalette::~ColorPalette()
{
	parent_mb_handler.remove_child(&mb_handler);
	parent_key_handler.remove_child(&key_handler);
	parent_scroll_handler.remove_child(&scroll_handler);
}

void ColorPalette::draw()
{
	// TODO render imgui
	rr_wget(*this, BACKGROUND).draw();
	get_subpalette(current_subscheme).draw();
	ur_wget(*this, BLACK_GRID).draw();
}

void ColorPalette::send_vp()
{
	Uniforms::send_matrix3(color_square_shader, "u_VP", *vp);
	Uniforms::send_matrix3(grid_shader, "u_VP", *vp);
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
	attach_widget(this, new ColorSubpalette(&color_square_shader));
	scheme.subschemes.push_back(std::make_shared<ColorSubscheme>(std::vector<RGBA>{ RGBA::WHITE }));
	current_subscheme = num_subpalettes() - 1;
	get_subpalette(current_subscheme).subscheme = scheme.subschemes[current_subscheme];
	get_subpalette(current_subscheme).self.transform.position.y = GRID_OFFSET_Y;
	get_subpalette(current_subscheme).reload_subscheme();
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
		auto bl = Machine.to_screen_coordinates(children[BACKGROUND]->global_of({ -0.5f, -0.5f }), *vp);
		auto tr = Machine.to_screen_coordinates(children[BACKGROUND]->global_of({ 0.5f, 0.5f }), *vp);
		if (in_diagonal_rect(Machine.main_window->cursor_pos(), bl, tr))
		{
			scroll_backlog -= s.yoff;
			float amount;
			scroll_backlog = modf(scroll_backlog, &amount);
			get_subpalette(current_subscheme).scroll_by((int)amount);
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
