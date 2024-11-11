#include "ColorPalette.h"

#include <glm/gtc/type_ptr.inl>

#include "../render/Uniforms.h"
#include "RoundRect.h"
#include "user/Machine.h"
#include "Button.h"

ColorSubpalette::ColorSubpalette(Shader* color_square_shader, Shader* outline_rect_shader)
	: Widget(_W_COUNT)
{
	assign_widget(this, SQUARES, new W_IndexedRenderable(color_square_shader));
	
	assign_widget(this, HOVER_SELECTOR, new W_UnitRenderable(outline_rect_shader));
	UnitRenderable& hur = ur_wget(*this, HOVER_SELECTOR);
	hur.set_attribute(1, glm::value_ptr(RGBA::WHITE.as_vec()));
	hur.set_attribute_single_vertex(0, 2, glm::value_ptr(glm::vec2{ 0, 0 }));
	hur.set_attribute_single_vertex(1, 2, glm::value_ptr(glm::vec2{ 1, 0 }));
	hur.set_attribute_single_vertex(2, 2, glm::value_ptr(glm::vec2{ 0, 1 }));
	hur.set_attribute_single_vertex(3, 2, glm::value_ptr(glm::vec2{ 1, 1 }));
	hur.send_buffer();

	assign_widget(this, PRIMARY_SELECTOR_B, new W_UnitRenderable(color_square_shader, 3));
	UnitRenderable& pbur = ur_wget(*this, PRIMARY_SELECTOR_B);
	pbur.set_attribute(1, glm::value_ptr(RGBA::BLACK.as_vec()));
	pbur.send_buffer();

	assign_widget(this, PRIMARY_SELECTOR_F, new W_UnitRenderable(color_square_shader, 3));
	UnitRenderable& pfur = ur_wget(*this, PRIMARY_SELECTOR_F);
	pfur.set_attribute(1, glm::value_ptr(RGBA::WHITE.as_vec()));
	pfur.send_buffer();

	assign_widget(this, ALTERNATE_SELECTOR_B, new W_UnitRenderable(color_square_shader, 3));
	UnitRenderable& abur = ur_wget(*this, ALTERNATE_SELECTOR_B);
	abur.set_attribute(1, glm::value_ptr(RGBA::BLACK.as_vec()));
	abur.send_buffer();

	assign_widget(this, ALTERNATE_SELECTOR_F, new W_UnitRenderable(color_square_shader, 3));
	UnitRenderable& afur = ur_wget(*this, ALTERNATE_SELECTOR_F);
	afur.set_attribute(1, glm::value_ptr(RGBA::WHITE.as_vec()));
	afur.send_buffer();
}

ColorPalette& ColorSubpalette::palette()
{
	return *dynamic_cast<ColorPalette*>(parent);
}

const ColorPalette& ColorSubpalette::palette() const
{
	return *dynamic_cast<const ColorPalette*>(parent);
}

void ColorSubpalette::reload_subscheme()
{
	if (subscheme->get_colors().empty())
		subscheme->insert(RGBA::WHITE);
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	squares.varr.clear();
	squares.iarr.clear();
	squares.set_num_vertices(subscheme->get_colors().size() * 4);
	squares.push_back_quads(subscheme->get_colors().size());
	squares.send_both_buffers_resized();
	sync_with_palette();
	update_primary_color_in_picker();
}

void ColorSubpalette::draw()
{
	ir_wget(*this, SQUARES).draw(num_squares_visible() * size_t(6), first_square() * size_t(6));
}

