#pragma once

#include "variety/Geometry.h"

extern void render_gui();

struct MainWindowLayout
{
	IntBounds MenuPanel;
	IntBounds BrushPanel;
	IntBounds EaselPanel;
	IntBounds PalettePanel;
	IntBounds ViewsPanel;

	MainWindowLayout(int width, int height, int menu_panel_height, int brush_panel_width, int palette_panel_width, int views_panel_height);

	void set_size(int width, int height);
	void set_menu_panel_height(int height);
	void set_brush_panel_width(int width);
	void set_palette_panel_width(int width);
	void set_views_panel_width(int height);
};
