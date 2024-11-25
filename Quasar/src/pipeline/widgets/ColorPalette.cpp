#include "ColorPalette.h"

#include <glm/gtc/type_ptr.inl>
#include <imgui/imgui_internal.h>

#include "../render/Uniforms.h"
#include "RoundRect.h"
#include "user/Machine.h"
#include "Button.h"
#include "ColorPicker.h"

struct ColorOverwriteAction : public ActionBase
{
	std::shared_ptr<ColorSubpalette> subpalette;
	RGBA prev_c, new_c;
	ColorPicker::EditingColor editing_color;
	ColorOverwriteAction(std::shared_ptr<ColorSubpalette>&& subpalette, RGBA prev_c, RGBA new_c, ColorPicker::EditingColor editing_color)
		: subpalette(std::move(subpalette)), prev_c(prev_c), new_c(new_c), editing_color(editing_color) { weight = sizeof(ColorOverwriteAction); }
	virtual void forward() override { execute(new_c, true); }
	virtual void backward() override { execute(prev_c, false); }
	void execute(RGBA c, bool update_picker) const
	{
		subpalette->focus(true);
		if (editing_color == ColorPicker::EditingColor::PRIMARY)
			subpalette->overwrite_primary_color(c, update_picker);
		else if (editing_color == ColorPicker::EditingColor::ALTERNATE)
			subpalette->overwrite_alternate_color(c, update_picker);
	}
	QUASAR_ACTION_EQUALS_OVERRIDE(ColorOverwriteAction)
};

// LATER move these two to Utils.h
static size_t rotated_index(size_t first, size_t middle, size_t last, size_t index)
{
	if (first == middle || index < first || index > last)
		return index;
	else if (index >= first && index < middle)
		return index + (last - middle + 1);
	else
		return index - (middle - first);
}

static int rotated_index(int first, int middle, int last, int index)
{
	if (first == middle || index < first || index > last)
		return index;
	else if (index >= first && index < middle)
		return index + (last - middle + 1);
	else
		return index - (middle - first);
}

struct ColorMove1DAction : public ActionBase
{
	std::shared_ptr<ColorSubpalette> subpalette;
	int initial_index, target_index;
	ColorMove1DAction(std::shared_ptr<ColorSubpalette>&& subpalette, int initial_index, int target_index)
		: subpalette(std::move(subpalette)), initial_index(initial_index), target_index(target_index) { weight = sizeof(ColorMove1DAction); }
	virtual void forward() override { execute(initial_index, target_index); }
	virtual void backward() override { execute(target_index, initial_index); }
	void execute(int initial, int target) const
	{
		subpalette->focus(true);
		if (initial < target)
		{
			std::rotate(subpalette->subscheme->colors.begin() + initial, subpalette->subscheme->colors.begin() + initial + 1, subpalette->subscheme->colors.begin() + target + 1);
			for (int i = initial; i <= target; ++i)
				subpalette->send_color(i);

			subpalette->set_primary_selector(rotated_index(initial, initial + 1, target, subpalette->primary_index));
			subpalette->set_alternate_selector(rotated_index(initial, initial + 1, target, subpalette->alternate_index));
		}
		else if (initial > target)
		{
			std::rotate(subpalette->subscheme->colors.begin() + target, subpalette->subscheme->colors.begin() + initial, subpalette->subscheme->colors.begin() + initial + 1);
			for (int i = target; i <= initial; ++i)
				subpalette->send_color(i);

			subpalette->set_primary_selector(rotated_index(target, initial, initial, subpalette->primary_index));
			subpalette->set_alternate_selector(rotated_index(target, initial, initial, subpalette->alternate_index));
		}
	}
	QUASAR_ACTION_EQUALS_OVERRIDE(ColorMove1DAction)
};

struct ColorMove2DAction : public ActionBase
{
	std::shared_ptr<ColorSubpalette> subpalette;
	int initial_index, target_index;
	ColorMove2DAction(std::shared_ptr<ColorSubpalette>&& subpalette, int initial_index, int target_index)
		: subpalette(std::move(subpalette)), initial_index(initial_index), target_index(target_index) { weight = sizeof(ColorMove2DAction); }
	virtual void forward() override { execute(); }
	virtual void backward() override { execute(); }
	void execute() const
	{
		subpalette->focus(false);
		std::swap(subpalette->subscheme->colors[initial_index], subpalette->subscheme->colors[target_index]);
		subpalette->send_color(initial_index);
		subpalette->send_color(target_index);

		if (subpalette->primary_index == initial_index)
			subpalette->set_primary_selector(target_index);
		else if (subpalette->primary_index == target_index)
			subpalette->set_primary_selector(initial_index);
		if (subpalette->alternate_index == initial_index)
			subpalette->set_alternate_selector(target_index);
		else if (subpalette->alternate_index == target_index)
			subpalette->set_alternate_selector(initial_index);
	}
	QUASAR_ACTION_EQUALS_OVERRIDE(ColorMove2DAction)
};

struct InsertColorAction : public ActionBase
{
	std::shared_ptr<ColorSubpalette> subpalette;
	RGBA color;
	int index, primary_index_1, primary_index_2, alternate_index_1, alternate_index_2;
	InsertColorAction(std::shared_ptr<ColorSubpalette>&& subpalette, RGBA color, int index,
		int primary_index_1, int primary_index_2, int alternate_index_1, int alternate_index_2)
		: subpalette(std::move(subpalette)), color(color), index(index), primary_index_1(primary_index_1),
		primary_index_2(primary_index_2), alternate_index_1(alternate_index_1), alternate_index_2(alternate_index_2) { weight = sizeof(InsertColorAction); }
	virtual void forward() override { subpalette->focus(false); subpalette->insert_color_at(index, primary_index_2, alternate_index_2, color); }
	virtual void backward() override { subpalette->focus(false); subpalette->remove_color_at(index, primary_index_1, alternate_index_1, true); }
	QUASAR_ACTION_EQUALS_OVERRIDE(InsertColorAction)
};