void ColorSubpalette::draw_selectors()
{
	int first = first_square();
	int end = first + num_squares_visible();
	if (alternate_index >= first && alternate_index < end)
	{
		ur_wget(*this, ALTERNATE_SELECTOR_B).draw();
		ur_wget(*this, ALTERNATE_SELECTOR_F).draw();
	}
	if (primary_index >= first && primary_index < end)
	{
		ur_wget(*this, PRIMARY_SELECTOR_B).draw();
		ur_wget(*this, PRIMARY_SELECTOR_F).draw();
	}
	if (hover_index >= 0)
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

const float tri_first_offset = 3;
const float tri_last_offset = 15;
const float tri_second_offset = tri_last_offset - tri_first_offset * (1 + glm::sqrt(2.0f));

void ColorSubpalette::sync_primary_selector()
{
	Scale sc = global_scale();

	const UnitRenderable& ubr = ur_wget(*this, PRIMARY_SELECTOR_B);
	ubr.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ primary_wp.left(), primary_wp.top() }));
	ubr.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_last_offset * sc.x, primary_wp.top()}));
	ubr.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ primary_wp.left(), primary_wp.top() - tri_last_offset * sc.y }));
	ubr.send_buffer();

	const UnitRenderable& ufr = ur_wget(*this, PRIMARY_SELECTOR_F);
	ufr.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_first_offset * sc.x, primary_wp.top() - tri_first_offset * sc.y }));
	ufr.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_second_offset * sc.x, primary_wp.top() - tri_first_offset * sc.y }));
	ufr.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_first_offset * sc.x, primary_wp.top() - tri_second_offset * sc.y }));
	ufr.send_buffer();
}

void ColorSubpalette::resync_primary_selector()
{
	primary_wp = square_wp(primary_index).relative_to(parent->self.transform);
	sync_primary_selector();
}

void ColorSubpalette::sync_alternate_selector()
{
	Scale sc = global_scale();

	const UnitRenderable& ubr = ur_wget(*this, ALTERNATE_SELECTOR_B);
	ubr.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ alternate_wp.right(), alternate_wp.top() }));
	ubr.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_last_offset * sc.x, alternate_wp.top() }));
	ubr.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ alternate_wp.right(), alternate_wp.top() - tri_last_offset * sc.y }));
	ubr.send_buffer();

	const UnitRenderable& ufr = ur_wget(*this, ALTERNATE_SELECTOR_F);
	ufr.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_first_offset * sc.x, alternate_wp.top() - tri_first_offset * sc.y }));
	ufr.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_second_offset * sc.x, alternate_wp.top() - tri_first_offset * sc.y }));
	ufr.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_first_offset * sc.x, alternate_wp.top() - tri_second_offset * sc.y }));
	ufr.send_buffer();
}

void ColorSubpalette::resync_alternate_selector()
{
	alternate_wp = square_wp(alternate_index).relative_to(parent->self.transform);
	sync_alternate_selector();
}

void ColorSubpalette::sync_with_palette()
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	size_t num_colors = subscheme->get_colors().size();
	for (size_t i = 0; i < num_colors; ++i)
		setup_color_buffer(i, squares);
	squares.send_vertex_buffer();

	resync_primary_selector();
	resync_alternate_selector();
}

