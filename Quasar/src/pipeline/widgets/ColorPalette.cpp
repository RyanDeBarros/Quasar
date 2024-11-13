#include "ColorPalette.h"

#include <glm/gtc/type_ptr.inl>
#include <imgui/imgui_internal.h>

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
	if (subscheme->colors.empty())
		subscheme->colors.push_back(RGBA::WHITE);
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	squares.varr.clear();
	squares.iarr.clear();
	squares.set_num_vertices(subscheme->colors.size() * 4);
	squares.push_back_quads(subscheme->colors.size());
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
	size_t num_colors = subscheme->colors.size();
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
	glm::vec4 color = subscheme->colors[i].as_vec();
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
	if (!is_moving_a_color())
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
	else
	{
		if (moving_color_start)
		{
			moving_color_start = false;
			unprocess();
		}
		int target = -1;
		if (get_visible_square_under_pos(cursor_world_pos(), target) && target != moving_color)
		{
			if (move_in_1d)
			{
				if (target < moving_color)
				{
					std::rotate(subscheme->colors.begin() + target, subscheme->colors.begin() + moving_color, subscheme->colors.begin() + moving_color + 1);
					for (int i = target; i <= moving_color; ++i)
						send_color(i);
				}
				else
				{
					std::rotate(subscheme->colors.begin() + moving_color, subscheme->colors.begin() + moving_color + 1, subscheme->colors.begin() + target + 1);
					for (int i = moving_color; i <= target; ++i)
						send_color(i);
				}
			}
			else
			{
				std::swap(subscheme->colors[target], subscheme->colors[moving_color]);
				send_color(target);
				send_color(moving_color);
			}
			moving_color = target;
		}
	}
}

void ColorSubpalette::unprocess()
{
	hover_index = -1;
	hover_wp = { {}, Scale(-1) };
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
	pos = local_of(pos) + 0.5f * ColorPalette::SQUARE_SEP * Position(palette().col_count() - 1, 1 - palette().row_count());
	
	float min_x = -0.5f * ColorPalette::SQUARE_SIZE;
	float max_x = (palette().col_count() - 1) * ColorPalette::SQUARE_SEP + 0.5f * ColorPalette::SQUARE_SIZE;
	if (pos.x < min_x || pos.x > max_x)
		return false;

	float max_y = 0.5f * ColorPalette::SQUARE_SIZE;
	float min_y = -(palette().row_count() - 1) * ColorPalette::SQUARE_SEP - 0.5f * ColorPalette::SQUARE_SIZE;
	if (pos.y < min_y || pos.y > max_y)
		return false;

	int ix = (int)((pos.x + 0.5f * ColorPalette::SQUARE_SIZE) / ColorPalette::SQUARE_SEP);
	int iy = (int)ceilf((pos.y + 0.5f * ColorPalette::SQUARE_SIZE) / -ColorPalette::SQUARE_SEP);
	Position c{ ColorPalette::SQUARE_SEP * ix, -ColorPalette::SQUARE_SEP * iy };
	if (std::abs(pos.x - c.x) >= ColorPalette::SQUARE_SIZE || std::abs(pos.y - c.y) >= ColorPalette::SQUARE_SIZE)
		return false;

	int i = ix + iy * palette().col_count() + first_square();
	if (i >= subscheme->colors.size())
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
	(*palette().primary_color_update)(subscheme->colors[primary_index]);
}

void ColorSubpalette::scroll_by(int delta)
{
	int new_offset = std::clamp(scroll_offset + delta, 0, std::max(0, ceil_divide((int)subscheme->colors.size(), palette().col_count()) - palette().row_count()));
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
		scroll_by(-ceil_divide(first_square() - i, palette().col_count()));
	}
	else if (i >= first_square() + num_squares_visible())
	{
		// scroll down
		scroll_by(1 - ceil_divide(first_square() + num_squares_visible() - i, palette().col_count()));
	}
}

void ColorSubpalette::scroll_down_full()
{
	int new_offset = std::max(0, ceil_divide((int)subscheme->colors.size(), palette().col_count()) - palette().row_count());
	if (new_offset != scroll_offset)
	{
		scroll_offset = new_offset;
		sync_with_palette();
	}
}