struct RemoveColorAction : public ActionBase
{
	std::shared_ptr<ColorSubpalette> subpalette;
	RGBA color;
	int index, primary_index_1, primary_index_2, alternate_index_1, alternate_index_2;
	RemoveColorAction(std::shared_ptr<ColorSubpalette>&& subpalette, RGBA color, int index,
		int primary_index_1, int primary_index_2, int alternate_index_1, int alternate_index_2)
		: subpalette(std::move(subpalette)), color(color), index(index), primary_index_1(primary_index_1),
		primary_index_2(primary_index_2), alternate_index_1(alternate_index_1), alternate_index_2(alternate_index_2) { weight = sizeof(RemoveColorAction); }
	virtual void forward() override { subpalette->focus(false); subpalette->remove_color_at(index, primary_index_2, alternate_index_2, true); }
	virtual void backward() override { subpalette->focus(false); subpalette->insert_color_at(index, primary_index_1, alternate_index_1, color); }
	QUASAR_ACTION_EQUALS_OVERRIDE(RemoveColorAction)
};

struct SubpaletteRenameAction : public ActionBase
{
	std::shared_ptr<ColorSubpalette> subpalette;
	std::string name_combo;
	size_t divider;
	SubpaletteRenameAction(std::shared_ptr<ColorSubpalette>&& subpalette, std::string&& old_name, char* new_name)
		: subpalette(std::move(subpalette)), name_combo(std::move(old_name))
	{
		divider = name_combo.size();
		name_combo += new_name;
		weight = sizeof(SubpaletteRenameAction) + name_combo.size() * sizeof(char);
	}
	virtual void forward() override { execute(divider, -1); }
	virtual void backward() override { execute(0, divider); }
	void execute(size_t start, size_t len) const
	{
		subpalette->focus(true);
		subpalette->subscheme->name = std::move(name_combo.substr(start, len));
	}
	QUASAR_ACTION_EQUALS_OVERRIDE(SubpaletteRenameAction)
};

struct SubpaletteNewAction : public ActionBase
{
	ColorPalette* palette;
	std::shared_ptr<ColorSubpalette> subpalette;
	size_t index;

	SubpaletteNewAction(ColorPalette* palette, std::shared_ptr<ColorSubpalette>&& subpalette, size_t index)
		: palette(palette), subpalette(std::move(subpalette)), index(index) { weight = sizeof(SubpaletteNewAction) + sizeof(ColorSubpalette) + this->subpalette->subscheme->colors.size() * sizeof(RGBA); }

	virtual void forward() override { if (palette) palette->insert_subpalette(index, subpalette); }
	virtual void backward() override { if (palette) palette->delete_subpalette(index); }
	QUASAR_ACTION_EQUALS_OVERRIDE(SubpaletteNewAction)
};

struct SubpaletteDeleteAction : public ActionBase
{
	ColorPalette* palette;
	std::shared_ptr<ColorSubpalette> subpalette;
	size_t index;

	SubpaletteDeleteAction(ColorPalette* palette, std::shared_ptr<ColorSubpalette>&& subpalette, size_t index)
		: palette(palette), subpalette(std::move(subpalette)), index(index) { weight = sizeof(SubpaletteDeleteAction) + sizeof(ColorSubpalette) + this->subpalette->subscheme->colors.size() * sizeof(RGBA); }

	virtual void forward() override { if (palette) palette->delete_subpalette(index); }
	virtual void backward() override { if (palette) palette->insert_subpalette(index, subpalette); }
	QUASAR_ACTION_EQUALS_OVERRIDE(SubpaletteDeleteAction)
};

// LATER import subpalette files and test this action
struct AssignColorSubschemeAction : public ActionBase
{
	ColorPalette* palette;
	std::shared_ptr<ColorSubscheme> prev_subscheme, new_subscheme;
	size_t index;
	AssignColorSubschemeAction(ColorPalette* palette, std::shared_ptr<ColorSubscheme>&& prev_subscheme, const std::shared_ptr<ColorSubscheme>& new_subscheme, size_t index)
		: palette(palette), prev_subscheme(std::move(prev_subscheme)), new_subscheme(new_subscheme), index(index)
	{ weight = sizeof(AssignColorSubschemeAction) + 2 * sizeof(ColorSubscheme) + this->prev_subscheme->colors.size() * sizeof(RGBA) + this->new_subscheme->colors.size() * sizeof(RGBA); }

	virtual void forward() override { if (palette) palette->assign_color_subscheme(index, new_subscheme, false); }
	virtual void backward() override { if (palette) palette->assign_color_subscheme(index, prev_subscheme, false); }
	QUASAR_ACTION_EQUALS_OVERRIDE(AssignColorSubschemeAction)
};

// LATER test importing schemes and undo/redo. remember about ColorPalette::clear_history_on_import setting
struct ImportColorSchemeAction : public ActionBase
{
	ColorPalette* palette;
	std::shared_ptr<ColorScheme> prev_cs;
	std::shared_ptr<ColorScheme> new_cs;
	ImportColorSchemeAction(ColorPalette* palette, std::shared_ptr<ColorScheme>&& prev_cs, const std::shared_ptr<ColorScheme>& new_cs)
		: palette(palette), prev_cs(std::move(prev_cs)), new_cs(new_cs)
	{
		weight = sizeof(ImportColorSchemeAction) + 2 * sizeof(ColorScheme) + this->prev_cs->subschemes.size() * sizeof(ColorSubscheme) + this->new_cs->subschemes.size() * sizeof(ColorSubscheme);
		for (const auto& subscheme : this->prev_cs->subschemes)
			weight += subscheme->colors.size() * sizeof(RGBA);
		for (const auto& subscheme : this->new_cs->subschemes)
			weight += subscheme->colors.size() * sizeof(RGBA);
	}
	virtual void forward() override { if (palette) palette->import_color_scheme(new_cs, false); }
	virtual void backward() override { if (palette) palette->import_color_scheme(prev_cs, false); }
	QUASAR_ACTION_EQUALS_OVERRIDE(ImportColorSchemeAction)
};