void ColorSubpalette::setup_color_buffer(size_t i, IndexedRenderable& squares) const
{
	WidgetPlacement global = square_wp((int)i).relative_to(parent->self.transform);
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

void ColorSubpalette::setup_color_buffer(size_t i)
{
	setup_color_buffer(i, ir_wget(*this, SQUARES));
}

void ColorSubpalette::process()
{
	hover_index = -1;
	if (get_visible_square_under_pos(cursor_world_pos(), hover_index))
	{
		WidgetPlacement new_wp = global_square_wp(hover_index);
		if (hover_wp != new_wp)
		{
			hover_wp = new_wp;
			sync_hover_selector();
		}
	}
}

bool ColorSubpalette::check_primary()
{
	if (get_visible_square_under_pos(cursor_world_pos(), primary_index))
	{
		WidgetPlacement new_wp = global_square_wp(primary_index);
		if (primary_wp != new_wp)
		{
			primary_wp = new_wp;
			sync_primary_selector();
		}
		return true;
	}
	else
		return false;
}

bool ColorSubpalette::check_alternate()
{
	if (get_visible_square_under_pos(cursor_world_pos(), alternate_index))
	{
		WidgetPlacement new_wp = global_square_wp(alternate_index);
		if (alternate_wp != new_wp)
		{
			alternate_wp = new_wp;
			sync_alternate_selector();
		}
		return true;
	}
	else
		return false;
}

bool ColorSubpalette::get_visible_square_under_pos(Position pos, int& index) const
{
	pos = local_of(pos) + 0.5f * ColorPalette::SQUARE_SEP * Position{ ColorPalette::COL_COUNT - 1, 1.0f - palette().row_count() };
	
	float min_x = -0.5f * ColorPalette::SQUARE_SIZE;
	float max_x = (ColorPalette::COL_COUNT - 1) * ColorPalette::SQUARE_SEP + 0.5f * ColorPalette::SQUARE_SIZE;
	if (pos.x < min_x || pos.x > max_x)
		return false;

	float max_y = 0.5f * ColorPalette::SQUARE_SIZE;
	float min_y = -(palette().row_count() - 1) * ColorPalette::SQUARE_SEP - 0.5f * ColorPalette::SQUARE_SIZE;
	if (pos.y < min_y || pos.y > max_y)
		return false;

	int ix = (pos.x + 0.5f * ColorPalette::SQUARE_SIZE) / ColorPalette::SQUARE_SEP;
	int iy = ceil_divide(pos.y + 0.5f * ColorPalette::SQUARE_SIZE, -ColorPalette::SQUARE_SEP); // TODO is ceil_divide(x, y) just equal to -(x / -y)?
	Position c{ ColorPalette::SQUARE_SEP * ix, -ColorPalette::SQUARE_SEP * iy };
	if (std::abs(pos.x - c.x) >= ColorPalette::SQUARE_SIZE || std::abs(pos.y - c.y) >= ColorPalette::SQUARE_SIZE)
		return false;

	int i = ix + iy * ColorPalette::COL_COUNT + first_square();
	if (i >= subscheme->get_colors().size())
		return false;

	index = i;
	return true;
}

void ColorSubpalette::switch_primary_and_alternate()
{
	std::swap(primary_index, alternate_index);
	resync_primary_selector();
	resync_alternate_selector();
	update_primary_color_in_picker();
}

void ColorSubpalette::update_primary_color_in_picker() const
{
	(*palette().primary_color_update)(subscheme->get_colors()[primary_index]);
}

void ColorSubpalette::scroll_by(int delta)
{
	int new_offset = std::clamp(scroll_offset + delta, 0, std::max(0, ceil_divide((int)subscheme->get_colors().size(), ColorPalette::COL_COUNT) - palette().row_count()));
	if (new_offset != scroll_offset)
	{
		scroll_offset = new_offset;
		sync_with_palette();
	}
}

void ColorSubpalette::scroll_to_view(int i)
{
	if (i < first_square())
	{
		// scroll up
		scroll_by(-ceil_divide(first_square() - i, ColorPalette::COL_COUNT));
	}
	else if (i >= first_square() + num_squares_visible())
	{
		// scroll down
		scroll_by(1 - ceil_divide(first_square() + num_squares_visible() - i, ColorPalette::COL_COUNT));
	}
}

void ColorSubpalette::scroll_down_full()
{
	int new_offset = std::max(0, ceil_divide((int)subscheme->get_colors().size(), ColorPalette::COL_COUNT) - palette().row_count());
	if (new_offset != scroll_offset)
	{
		scroll_offset = new_offset;
		sync_with_palette();
	}
}

WidgetPlacement ColorSubpalette::square_wp(int i) const
{
	Position initial_pos{ -ColorPalette::SQUARE_SEP * (ColorPalette::COL_COUNT - 1) * 0.5f, ColorPalette::SQUARE_SEP * (palette().row_count() - 1) * 0.5f };
	Position delta_pos{ ColorPalette::SQUARE_SEP * (i % ColorPalette::COL_COUNT), -ColorPalette::SQUARE_SEP * (i / ColorPalette::COL_COUNT - scroll_offset)};
	return WidgetPlacement{ { { initial_pos + delta_pos}, Scale(ColorPalette::SQUARE_SIZE) } }.relative_to(self.transform);
}

WidgetPlacement ColorSubpalette::global_square_wp(int i) const
{
	return square_wp(i).relative_to(parent->self.transform);
}

int ColorSubpalette::first_square() const
{
	return scroll_offset * ColorPalette::COL_COUNT;
}

int ColorSubpalette::num_squares_visible() const
{
	int leftover = (int)subscheme->get_colors().size() - first_square();
	return std::clamp(leftover, 0, ColorPalette::COL_COUNT * palette().row_count());
}

bool ColorSubpalette::is_square_visible(int i) const
{
	return i >= first_square() && i < first_square() + num_squares_visible();
}

Position ColorSubpalette::cursor_world_pos() const
{
	return Machine.cursor_world_pos(glm::inverse(*palette().vp));
}

void ColorSubpalette::override_current_color(RGBA color)
{
	if (color != subscheme->get_colors()[primary_index])
	{
		*subscheme->at(primary_index) = color;
		send_color(primary_index, color);
	}
}

void ColorSubpalette::new_color(RGBA color, bool adjacent, bool update_primary)
{
	if (adjacent)
	{
		subscheme->insert(color, primary_index + 1);
		if (alternate_index > primary_index)
		{
			++alternate_index;
			resync_alternate_selector();
		}

		IndexedRenderable& squares = ir_wget(*this, SQUARES);
		squares.insert_vertices(4, 4 * (primary_index + 1));
		squares.push_back_quads(1);
		setup_color_buffer(primary_index + 1, squares);

		size_t num_colors = subscheme->get_colors().size();
		for (size_t i = primary_index + 2; i < num_colors; ++i)
		{
			WidgetPlacement global = square_wp((int)i).relative_to(parent->self.transform);
			squares.set_attribute_single_vertex(i * 4 + 0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
			squares.set_attribute_single_vertex(i * 4 + 1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
			squares.set_attribute_single_vertex(i * 4 + 2, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
			squares.set_attribute_single_vertex(i * 4 + 3, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
		}

		squares.send_both_buffers_resized();

		if (update_primary)
		{
			++primary_index;
			resync_primary_selector();
		}
		scroll_to_view(primary_index);
	}
	else
	{
		subscheme->insert(color);

		IndexedRenderable& squares = ir_wget(*this, SQUARES);
		squares.push_back_vertices(4);
		squares.push_back_quads(1);
		setup_color_buffer(subscheme->get_colors().size() - 1, squares);

		squares.send_both_buffers_resized();

		if (update_primary)
		{
			primary_index = (int)subscheme->get_colors().size() - 1;
			resync_primary_selector();
			if (primary_index > first_square() + num_squares_visible())
				scroll_down_full();
		}
		else
			scroll_to_view(primary_index);
	}
}

void ColorSubpalette::send_color(size_t i, RGBA color)
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	glm::vec4 rgba = color.as_vec();
	squares.set_attribute_single_vertex(i * 4 + 0, 1, glm::value_ptr(rgba));
	squares.set_attribute_single_vertex(i * 4 + 1, 1, glm::value_ptr(rgba));
	squares.set_attribute_single_vertex(i * 4 + 2, 1, glm::value_ptr(rgba));
	squares.set_attribute_single_vertex(i * 4 + 3, 1, glm::value_ptr(rgba));
	squares.send_single_vertex(i * 4 + 0);
	squares.send_single_vertex(i * 4 + 1);
	squares.send_single_vertex(i * 4 + 2);
	squares.send_single_vertex(i * 4 + 3);
}

void ColorSubpalette::send_color(size_t i)
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	glm::vec4 rgba = subscheme->get_colors()[i].as_vec();
	squares.set_attribute_single_vertex(i * 4 + 0, 1, glm::value_ptr(rgba));
	squares.set_attribute_single_vertex(i * 4 + 1, 1, glm::value_ptr(rgba));
	squares.set_attribute_single_vertex(i * 4 + 2, 1, glm::value_ptr(rgba));
	squares.set_attribute_single_vertex(i * 4 + 3, 1, glm::value_ptr(rgba));
	squares.send_single_vertex(i * 4 + 0);
	squares.send_single_vertex(i * 4 + 1);
	squares.send_single_vertex(i * 4 + 2);
	squares.send_single_vertex(i * 4 + 3);
}

void ColorSubpalette::remove_square_under_cursor(bool send_vb)
{
	int index;
	if (!get_visible_square_under_pos(cursor_world_pos(), index))
		return;

	subscheme->remove(index);
	
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	size_t num_colors = subscheme->get_colors().size();
	for (size_t i = index; i < num_colors; ++i)
		setup_color_buffer(i, squares);

	if (send_vb)
		squares.send_vertex_buffer();

	if (primary_index >= subscheme->get_colors().size())
	{
		primary_index = (int)subscheme->get_colors().size() - 1;
		resync_primary_selector();
		update_primary_color_in_picker();
	}
	else if (index <= primary_index)
		update_primary_color_in_picker();
	if (alternate_index >= subscheme->get_colors().size())
	{
		alternate_index = (int)subscheme->get_colors().size() - 1;
		resync_alternate_selector();
	}
}

void ColorSubpalette::clean_extra_buffer_space()
{
	int end = (int)subscheme->get_colors().size();
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	if (squares.num_vertices() < end * 4)
		return;
	squares.remove_from_varr(end * 4);
	squares.remove_from_iarr(end * 6);
	squares.send_both_buffers_resized();
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
ColorPalette::ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler, ScrollHandler& parent_scroll_handler,
	const std::function<void(RGBA)>* primary_color_update, const std::function<RGBA()>* get_picker_rgba)
	: Widget(SUBPALETTE_START), vp(vp), parent_mb_handler(parent_mb_handler), parent_key_handler(parent_key_handler), parent_scroll_handler(parent_scroll_handler),
	grid_shader(FileSystem::shader_path("palette/black_grid.vert"), FileSystem::shader_path("palette/black_grid.frag")),
	color_square_shader(FileSystem::shader_path("palette/color_square.vert"), FileSystem::shader_path("palette/color_square.frag")),
	outline_rect_shader(FileSystem::shader_path("palette/outline_rect.vert"), FileSystem::shader_path("palette/outline_rect.frag")),
	round_rect_shader(FileSystem::shader_path("round_rect.vert"), FileSystem::shader_path("round_rect.frag")),
	primary_color_update(primary_color_update), get_picker_rgba(get_picker_rgba)
{
	initialize_widget();
	connect_input_handlers();
	new_subpalette(); // always have at least one subpalette

	Uniforms::send_1(grid_shader, "u_ColProportion", 1.0f / COL_COUNT);
	Uniforms::send_1(grid_shader, "u_RowProportion", 1.0f / row_count());
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
	current_subpalette().draw();
	ur_wget(*this, BLACK_GRID).draw();
	current_subpalette().draw_selectors();
	// LATER use StandardIButton instead of StandardTButton
	sb_t_wget(*this, BUTTON_OVERRIDE_COLOR).draw();
	sb_t_wget(*this, BUTTON_INSERT_NEW_COLOR).draw();
}

void ColorPalette::process()
{
	if (cursor_in_bkg())
	{
		current_subpalette().process();
		b_t_wget(*this, BUTTON_OVERRIDE_COLOR).process();
		b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).process();
	}
	else
	{
		b_t_wget(*this, BUTTON_OVERRIDE_COLOR).unhover();
		b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).unhover();
	}
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
	scheme.subschemes.push_back(std::make_shared<ColorSubscheme>());
	current_subscheme = num_subpalettes() - 1;
	current_subpalette().subscheme = scheme.subschemes[current_subscheme];
	current_subpalette().self.transform.position.y = grid_offset_y();
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
	mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (mb.action != IAction::RELEASE || !cursor_in_bkg())
			return;
		if (!(mb.mods & Mods::CONTROL))
		{
			if (mb.button == MouseButton::LEFT)
			{
				if (current_subpalette().check_primary())
				{
					mb.consumed = true;
					current_subpalette().update_primary_color_in_picker();
				}
			}
			else if (mb.button == MouseButton::RIGHT)
			{
				if (current_subpalette().check_alternate())
					mb.consumed = true;
			}
		}
		else if (mb.button == MouseButton::LEFT)
		{
			current_subpalette().remove_square_under_cursor(false); // false because of clean_extra_buffer_space() on next line 
			current_subpalette().clean_extra_buffer_space();
		}
		};
	parent_key_handler.children.push_back(&key_handler);
	key_handler.callback = [this](const KeyEvent& k) {
		if (k.action == IAction::RELEASE && k.key == Key::X)
		{
			k.consumed = true;
			current_subpalette().switch_primary_and_alternate();
		}
		};
	parent_scroll_handler.children.push_back(&scroll_handler);
	scroll_handler.callback = [this](const ScrollEvent& s) {
		if (cursor_in_bkg())
		{
			s.consumed = true;
			scroll_backlog -= s.yoff;
			float amount;
			scroll_backlog = modf(scroll_backlog, &amount);
			current_subpalette().scroll_by((int)amount);
		}
		};
}

