#include "ColorPalette.h"

#include <glm/gtc/type_ptr.inl>

#include "../render/Uniforms.h"


ColorSubpalette::ColorSubpalette()
	: Widget(_W_COUNT)
{
}

void ColorSubpalette::init(Shader* color_square_shader)
{
	// TODO initialize widgets
	assign_widget(this, SQUARES, new W_IndexedRenderable(color_square_shader));
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	squares.set_num_vertices(ColorSubscheme::MAX_COLORS * 4);
	squares.push_back_quads(ColorSubscheme::MAX_COLORS);
	squares.send_both_buffers_resized();

	sync_with_palette();
}

void ColorSubpalette::reload_subscheme()
{
	// TODO
}

void ColorSubpalette::draw()
{
	ir_wget(*this, SQUARES).draw();
}

void ColorSubpalette::sync_with_palette()
{
	IndexedRenderable& squares = ir_wget(*this, SQUARES);
	WidgetPlacement global;
	for (size_t i = 0; i < ColorSubscheme::MAX_COLORS; ++i)
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

WidgetPlacement ColorSubpalette::square_wp(size_t i)
{
	Position initial_pos{ -ColorPalette::SQUARE_SEP * (ColorPalette::COL_COUNT - 1) * 0.5f, ColorPalette::SQUARE_SEP * (ColorPalette::ROW_COUNT - 1) * 0.5f };
	return { { { initial_pos + Position{ ColorPalette::SQUARE_SEP * (i % ColorPalette::COL_COUNT), -ColorPalette::SQUARE_SEP * (i / ColorPalette::ROW_COUNT) }}, ColorPalette::SQUARE_SIZE } };
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

ColorPalette::ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler)
	: Widget(SUBPALETTE_START), vp(vp), parent_mb_handler(parent_mb_handler), parent_key_handler(parent_key_handler),
	grid_shader(FileSystem::shader_path("palette/black_grid.vert"), FileSystem::shader_path("palette/black_grid.frag")),
	color_square_shader(FileSystem::shader_path("palette/color_square.vert"), FileSystem::shader_path("palette/color_square.frag"))
{
	assign_widget(this, BLACK_GRID, new W_UnitRenderable(&grid_shader));
	sync_widget_with_vp();

	connect_input_handlers();
	new_subpalette(); // always have at least one subpalette
}

ColorPalette::~ColorPalette()
{
	parent_mb_handler.remove_child(&mb_handler);
	parent_key_handler.remove_child(&key_handler);
}

void ColorPalette::draw()
{
	// TODO render imgui
	get_subpalette(current_subscheme).draw();
	ur_wget(*this, BLACK_GRID).draw();
}

void ColorPalette::send_vp()
{
	Uniforms::send_matrix3(color_square_shader, "u_VP", *vp);
	Uniforms::send_matrix3(grid_shader, "u_VP", *vp);
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
	attach_widget(this, new ColorSubpalette());
	scheme.subschemes.push_back(std::make_shared<ColorSubscheme>(std::vector<RGBA>{
		RGBA(1.0f, 0.0f, 0.0f, 1.0f),
		RGBA(1.0f, 0.0f, 1.0f, 1.0f),
		RGBA(0.0f, 1.0f, 1.0f, 1.0f),
		RGBA(0.0f, 1.0f, 0.0f, 1.0f),
		RGBA(0.0f, 0.0f, 1.0f, 1.0f),
		RGBA(1.0f, 1.0f, 0.0f, 1.0f),
	})); // TODO remove list initializer
	current_subscheme = num_subpalettes() - 1;
	get_subpalette(current_subscheme).subscheme = scheme.subschemes[current_subscheme];
	get_subpalette(current_subscheme).init(&color_square_shader);
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
	key_handler.callback = [](const KeyEvent& mb) {
		// TODO
		};
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