ColorSubpalette::ColorSubpalette(Shader* color_square_shader, Shader* outline_rect_shader)
	: Widget(_W_COUNT)
{
	assign_widget(this, SQUARES, std::make_shared<W_IndexedRenderable>(color_square_shader));
	
	assign_widget(this, HOVER_SELECTOR, std::make_shared<W_UnitRenderable>(outline_rect_shader));
	ur_wget(*this, HOVER_SELECTOR)
		.set_attribute(1, glm::value_ptr(RGBA::WHITE.as_vec()))
		.set_attribute_single_vertex(0, 2, glm::value_ptr(glm::vec2{ 0, 0 }))
		.set_attribute_single_vertex(1, 2, glm::value_ptr(glm::vec2{ 1, 0 }))
		.set_attribute_single_vertex(2, 2, glm::value_ptr(glm::vec2{ 0, 1 }))
		.set_attribute_single_vertex(3, 2, glm::value_ptr(glm::vec2{ 1, 1 }))
		.send_buffer();

	assign_widget(this, PRIMARY_SELECTOR_B, std::make_shared<W_UnitRenderable>(color_square_shader, 3));
	ur_wget(*this, PRIMARY_SELECTOR_B).set_attribute(1, glm::value_ptr(RGBA::BLACK.as_vec())).send_buffer();

	assign_widget(this, PRIMARY_SELECTOR_F, std::make_shared<W_UnitRenderable>(color_square_shader, 3));
	ur_wget(*this, PRIMARY_SELECTOR_F).set_attribute(1, glm::value_ptr(RGBA::WHITE.as_vec())).send_buffer();

	assign_widget(this, ALTERNATE_SELECTOR_B, std::make_shared<W_UnitRenderable>(color_square_shader, 3));
	ur_wget(*this, ALTERNATE_SELECTOR_B).set_attribute(1, glm::value_ptr(RGBA::BLACK.as_vec())).send_buffer();

	assign_widget(this, ALTERNATE_SELECTOR_F, std::make_shared<W_UnitRenderable>(color_square_shader, 3));
	ur_wget(*this, ALTERNATE_SELECTOR_F).set_attribute(1, glm::value_ptr(RGBA::WHITE.as_vec())).send_buffer();
}

ColorPalette& ColorSubpalette::palette()
{
	return *dynamic_cast<ColorPalette*>(parent);
}

const ColorPalette& ColorSubpalette::palette() const
{
	return *dynamic_cast<const ColorPalette*>(parent);
}

void ColorSubpalette::reload_subscheme(bool update_picker_colors)
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
	if (update_picker_colors)
	{
		update_primary_color_in_picker();
		update_alternate_color_in_picker();
	}
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
	// LATER create utility for this somewhere, since this wp calculation for setting vertices is done everywhere. new file? WidgetUtility.h
	ur_wget(*this, HOVER_SELECTOR)
		.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ hover_wp.left(), hover_wp.bottom() }))
		.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ hover_wp.right(), hover_wp.bottom() }))
		.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ hover_wp.left(), hover_wp.top() }))
		.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ hover_wp.right(), hover_wp.top() }))
		.send_buffer();
}

const float tri_first_offset = 3;
const float tri_last_offset = 15;
const float tri_second_offset = tri_last_offset - tri_first_offset * (1 + glm::sqrt(2.0f));

void ColorSubpalette::sync_primary_selector()
{
	Scale sc = global_scale();

	ur_wget(*this, PRIMARY_SELECTOR_B)
		.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ primary_wp.left(), primary_wp.top() }))
		.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_last_offset * sc.x, primary_wp.top()}))
		.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ primary_wp.left(), primary_wp.top() - tri_last_offset * sc.y }))
		.send_buffer();

	ur_wget(*this, PRIMARY_SELECTOR_F)
		.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_first_offset * sc.x, primary_wp.top() - tri_first_offset * sc.y }))
		.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_second_offset * sc.x, primary_wp.top() - tri_first_offset * sc.y }))
		.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ primary_wp.left() + tri_first_offset * sc.x, primary_wp.top() - tri_second_offset * sc.y }))
		.send_buffer();
}

void ColorSubpalette::resync_primary_selector()
{
	primary_wp = square_wp(primary_index).relative_to(parent->self.transform);
	sync_primary_selector();
}