void ColorPalette::initialize_widget()
{
	const float button1_x = -30;
	const float button_y = 170;

	assign_widget(this, BACKGROUND, new RoundRect(&round_rect_shader));
	rr_wget(*this, BACKGROUND).thickness = 0.25f;
	rr_wget(*this, BACKGROUND).corner_radius = 10;
	rr_wget(*this, BACKGROUND).border_color = RGBA(HSV(0.7f, 0.5f, 0.5f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).update_all();

	assign_widget(this, BLACK_GRID, new W_UnitRenderable(&grid_shader));

	StandardTButtonArgs sba(&mb_handler, &round_rect_shader, vp);
	sba.frange = Fonts::label_black;
	sba.font_size = 26;
	sba.pivot = { 0, 1 };
	sba.transform.scale = { 28, 28 };
	
	sba.transform.position = { button1_x, button_y };
	sba.text = "↓";
	sba.on_select = [this](StandardTButton&, const MouseButtonEvent& mb, Position) {
		current_subpalette().override_current_color((*get_picker_rgba)());
		};
	assign_widget(this, BUTTON_OVERRIDE_COLOR, new StandardTButton(sba));
	b_t_wget(*this, BUTTON_OVERRIDE_COLOR).text().self.transform.position = { 0.05f, -0.05f };
	b_t_wget(*this, BUTTON_OVERRIDE_COLOR).text().self.transform.scale *= 1.0f;

	sba.transform.position.x += sba.transform.scale.x;
	sba.text = "+";
	sba.on_select = [this](StandardTButton&, const MouseButtonEvent& mb, Position) {
		bool adjacent = !Machine.main_window->is_shift_pressed();
		bool update_primary = Machine.main_window->is_ctrl_pressed();
		current_subpalette().new_color((*get_picker_rgba)(), adjacent, update_primary);
		};
	assign_widget(this, BUTTON_INSERT_NEW_COLOR, new StandardTButton(sba));
	b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).text().self.transform.position = { 0.1f, -0.15f };
	b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).text().self.transform.scale *= 1.4f;
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
		b_t_wget(*this, BUTTON_OVERRIDE_COLOR).update_corner_radius(sc).update_thickness(sc);
		b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).update_corner_radius(sc).update_thickness(sc);
	}
	rr_wget(*this, BACKGROUND).update_transform().send_buffer();
	b_t_wget(*this, BUTTON_OVERRIDE_COLOR).send_vp();
	b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).send_vp();

	UnitRenderable& ur = ur_wget(*this, BLACK_GRID);
	WidgetPlacement global = grid_wp().relative_to(self.transform);

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

bool ColorPalette::cursor_in_bkg() const
{
	return children[BACKGROUND]->contains_screen_point(Machine.cursor_screen_pos(), *vp);
}
