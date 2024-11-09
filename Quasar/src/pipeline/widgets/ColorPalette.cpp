#include "ColorPalette.h"

ColorPalette::ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler)
	: Widget(_W_COUNT), parent_mb_handler(parent_mb_handler), parent_key_handler(parent_key_handler)
{
	initialize_widget();
	connect_input_handlers();
}

ColorPalette::~ColorPalette()
{
	parent_mb_handler.remove_child(&mb_handler);
	parent_key_handler.remove_child(&key_handler);
}

void ColorPalette::draw()
{
	// TODO draw subwidgets
}

void ColorPalette::send_vp()
{
	// TODO
	//Uniforms::send_matrix3(shaders..., "u_VP", *vp);
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

void ColorPalette::initialize_widget()
{
	// TODO initialize subwidgets
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
	// TODO sync subpalettes with subschemes
}

void ColorPalette::sync_widget_with_vp()
{
	// TODO
}