void ColorSubpalette::sync_alternate_selector()
{
	Scale sc = global_scale();

	ur_wget(*this, ALTERNATE_SELECTOR_B)
		.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ alternate_wp.right(), alternate_wp.top() }))
		.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_last_offset * sc.x, alternate_wp.top() }))
		.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ alternate_wp.right(), alternate_wp.top() - tri_last_offset * sc.y }))
		.send_buffer();

	ur_wget(*this, ALTERNATE_SELECTOR_F)
		.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_first_offset * sc.x, alternate_wp.top() - tri_first_offset * sc.y }))
		.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_second_offset * sc.x, alternate_wp.top() - tri_first_offset * sc.y }))
		.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ alternate_wp.right() - tri_first_offset * sc.x, alternate_wp.top() - tri_second_offset * sc.y }))
		.send_buffer();
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
	glm::vec4 color = subscheme->colors[i].as_vec();
	squares.set_attribute_single_vertex(i * 4 + 0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
	squares.set_attribute_single_vertex(i * 4 + 1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
	squares.set_attribute_single_vertex(i * 4 + 2, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
	squares.set_attribute_single_vertex(i * 4 + 3, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
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
	if (is_moving_a_color())
		move_color();
	else
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

void ColorSubpalette::set_primary_selector(int index)
{
	if (primary_index != index)
	{
		primary_index = index;
		WidgetPlacement new_wp = global_square_wp(primary_index);
		if (primary_wp != new_wp)
		{
			primary_wp = new_wp;
			sync_primary_selector();
		}
	}
}

void ColorSubpalette::set_alternate_selector(int index)
{
	if (alternate_index != index)
	{
		alternate_index = index;
		WidgetPlacement new_wp = global_square_wp(alternate_index);
		if (alternate_wp != new_wp)
		{
			alternate_wp = new_wp;
			sync_alternate_selector();
		}
	}
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
	(*palette().reflection.swap_picker_colors)();
}

void ColorSubpalette::update_primary_color_in_picker() const
{
	if (&palette().current_subpalette() == this)
		(*palette().reflection.pri_color_update)(subscheme->colors[primary_index]);
}

void ColorSubpalette::update_alternate_color_in_picker() const
{
	if (&palette().current_subpalette() == this)
		(*palette().reflection.alt_color_update)(subscheme->colors[alternate_index]);
}

void ColorSubpalette::focus(bool update_gfx)
{
	if (&palette().current_subpalette() != this)
		palette().switch_to_subpalette(this, update_gfx);
}

void ColorSubpalette::cycle_primary_through(int delta)
{
	primary_index = std::clamp(primary_index + delta, 0, (int)subscheme->colors.size() - 1);
	resync_primary_selector();
	scroll_to_view(primary_index);
	update_primary_color_in_picker();
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

void ColorSubpalette::overwrite_primary_color(RGBA color, bool update_in_picker)
{
	if (color != subscheme->colors[primary_index])
	{
		subscheme->colors[primary_index] = color;
		send_color(primary_index, color);
		if (update_in_picker)
			update_primary_color_in_picker();
	}
}

void ColorSubpalette::overwrite_alternate_color(RGBA color, bool update_in_picker)
{
	if (color != subscheme->colors[alternate_index])
	{
		subscheme->colors[alternate_index] = color;
		send_color(alternate_index, color);
		if (update_in_picker)
			update_alternate_color_in_picker();
	}
}

void ColorSubpalette::new_color_from_primary(RGBA color, bool adjacent, bool update_primary, bool create_action)
{
	int index = int(adjacent ? primary_index + 1 : subscheme->colors.size());
	int initial_primary_index = primary_index;
	int initial_alternate_index = alternate_index;

	insert_color_setup(color, index);
	if (adjacent && alternate_index >= index)
	{
		++alternate_index;
		resync_alternate_selector();
	}
	send_inserted_color(color, index, adjacent);
	if (update_primary)
	{
		primary_index = index;
		resync_primary_selector();
	}
	scroll_to_view(primary_index);

	if (create_action)
		Machine.history.push(std::make_shared<InsertColorAction>(palette().subpalette_ref(this), color, index, initial_primary_index, primary_index, initial_alternate_index, alternate_index));
}

void ColorSubpalette::new_color_from_alternate(RGBA color, bool adjacent, bool update_alternate, bool create_action)
{
	int index = int(adjacent ? alternate_index + 1 : subscheme->colors.size());
	int initial_primary_index = primary_index;
	int initial_alternate_index = alternate_index;

	insert_color_setup(color, index);
	if (adjacent && primary_index >= index)
	{
		++primary_index;
		resync_primary_selector();
	}
	send_inserted_color(color, index, adjacent);
	if (update_alternate)
	{
		alternate_index = index;
		resync_alternate_selector();
	}
	scroll_to_view(primary_index);

	if (create_action)
		Machine.history.push(std::make_shared<InsertColorAction>(palette().subpalette_ref(this), color, index, initial_primary_index, primary_index, initial_alternate_index, alternate_index));
}

void ColorSubpalette::insert_color_setup(RGBA color, int index)
{
	subscheme->insert(color, index);
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	squares.insert_vertices(4, 4 * index);
	squares.push_back_quads(1);
	setup_color_buffer(index, squares);
}

void ColorSubpalette::send_inserted_color(RGBA color, int index, bool adjacent)
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	
	if (adjacent)
	{
		size_t num_colors = subscheme->colors.size();
		for (size_t i = index + 1; i < num_colors; ++i)
		{
			WidgetPlacement global = square_wp((int)i).relative_to(parent->self.transform);
			squares.set_attribute_single_vertex(i * 4 + 0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
			squares.set_attribute_single_vertex(i * 4 + 1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
			squares.set_attribute_single_vertex(i * 4 + 2, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
			squares.set_attribute_single_vertex(i * 4 + 3, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
		}
	}
	
	squares.send_both_buffers_resized();
}

void ColorSubpalette::send_color(size_t i, RGBA color)
{
	glm::vec4 rgba = color.as_vec();
	ir_wget(*this, SQUARES)
		.set_attribute_single_vertex(i * 4 + 0, 1, glm::value_ptr(rgba))
		.set_attribute_single_vertex(i * 4 + 1, 1, glm::value_ptr(rgba))
		.set_attribute_single_vertex(i * 4 + 2, 1, glm::value_ptr(rgba))
		.set_attribute_single_vertex(i * 4 + 3, 1, glm::value_ptr(rgba))
		.send_single_vertex(i * 4 + 0)
		.send_single_vertex(i * 4 + 1)
		.send_single_vertex(i * 4 + 2)
		.send_single_vertex(i * 4 + 3);
}

void ColorSubpalette::send_color(size_t i)
{
	glm::vec4 rgba = subscheme->colors[i].as_vec();
	ir_wget(*this, SQUARES)
		.set_attribute_single_vertex(i * 4 + 0, 1, glm::value_ptr(rgba))
		.set_attribute_single_vertex(i * 4 + 1, 1, glm::value_ptr(rgba))
		.set_attribute_single_vertex(i * 4 + 2, 1, glm::value_ptr(rgba))
		.set_attribute_single_vertex(i * 4 + 3, 1, glm::value_ptr(rgba))
		.send_single_vertex(i * 4 + 0)
		.send_single_vertex(i * 4 + 1)
		.send_single_vertex(i * 4 + 2)
		.send_single_vertex(i * 4 + 3);
}

void ColorSubpalette::remove_square_under_cursor(bool send_vb, bool create_action)
{
	int index;
	if (!get_visible_square_under_pos(cursor_world_pos(), index))
		return;
	remove_square_under_index(index, send_vb, create_action);
}

void ColorSubpalette::remove_square_under_index(int index, bool send_vb, bool create_action)
{
	int initial_primary = primary_index;
	int initial_alternate = alternate_index;
	RGBA color = subscheme->colors[index];

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
	}
	else if (index < primary_index)
	{
		--primary_index;
		resync_primary_selector();
	}
	if (alternate_index >= subscheme->colors.size())
	{
		alternate_index = (int)subscheme->colors.size() - 1;
		resync_alternate_selector();
	}
	else if (index < alternate_index)
	{
		--alternate_index;
		resync_alternate_selector();
	}

	if (create_action)
		Machine.history.push(std::make_shared<RemoveColorAction>(palette().subpalette_ref(this), color, index, initial_primary, primary_index, initial_alternate, alternate_index));
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

void ColorSubpalette::insert_color_at(int index, int to_primary_index, int to_alternate_index, RGBA color)
{
	subscheme->insert(color, index);

	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	squares.insert_vertices(4, 4 * index);
	squares.push_back_quads(1);
	setup_color_buffer(index, squares);

	size_t num_colors = subscheme->colors.size();
	for (int i = index + 1; i < num_colors; ++i)
	{
		WidgetPlacement global = square_wp((int)i).relative_to(parent->self.transform);
		squares.set_attribute_single_vertex(i * 4 + 0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }));
		squares.set_attribute_single_vertex(i * 4 + 1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }));
		squares.set_attribute_single_vertex(i * 4 + 2, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }));
		squares.set_attribute_single_vertex(i * 4 + 3, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }));
	}

	squares.send_both_buffers_resized();
	
	if (to_primary_index != primary_index)
	{
		primary_index = to_primary_index;
		resync_primary_selector();
	}
	if (to_alternate_index != alternate_index)
	{
		alternate_index = to_alternate_index;
		resync_alternate_selector();
	}
	
	scroll_to_view(primary_index);
}