WidgetPlacement ColorSubpalette::square_wp(int i) const
{
	Position initial_pos{ -ColorPalette::SQUARE_SEP * (palette().col_count() - 1) * 0.5f, ColorPalette::SQUARE_SEP * (palette().row_count() - 1) * 0.5f };
	Position delta_pos{ ColorPalette::SQUARE_SEP * (i % palette().col_count()), -ColorPalette::SQUARE_SEP * (i / palette().col_count() - scroll_offset)};
	return WidgetPlacement{ { { initial_pos + delta_pos}, Scale(ColorPalette::SQUARE_SIZE) } }.relative_to(self.transform);
}

WidgetPlacement ColorSubpalette::global_square_wp(int i) const
{
	return square_wp(i).relative_to(parent->self.transform);
}

int ColorSubpalette::first_square() const
{
	return scroll_offset * palette().col_count();
}

int ColorSubpalette::num_squares_visible() const
{
	int leftover = (int)subscheme->colors.size() - first_square();
	return std::clamp(leftover, 0, palette().col_count() * palette().row_count());
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
	if (color != subscheme->colors[primary_index])
	{
		subscheme->colors[primary_index] = color;
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

		size_t num_colors = subscheme->colors.size();
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
		subscheme->colors.push_back(color);

		IndexedRenderable& squares = ir_wget(*this, SQUARES);
		squares.push_back_vertices(4);
		squares.push_back_quads(1);
		setup_color_buffer(subscheme->colors.size() - 1, squares);

		squares.send_both_buffers_resized();

		if (update_primary)
		{
			primary_index = (int)subscheme->colors.size() - 1;
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
	glm::vec4 rgba = subscheme->colors[i].as_vec();
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
	size_t num_colors = subscheme->colors.size();
	for (size_t i = index; i < num_colors; ++i)
		setup_color_buffer(i, squares);

	if (send_vb)
		squares.send_vertex_buffer();

	if (primary_index >= subscheme->colors.size())
	{
		primary_index = (int)subscheme->colors.size() - 1;
		resync_primary_selector();
		update_primary_color_in_picker();
	}
	else if (index <= primary_index)
		update_primary_color_in_picker();
	if (alternate_index >= subscheme->colors.size())
	{
		alternate_index = (int)subscheme->colors.size() - 1;
		resync_alternate_selector();
	}
}

void ColorSubpalette::clean_extra_buffer_space()
{
	int end = (int)subscheme->colors.size();
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	if (squares.num_vertices() < end * 4)
		return;
	squares.remove_from_varr(end * 4);
	squares.remove_from_iarr(end * 6);
	squares.send_both_buffers_resized();
}

void ColorSubpalette::begin_moving_color_under_cursor(bool _1d)
{
	if (get_visible_square_under_pos(cursor_world_pos(), moving_color))
	{
		move_in_1d = _1d;
		moving_color_start = true;
	}
	else
		moving_color = -1;
}

void ColorSubpalette::stop_moving_color()
{
	moving_color = -1;
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

static const size_t BUTTONS[] = {
	ColorPalette::BUTTON_OVERRIDE_COLOR,
	ColorPalette::BUTTON_INSERT_NEW_COLOR,
	ColorPalette::BUTTON_SUBPALETTE_NEW,
	ColorPalette::BUTTON_SUBPALETTE_RENAME,
	ColorPalette::BUTTON_SUBPALETTE_DELETE
};

const float grid_padding_x1 = 10;
const float grid_padding_x2 = 10;
const float grid_padding_y1 = 100;
const float grid_padding_y2 = 10;

const float button1_x = -110;
const float button2_x = 25;
const float button_y = 55;

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
}

ColorPalette::~ColorPalette()
{
	parent_mb_handler.remove_child(&mb_handler);
	parent_key_handler.remove_child(&key_handler);
	parent_scroll_handler.remove_child(&scroll_handler);
}

void ColorPalette::draw()
{
	render_imgui();
	rr_wget(*this, BACKGROUND).draw();
	current_subpalette().draw();
	ur_wget(*this, BLACK_GRID).draw();
	current_subpalette().draw_selectors();
	// LATER use StandardIButton instead of StandardTButton
	for (size_t button : BUTTONS)
		sb_t_wget(*this, button).draw();
	if (imgui_editing || renaming_subpalette)
		rr_wget(*this, BACKGROUND).draw();
}

void ColorPalette::process()
{
	if (!imgui_editing && !renaming_subpalette && cursor_in_bkg())
	{
		current_subpalette().process();
		for (size_t button : BUTTONS)
			b_t_wget(*this, button).process();
	}
	else
	{
		current_subpalette().unprocess();
		for (size_t button : BUTTONS)
			b_t_wget(*this, button).unhover();
	}
}

void ColorPalette::send_vp()
{
	Uniforms::send_matrix3(color_square_shader, "u_VP", *vp);
	Uniforms::send_matrix3(grid_shader, "u_VP", grid_vp());
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
	// LATER import default subscheme here
	attach_widget(this, new ColorSubpalette(&color_square_shader, &outline_rect_shader));
	size_t last = num_subpalettes() - 1;
	scheme.subschemes.push_back(std::make_shared<ColorSubscheme>("default#" + std::to_string(last - 1)));
	switch_to_subpalette(last);
	current_subpalette().subscheme = scheme.subschemes[current_subscheme];
	current_subpalette().self.transform.position.y = subpalette_pos_y();
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

void ColorPalette::switch_to_subpalette(size_t pos)
{
	current_subscheme = pos;
}

size_t ColorPalette::num_subpalettes() const
{
	return children.size() - SUBPALETTE_START;
}

void ColorPalette::render_imgui()
{
	ImGui::SetNextWindowBgAlpha(0);
	ImGui::SetNextWindowPos({ gui_transform.position.x, gui_transform.position.y });
	ImGui::SetNextWindowSize({ gui_transform.scale.x, gui_transform.scale.y });

	// LATER put window setup in external function to be used across project
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoBackground;
	if (ImGui::Begin("##color-palette", nullptr, window_flags))
	{
		float font_window_scale = ImGui::GetCurrentWindow()->FontWindowScale;
		ImGui::SetWindowFontScale(scale1d() * font_window_scale);

		imgui_editing = false;

		ImGui::BeginDisabled(renaming_subpalette || current_subpalette().is_moving_a_color());

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::BeginCombo("##Subschemes", current_subpalette().subscheme->name.c_str()))
		{
			imgui_editing = true;
			size_t num_subs = num_subpalettes();
			for (int i = 0; i < num_subs; ++i)
			{
				if (ImGui::Selectable(get_subpalette(i).subscheme->name.c_str(), i == current_subscheme))
					switch_to_subpalette(i);
				if (i == current_subscheme)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::EndDisabled();

		if (renaming_subpalette)
		{
			if (renaming_subpalette_start)
			{
				renaming_subpalette_start = false;
				ImGui::SetKeyboardFocusHere();
				memset(rename_buf, '\0', ColorSubscheme::MAX_NAME_LENGTH);
				memcpy(rename_buf, current_subpalette().subscheme->name.c_str(), std::min(current_subpalette().subscheme->name.size(), ColorSubscheme::MAX_NAME_LENGTH));
			}
			ImGui::InputText("##rename-input", rename_buf, ColorSubscheme::MAX_NAME_LENGTH);
			if (ImGui::IsItemDeactivated())
			{
				renaming_subpalette = false;
				if (!Machine.main_window->is_key_pressed(Key::ESCAPE))
					current_subpalette().subscheme->name = rename_buf;
			}
		}

		ImGui::SetWindowFontScale(font_window_scale);
		ImGui::End();
	}
}

void ColorPalette::connect_input_handlers()
{
	parent_mb_handler.children.push_back(&mb_handler);
	mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (mb.action == IAction::RELEASE && mb.button != MouseButton::RIGHT && current_subpalette().is_moving_a_color())
			current_subpalette().stop_moving_color();
		if (imgui_editing || renaming_subpalette || !cursor_in_bkg())
			return;
		else if (mb.action == IAction::RELEASE && mb.button == MouseButton::LEFT && (mb.mods & Mods::CONTROL))
		{
			// delete color
			current_subpalette().remove_square_under_cursor(false); // false because of clean_extra_buffer_space() on next line 
			current_subpalette().clean_extra_buffer_space();
		}
		else if (mb.action == IAction::RELEASE && !Machine.main_window->is_key_pressed(Key::SPACE))
		{
			// select primary/alternate color
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
		else if (mb.action == IAction::PRESS && (mb.button == MouseButton::MIDDLE || (mb.button == MouseButton::LEFT && Machine.main_window->is_key_pressed(Key::SPACE))))
		{
			// move color without selecting
			current_subpalette().begin_moving_color_under_cursor(!(mb.mods & Mods::ALT));
		}
		};
	parent_key_handler.children.push_back(&key_handler);
	key_handler.callback = [this](const KeyEvent& k) {
		if (!imgui_editing && !renaming_subpalette && k.action == IAction::RELEASE && k.key == Key::X)
		{
			k.consumed = true;
			current_subpalette().switch_primary_and_alternate();
		}
		else if (renaming_subpalette && !imgui_editing && k.action == IAction::PRESS && k.key == Key::ESCAPE)
		{
			k.consumed = true;
			renaming_subpalette = false;
			renaming_subpalette_start = false;
		}
		};
	parent_scroll_handler.children.push_back(&scroll_handler);
	scroll_handler.callback = [this](const ScrollEvent& s) {
		if (!imgui_editing && !renaming_subpalette && cursor_in_bkg())
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
	sba.is_hoverable = [this]() { return !imgui_editing && !renaming_subpalette && !current_subpalette().is_moving_a_color(); };
	sba.is_selectable = fconv_st_check([this](const MouseButtonEvent& m) { return m.button == MouseButton::LEFT && !imgui_editing && !renaming_subpalette && !current_subpalette().is_moving_a_color(); });
	
	sba.transform.position = { button1_x, absolute_y_off_bkg_top(button_y) };
	sba.text = "↓";
	sba.on_select = fconv_st_on_action([this]() { current_subpalette().override_current_color((*get_picker_rgba)()); });
	assign_widget(this, BUTTON_OVERRIDE_COLOR, new StandardTButton(sba));
	b_t_wget(*this, BUTTON_OVERRIDE_COLOR).text().self.transform.position = { 0.05f, -0.05f };
	b_t_wget(*this, BUTTON_OVERRIDE_COLOR).text().self.transform.scale *= 1.0f;

	sba.transform.position.x += sba.transform.scale.x;
	sba.text = "+";
	sba.on_select = fconv_st_on_action([this]() {
		bool adjacent = !Machine.main_window->is_shift_pressed();
		bool update_primary = Machine.main_window->is_ctrl_pressed();
		current_subpalette().new_color((*get_picker_rgba)(), adjacent, update_primary);
		});
	assign_widget(this, BUTTON_INSERT_NEW_COLOR, new StandardTButton(sba));
	b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).text().self.transform.position = { 0.1f, -0.15f };
	b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).text().self.transform.scale *= 1.4f;

	// LATER use image buttons
	sba.frange = Fonts::label_regular;
	sba.font_size = 22;
	sba.transform.position.x = button2_x;
	sba.text = "N";
	sba.on_select = fconv_st_on_action([this]() { new_subpalette(); });
	assign_widget(this, BUTTON_SUBPALETTE_NEW, new StandardTButton(sba));
	
	sba.transform.position.x += sba.transform.scale.x;
	sba.text = "R";
	sba.on_select = fconv_st_on_action([this]() { renaming_subpalette = true; renaming_subpalette_start = true; });
	assign_widget(this, BUTTON_SUBPALETTE_RENAME, new StandardTButton(sba));
	
	sba.transform.position.x += sba.transform.scale.x;
	sba.text = "D";
	sba.on_select = fconv_st_on_action([this]() { delete_subpalette(current_subscheme); });
	assign_widget(this, BUTTON_SUBPALETTE_DELETE, new StandardTButton(sba));
}

void ColorPalette::import_color_scheme()
{
	size_t num_subs = num_subpalettes();
	if (scheme.subschemes.size() < num_subs)
	{
		for (size_t i = 0; i < scheme.subschemes.size(); ++i)
		{
			get_subpalette(i).subscheme = scheme.subschemes[i];
			get_subpalette(i).reload_subscheme();
		}
		for (size_t i = scheme.subschemes.size(); i < num_subs; ++i)
			delete children[subpalette_index_in_widget(i)];
		children.erase(children.begin() + subpalette_index_in_widget(scheme.subschemes.size()), children.end());
	}
	else
	{
		for (size_t i = 0; i < num_subs; ++i)
		{
			get_subpalette(i).subscheme = scheme.subschemes[i];
			get_subpalette(i).reload_subscheme();
		}
		for (size_t i = num_subs; i < scheme.subschemes.size(); ++i)
		{
			attach_widget(this, new ColorSubpalette(&color_square_shader, &outline_rect_shader));
			get_subpalette(i).subscheme = scheme.subschemes[i];
			get_subpalette(i).self.transform.position.y = subpalette_pos_y();
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
		for (size_t button : BUTTONS)
			b_t_wget(*this, button).update_corner_radius(sc).update_thickness(sc);
	}
	rr_wget(*this, BACKGROUND).update_transform().send_buffer();
	for (size_t button : BUTTONS)
		b_t_wget(*this, button).send_vp();

	UnitRenderable& ur = ur_wget(*this, BLACK_GRID);
	FlatTransform global = self.transform;
	global.scale *= Scale(_col_count * SQUARE_SEP, _row_count * SQUARE_SEP);

	ur.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
	ur.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
	ur.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
	ur.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
	ur.set_attribute_single_vertex(0, 1, glm::value_ptr(glm::vec2{ 0, 0 }));
	ur.set_attribute_single_vertex(1, 1, glm::value_ptr(glm::vec2{ 1, 0 }));
	ur.set_attribute_single_vertex(2, 1, glm::value_ptr(glm::vec2{ 0, 1 }));
	ur.set_attribute_single_vertex(3, 1, glm::value_ptr(glm::vec2{ 1, 1 }));
	ur.send_buffer();

	// LATER is this necessary, or should only current subpalette be selected, and when selecting a new subpalette, sync it then.
	for (size_t i = 0; i < num_subpalettes(); ++i)
		get_subpalette(i).sync_with_palette();

	gui_transform.scale = wp_at(BACKGROUND).transform.scale * Machine.get_app_scale() * self.transform.scale;
	gui_transform.position = Machine.to_screen_coordinates(children[BACKGROUND]->global_of({ -0.5f, 0.5f }), *vp);
	gui_transform.position.y = Machine.main_window->height() - gui_transform.position.y;
}

void ColorPalette::set_grid_metrics(int col, int row, bool sync)
{
	_col_count = col;
	_row_count = row;
	grid_offset_y = -wp_at(BACKGROUND).transform.scale.y + grid_padding_y1 + grid_padding_y2 + (row - 1) * SQUARE_SEP + SQUARE_SIZE;

	Uniforms::send_1(grid_shader, "u_ColProportion", 1.0f / col_count());
	Uniforms::send_1(grid_shader, "u_RowProportion", 1.0f / row_count());
	for (int i = 0; i < num_subpalettes(); ++i)
	{
		get_subpalette(i).self.transform.position.y = subpalette_pos_y();
		get_subpalette(i).scroll_by(0);
	}
	if (sync)
	{
		sync_widget_with_vp();
		Uniforms::send_matrix3(grid_shader, "u_VP", grid_vp());
	}
}

void ColorPalette::set_size(Scale size, bool sync)
{
	wp_at(BACKGROUND).transform.scale = size;
	rr_wget(*this, BACKGROUND).update_transform();
	
	for (size_t button : BUTTONS)
	{
		wp_at(button).transform.position.y = absolute_y_off_bkg_top(button_y);
		b_t_wget(*this, button).update_transform();
	}

	set_grid_metrics(std::max(1, (int)((size.x - SQUARE_SIZE - (grid_padding_x1 + grid_padding_x2)) / SQUARE_SEP) + 1),
		std::max(1, (int)((size.y - SQUARE_SIZE - (grid_padding_y1 + grid_padding_y2)) / SQUARE_SEP) + 1), sync);
}

Scale ColorPalette::minimum_display() const
{
	return Scale { grid_padding_x1 + grid_padding_x2 + 100, grid_padding_y1 + grid_padding_y2 + 50 };
}

bool ColorPalette::cursor_in_bkg() const
{
	return children[BACKGROUND]->contains_screen_point(Machine.cursor_screen_pos(), *vp);
}

glm::mat3 ColorPalette::grid_vp() const
{
	return (*vp) * FlatTransform { global_scale() * 0.5f * Position{ grid_padding_x2 - grid_padding_x1, -grid_offset_y + grid_padding_y2 - grid_padding_y1 } }.matrix();
}

float ColorPalette::subpalette_pos_y() const
{
	return (-grid_offset_y + grid_padding_y2 - grid_padding_y1) * 0.5f;
}

float ColorPalette::absolute_y_off_bkg_top(float y) const
{
	return 0.5f * wp_at(BACKGROUND).transform.scale.y - y;
}

float ColorPalette::absolute_y_off_bkg_bot(float y) const
{
	return -0.5f * wp_at(BACKGROUND).transform.scale.y + y;
}