void ColorSubpalette::remove_color_at(int index, int to_primary_index, int to_alternate_index, bool send_vb)
{
	subscheme->remove(index);

	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	size_t num_colors = subscheme->colors.size();
	for (size_t i = index; i < num_colors; ++i)
		setup_color_buffer(i, squares);

	if (send_vb)
		squares.send_vertex_buffer();

	if (primary_index != to_primary_index)
	{
		primary_index = to_primary_index;
		resync_primary_selector();
	}
	if (alternate_index != to_alternate_index)
	{
		alternate_index = to_alternate_index;
		resync_alternate_selector();
	}
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

void ColorSubpalette::move_color()
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
			Machine.history.execute(std::make_shared<ColorMove1DAction>(palette().subpalette_ref(this), moving_color, target));
		else
			Machine.history.execute(std::make_shared<ColorMove2DAction>(palette().subpalette_ref(this), moving_color, target));
		moving_color = target;
	}
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

std::shared_ptr<ColorSubpalette> ColorPalette::subpalette_ref(ColorSubpalette* subpalette) const
{
	size_t num = num_subpalettes();
	for (size_t i = 0; i < num; ++i)
		if (children[subpalette_index_in_widget(i)].get() == subpalette)
			return std::dynamic_pointer_cast<ColorSubpalette>(children[subpalette_index_in_widget(i)]);
	return nullptr;
}

std::shared_ptr<ColorSubpalette> ColorPalette::subpalette_ref(size_t pos) const
{
	return std::dynamic_pointer_cast<ColorSubpalette>(children[subpalette_index_in_widget(pos)]);
}

std::shared_ptr<ColorSubpalette> ColorPalette::current_subpalette_ref() const
{
	return std::dynamic_pointer_cast<ColorSubpalette>(children[subpalette_index_in_widget(current_subscheme)]);
}

size_t ColorPalette::subpalette_index_in_widget(size_t pos) const
{
	return SUBPALETTE_START + pos;
}

static const size_t BUTTONS[] = {
	ColorPalette::BUTTON_OVERWRITE_COLOR,
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

ColorPalette::ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler, ScrollHandler& parent_scroll_handler, const Reflection& reflection)
	: Widget(SUBPALETTE_START), vp(vp), parent_mb_handler(parent_mb_handler), parent_key_handler(parent_key_handler), parent_scroll_handler(parent_scroll_handler),
	grid_shader(FileSystem::shader_path("palette/black_grid.vert"), FileSystem::shader_path("palette/black_grid.frag")),
	color_square_shader(FileSystem::shader_path("color_square.vert"), FileSystem::shader_path("color_square.frag")),
	outline_rect_shader(FileSystem::shader_path("palette/outline_rect.vert"), FileSystem::shader_path("palette/outline_rect.frag")),
	round_rect_shader(FileSystem::shader_path("round_rect.vert"), FileSystem::shader_path("round_rect.frag")), reflection(reflection), scheme(std::make_shared<ColorScheme>())
{
	initialize_widget();
	connect_input_handlers();
	new_subpalette(false); // always have at least one subpalette
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
	if (imgui_combo_open || renaming_subpalette)
		rr_wget(*this, BACKGROUND).draw();
}

void ColorPalette::process()
{
	if (!imgui_combo_open && !renaming_subpalette && cursor_in_bkg())
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

void ColorPalette::import_color_scheme(const std::shared_ptr<ColorScheme>& color_scheme, bool create_action)
{
	if (create_action)
	{
		if (clear_history_on_import)
			Machine.history.clear_history();
		else
			Machine.history.push(std::make_shared<ImportColorSchemeAction>(this, std::move(scheme), color_scheme));
	}
	scheme = color_scheme;
	import_color_scheme();
}

void ColorPalette::import_color_scheme(std::shared_ptr<ColorScheme>&& color_scheme, bool create_action)
{
	if (create_action)
	{
		if (clear_history_on_import)
			Machine.history.clear_history();
		else
			Machine.history.push(std::make_shared<ImportColorSchemeAction>(this, std::move(scheme), color_scheme));
	}
	scheme = std::move(color_scheme);
	import_color_scheme();
}

void ColorPalette::assign_color_subscheme(size_t pos, const std::shared_ptr<ColorSubscheme>& subscheme, bool create_action)
{
	if (pos >= num_subpalettes())
		return;
	switch_to_subpalette(pos, false);
	std::shared_ptr<ColorSubscheme> prev_subscheme = current_subpalette().subscheme;
	scheme->subschemes[pos] = subscheme;
	current_subpalette().subscheme = subscheme;
	current_subpalette().reload_subscheme(true);
	if (create_action)
		Machine.history.push(std::make_shared<AssignColorSubschemeAction>(this, std::move(prev_subscheme), subscheme, pos));
}

void ColorPalette::assign_color_subscheme(size_t pos, std::shared_ptr<ColorSubscheme>&& subscheme, bool create_action)
{
	if (pos >= num_subpalettes())
		return;
	switch_to_subpalette(pos, false);
	std::shared_ptr<ColorSubscheme> prev_subscheme = current_subpalette().subscheme;
	scheme->subschemes[pos] = std::move(subscheme);
	current_subpalette().subscheme = scheme->subschemes[pos];
	current_subpalette().reload_subscheme(true);
	if (create_action)
		Machine.history.push(std::make_shared<AssignColorSubschemeAction>(this, std::move(prev_subscheme), scheme->subschemes[pos], pos));
}

void ColorPalette::insert_subpalette(size_t pos, const std::shared_ptr<ColorSubpalette>& subpalette)
{
	insert_widget(this, subpalette_index_in_widget(pos), subpalette);
	scheme->subschemes.insert(scheme->subschemes.begin() + pos, subpalette->subscheme);
	switch_to_subpalette(pos, false);
	current_subpalette().self.transform.position.y = subpalette_pos_y();
	current_subpalette().sync_with_palette();
	current_subpalette().update_primary_color_in_picker();
	current_subpalette().update_alternate_color_in_picker();
}

void ColorPalette::insert_subpalette(size_t pos, std::shared_ptr<ColorSubpalette>&& subpalette)
{
	std::shared_ptr<ColorSubscheme> subscheme = subpalette->subscheme;
	insert_widget(this, subpalette_index_in_widget(pos), std::move(subpalette));
	scheme->subschemes.insert(scheme->subschemes.begin() + pos, subscheme);
	switch_to_subpalette(pos, false);
	current_subpalette().self.transform.position.y = subpalette_pos_y();
	current_subpalette().sync_with_palette();
	current_subpalette().update_primary_color_in_picker();
	current_subpalette().update_alternate_color_in_picker();
}

void ColorPalette::new_subpalette(bool create_action)
{
	attach_widget(this, std::make_shared<ColorSubpalette>(&color_square_shader, &outline_rect_shader));
	size_t last = num_subpalettes() - 1;
	scheme->subschemes.push_back(std::make_shared<ColorSubscheme>("default#" + std::to_string(last)));
	switch_to_subpalette(last, false);
	current_subpalette().subscheme = scheme->subschemes[current_subscheme];
	current_subpalette().self.transform.position.y = subpalette_pos_y();
	current_subpalette().reload_subscheme(true);
	if (create_action)
		Machine.history.push(std::make_shared<SubpaletteNewAction>(this, current_subpalette_ref(), current_subscheme));
}

void ColorPalette::rename_subpalette()
{
	if (!renaming_subpalette)
	{
		renaming_subpalette = true;
		renaming_subpalette_start = true;
	}
}

void ColorPalette::delete_subpalette(size_t pos)
{
	if (pos >= num_subpalettes())
		return;
	detach_widget(this, SUBPALETTE_START + pos);
	scheme->subschemes.erase(scheme->subschemes.begin() + pos);
	if (num_subpalettes() == 0)
		new_subpalette(false);
	if (current_subscheme >= num_subpalettes())
		switch_to_subpalette(num_subpalettes() - 1, true);
}

void ColorPalette::delete_current_subpalette(bool create_action)
{
	if (create_action)
	{
		std::shared_ptr<ColorSubpalette> subpalette = current_subpalette_ref();
		size_t index = current_subscheme;
		delete_subpalette(current_subscheme);
		Machine.history.push(std::make_shared<SubpaletteDeleteAction>(this, std::move(subpalette), index));
	}
	else
		delete_subpalette(current_subscheme);
		
}

void ColorPalette::switch_to_subpalette(size_t pos, bool update_gfx)
{
	if (current_subscheme != pos && pos < num_subpalettes())
	{
		current_subscheme = pos;
		if (update_gfx)
		{
			current_subpalette().update_primary_color_in_picker();
			current_subpalette().update_alternate_color_in_picker();
		}
	}
}

void ColorPalette::switch_to_subpalette(ColorSubpalette* subpalette, bool update_gfx)
{
	size_t num = num_subpalettes();
	for (size_t i = 0; i < num; ++i)
	{
		if (children[subpalette_index_in_widget(i)].get() == subpalette)
		{
			current_subscheme = i;
			if (update_gfx)
			{
				current_subpalette().update_primary_color_in_picker();
				current_subpalette().update_alternate_color_in_picker();
			}
			return;
		}
	}
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

		imgui_combo_open = false;

		ImGui::BeginDisabled(renaming_subpalette || current_subpalette().is_moving_a_color());

		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::BeginCombo("##Subschemes", current_subpalette().subscheme->name.c_str(), ImGuiComboFlags_HeightLarge))
		{
			if (escape_to_close_popup)
			{
				escape_to_close_popup = false;
				ImGui::CloseCurrentPopup();
			}
			else
				imgui_combo_open = true;
			int num_subs = (int)num_subpalettes();
			for (int i = num_subs - 1; i >= 0; --i)
			{
				if (ImGui::Selectable(get_subpalette(i).subscheme->name.c_str(), i == current_subscheme))
					switch_to_subpalette(i, true);
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
			ImGui::InputText("##rename-input", rename_buf, ColorSubscheme::MAX_NAME_LENGTH); // LATER ImGuiInputTextFlags_AutoSelectAll ? to auto select text on focus
			if (ImGui::IsItemDeactivated())
			{
				renaming_subpalette = false;
				if (!MainWindow->is_key_pressed(Key::ESCAPE))
				{
					if (current_subpalette().subscheme->name != rename_buf)
					{
						std::string old_name = current_subpalette().subscheme->name;
						current_subpalette().subscheme->name = rename_buf;
						Machine.history.push(std::make_shared<SubpaletteRenameAction>(current_subpalette_ref(), std::move(old_name), rename_buf));
					}
				}
			}
		}

		ImGui::SetWindowFontScale(font_window_scale);
		ImGui::End();
	}
}

void ColorPalette::connect_input_handlers()
{
	parent_mb_handler.add_child(&mb_handler);
	mb_handler.callback = [this](const MouseButtonEvent& mb) {
		if (mb.action == IAction::RELEASE && mb.button != MouseButton::RIGHT && current_subpalette().is_moving_a_color())
			current_subpalette().stop_moving_color();
		if (imgui_combo_open || renaming_subpalette || !cursor_in_bkg())
			return;
		else if (mb.action == IAction::RELEASE && mb.button == MouseButton::LEFT && (mb.mods & Mods::CONTROL))
		{
			// delete color
			current_subpalette().remove_square_under_cursor(false, true); // false for first parameter because of clean_extra_buffer_space() on next line 
			current_subpalette().clean_extra_buffer_space();
		}
		else if (mb.action == IAction::RELEASE && !MainWindow->is_key_pressed(Key::SPACE))
		{
			// select primary/alternate color
			if (mb.button == MouseButton::LEFT)
			{
				int prev_i = current_subpalette().primary_index;
				if (current_subpalette().check_primary())
				{
					mb.consumed = true;
					current_subpalette().update_primary_color_in_picker();
				}
			}
			else if (mb.button == MouseButton::RIGHT)
			{
				int prev_i = current_subpalette().alternate_index;
				if (current_subpalette().check_alternate())
				{
					mb.consumed = true;
					current_subpalette().update_alternate_color_in_picker();
				}
			}
		}
		else if (mb.action == IAction::PRESS && (mb.button == MouseButton::MIDDLE || (mb.button == MouseButton::LEFT && MainWindow->is_key_pressed(Key::SPACE))))
		{
			// move color without selecting
			current_subpalette().begin_moving_color_under_cursor(!(mb.mods & Mods::ALT));
		}
		};
	parent_key_handler.add_child(&key_handler);
	key_handler.callback = [this](const KeyEvent& k) {
		if (renaming_subpalette)
		{
			if (!imgui_combo_open && k.action == IAction::PRESS && k.key == Key::ESCAPE)
			{
				k.consumed = true;
				renaming_subpalette = false;
				renaming_subpalette_start = false;
				return;
			}
		}
		else if (k.action == IAction::PRESS)
		{
			if (imgui_combo_open)
			{
				if (k.key == Key::ESCAPE)
				{
					k.consumed = true;
					escape_to_close_popup = true;
					return;
				}
			}
			else
			{
				if (k.key == Key::X)
				{
					k.consumed = true;
					current_subpalette().switch_primary_and_alternate();
					return;
				}
			}
			if (k.mods & Mods::CONTROL)
			{
				if (k.key == Key::UP)
				{
					k.consumed = true;
					switch_to_subpalette(current_subscheme + 1, true);
					return;
				}
				else if (k.key == Key::DOWN)
				{
					k.consumed = true;
					switch_to_subpalette(current_subscheme - 1, true);
					return;
				}
			}

		}
		};
	parent_scroll_handler.add_child(&scroll_handler);
	scroll_handler.callback = [this](const ScrollEvent& s) {
		if (!imgui_combo_open && !renaming_subpalette)
		{
			if (MainWindow->is_alt_pressed())
			{
				s.consumed = true;
				cycle_backlog -= s.yoff;
				float amount;
				cycle_backlog = modf(cycle_backlog, &amount);
				current_subpalette().cycle_primary_through((int)amount);
			}
			else
			{
				if (cursor_in_bkg())
				{
					s.consumed = true;
					scroll_backlog -= s.yoff;
					float amount;
					scroll_backlog = modf(scroll_backlog, &amount);
					current_subpalette().scroll_by((int)amount);
				}
			}
		}
		};
}

void ColorPalette::initialize_widget()
{
	assign_widget(this, BACKGROUND, std::make_shared<RoundRect>(&round_rect_shader));
	rr_wget(*this, BACKGROUND).thickness = 0.25f;
	rr_wget(*this, BACKGROUND).corner_radius = 10;
	rr_wget(*this, BACKGROUND).border_color = RGBA(HSV(0.7f, 0.5f, 0.5f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).fill_color = RGBA(HSV(0.7f, 0.3f, 0.3f).to_rgb(), 0.5f);
	rr_wget(*this, BACKGROUND).update_all();

	assign_widget(this, BLACK_GRID, std::make_shared<W_UnitRenderable>(&grid_shader));

	StandardTButtonArgs sba(&mb_handler, &round_rect_shader, vp);
	sba.frange = Fonts::label_black;
	sba.font_size = 26;
	sba.pivot = { 0, 1 };
	sba.transform.scale = { 28, 28 };
	sba.is_hoverable = [this]() { return !imgui_combo_open && !renaming_subpalette && !current_subpalette().is_moving_a_color(); };
	sba.is_selectable = fconv_check([this](const MouseButtonEvent& m) { return m.button == MouseButton::LEFT && !imgui_combo_open && !renaming_subpalette && !current_subpalette().is_moving_a_color(); });
	
	sba.transform.position = { button1_x, absolute_y_off_bkg_top(button_y) };
	sba.text = "↓";
	sba.on_select = fconv_on_action([this]() {
		if ((*reflection.use_primary)())
			subpalette_overwrite_primary(true);
		else if ((*reflection.use_alternate)())
			subpalette_overwrite_alternate(true);
		});
	assign_widget(this, BUTTON_OVERWRITE_COLOR, std::make_shared<StandardTButton>(sba));
	b_t_wget(*this, BUTTON_OVERWRITE_COLOR).text().self.transform.position = { 0.05f, -0.05f };
	b_t_wget(*this, BUTTON_OVERWRITE_COLOR).text().self.transform.scale *= 1.0f;

	sba.transform.position.x += sba.transform.scale.x;
	sba.text = "+";
	sba.on_select = fconv_on_action([this]() {
		if ((*reflection.use_primary)())
		{
			bool adjacent = !MainWindow->is_shift_pressed();
			bool update_primary = MainWindow->is_ctrl_pressed();
			current_subpalette().new_color_from_primary((*reflection.get_picker_pri_rgba)(), adjacent, update_primary, true);
		}
		else if ((*reflection.use_alternate)())
		{
			bool adjacent = !MainWindow->is_shift_pressed();
			bool update_alternate = MainWindow->is_ctrl_pressed();
			current_subpalette().new_color_from_alternate((*reflection.get_picker_alt_rgba)(), adjacent, update_alternate, true);
		}
		});
	assign_widget(this, BUTTON_INSERT_NEW_COLOR, std::make_shared<StandardTButton>(sba));
	b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).text().self.transform.position = { 0.1f, -0.15f };
	b_t_wget(*this, BUTTON_INSERT_NEW_COLOR).text().self.transform.scale *= 1.4f;

	// LATER use image buttons
	sba.frange = Fonts::label_regular;
	sba.font_size = 22;
	sba.transform.position.x = button2_x;
	sba.text = "N";
	sba.on_select = fconv_on_action([this]() { new_subpalette(true); });
	assign_widget(this, BUTTON_SUBPALETTE_NEW, std::make_shared<StandardTButton>(sba));
	
	sba.transform.position.x += sba.transform.scale.x;
	sba.text = "R";
	sba.on_select = fconv_on_action([this]() { rename_subpalette(); });
	assign_widget(this, BUTTON_SUBPALETTE_RENAME, std::make_shared<StandardTButton>(sba));
	
	sba.transform.position.x += sba.transform.scale.x;
	sba.text = "D";
	sba.on_select = fconv_on_action([this]() { delete_current_subpalette(true); });
	assign_widget(this, BUTTON_SUBPALETTE_DELETE, std::make_shared<StandardTButton>(sba));
}

void ColorPalette::import_color_scheme()
{
	size_t num_subs = num_subpalettes();
	if (scheme->subschemes.size() < num_subs)
	{
		for (size_t i = 0; i < scheme->subschemes.size(); ++i)
		{
			get_subpalette(i).subscheme = scheme->subschemes[i];
			get_subpalette(i).reload_subscheme(true);
		}
		for (size_t i = scheme->subschemes.size(); i < num_subs; ++i)
			detach_widget(this, subpalette_index_in_widget(i));
		children.erase(children.begin() + subpalette_index_in_widget(scheme->subschemes.size()), children.end());
	}
	else
	{
		for (size_t i = 0; i < num_subs; ++i)
		{
			get_subpalette(i).subscheme = scheme->subschemes[i];
			get_subpalette(i).reload_subscheme(true);
		}
		for (size_t i = num_subs; i < scheme->subschemes.size(); ++i)
		{
			attach_widget(this, std::make_shared<ColorSubpalette>(&color_square_shader, &outline_rect_shader));
			get_subpalette(i).subscheme = scheme->subschemes[i];
			get_subpalette(i).self.transform.position.y = subpalette_pos_y();
			get_subpalette(i).reload_subscheme(true);
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

	FlatTransform global = self.transform;
	global.scale *= Scale(_col_count * SQUARE_SEP, _row_count * SQUARE_SEP);

	ur_wget(*this, BLACK_GRID)
		.set_attribute_single_vertex(0, 0, glm::value_ptr(glm::vec2{ global.left(), global.bottom() }))
		.set_attribute_single_vertex(1, 0, glm::value_ptr(glm::vec2{ global.right(), global.bottom() }))
		.set_attribute_single_vertex(2, 0, glm::value_ptr(glm::vec2{ global.left(), global.top() }))
		.set_attribute_single_vertex(3, 0, glm::value_ptr(glm::vec2{ global.right(), global.top() }))
		.set_attribute_single_vertex(0, 1, glm::value_ptr(glm::vec2{ 0, 0 }))
		.set_attribute_single_vertex(1, 1, glm::value_ptr(glm::vec2{ 1, 0 }))
		.set_attribute_single_vertex(2, 1, glm::value_ptr(glm::vec2{ 0, 1 }))
		.set_attribute_single_vertex(3, 1, glm::value_ptr(glm::vec2{ 1, 1 }))
		.send_buffer();

	// LATER is this necessary, or should only current subpalette be selected, and when selecting a new subpalette, sync it then.
	for (size_t i = 0; i < num_subpalettes(); ++i)
		get_subpalette(i).sync_with_palette();

	gui_transform.scale = wp_at(BACKGROUND).transform.scale * Machine.get_app_scale() * self.transform.scale;
	gui_transform.position = Machine.to_screen_coordinates(children[BACKGROUND]->global_of({ -0.5f, 0.5f }), *vp);
	gui_transform.position.y = MainWindow->height() - gui_transform.position.y;
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

void ColorPalette::subpalette_overwrite_primary(bool create_action)
{
	if (create_action)
	{
		RGBA prev_c = current_subpalette().subscheme->colors[current_subpalette().primary_index];
		RGBA new_c = (*reflection.get_picker_pri_rgba)();
		current_subpalette().overwrite_primary_color(new_c, false);
		Machine.history.push(std::make_shared<ColorOverwriteAction>(current_subpalette_ref(), prev_c, new_c, ColorPicker::EditingColor::PRIMARY));
	}
	else
		current_subpalette().overwrite_primary_color((*reflection.get_picker_pri_rgba)(), false);
}

void ColorPalette::subpalette_overwrite_alternate(bool create_action)
{
	if (create_action)
	{
		RGBA prev_c = current_subpalette().subscheme->colors[current_subpalette().alternate_index];
		RGBA new_c = (*reflection.get_picker_alt_rgba)();
		current_subpalette().overwrite_alternate_color(new_c, false);
		Machine.history.push(std::make_shared<ColorOverwriteAction>(current_subpalette_ref(), prev_c, new_c, ColorPicker::EditingColor::ALTERNATE));
	}
	else
		current_subpalette().overwrite_alternate_color((*reflection.get_picker_alt_rgba)(), false);
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
