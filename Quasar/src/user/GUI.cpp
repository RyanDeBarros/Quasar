#include "GUI.h"

#include "Machine.h"

MainWindowLayout::MainWindowLayout(int width, int height, int menu_panel_height, int brush_panel_width, int palette_panel_width, int views_panel_height)
{
	MenuPanel.x1 = 0;
	MenuPanel.x2 = width;
	MenuPanel.y1 = height - menu_panel_height;
	MenuPanel.y2 = height;
	BrushPanel.x1 = 0;
	BrushPanel.x2 = brush_panel_width;
	BrushPanel.y1 = views_panel_height;
	BrushPanel.y2 = MenuPanel.y1;
	EaselPanel.x1 = BrushPanel.x2;
	EaselPanel.x2 = width - palette_panel_width;
	EaselPanel.y1 = BrushPanel.y1;
	EaselPanel.y2 = BrushPanel.y2;
	PalettePanel.x1 = EaselPanel.x2;
	PalettePanel.x2 = width;
	PalettePanel.y1 = EaselPanel.y1;
	PalettePanel.y2 = EaselPanel.y2;
	ViewsPanel.x1 = 0;
	ViewsPanel.x2 = width;
	ViewsPanel.y1 = 0;
	ViewsPanel.y2 = views_panel_height;
}

void MainWindowLayout::set_size(int width, int height)
{
	MenuPanel.x2 = width;
	MenuPanel.y1 = height - (MenuPanel.y2 - MenuPanel.y1);
	MenuPanel.y2 = height;
	BrushPanel.y2 = MenuPanel.y1;
	EaselPanel.x1 = BrushPanel.x2;
	EaselPanel.x2 = width - (PalettePanel.x2 - PalettePanel.x1);
	EaselPanel.y2 = BrushPanel.y2;
	PalettePanel.x1 = EaselPanel.x2;
	PalettePanel.x2 = width;
	PalettePanel.y2 = EaselPanel.y2;
	ViewsPanel.x2 = width;
}

void MainWindowLayout::set_menu_panel_height(int height)
{
	MenuPanel.y1 = MenuPanel.y2 - height;
	BrushPanel.y2 = MenuPanel.y1;
	EaselPanel.y2 = MenuPanel.y1;
	PalettePanel.y2 = MenuPanel.y1;
}

void MainWindowLayout::set_brush_panel_width(int width)
{
	BrushPanel.x2 = width;
	EaselPanel.x1 = BrushPanel.x2;
}

void MainWindowLayout::set_palette_panel_width(int width)
{
	PalettePanel.x1 = PalettePanel.x2 - width;
	EaselPanel.x2 = PalettePanel.x1;
}

void MainWindowLayout::set_views_panel_width(int height)
{
	ViewsPanel.y2 = height;
	BrushPanel.y1 = ViewsPanel.y2;
	EaselPanel.y1 = ViewsPanel.y2;
	PalettePanel.y1 = ViewsPanel.y2;
}

static void render_main_menu()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::PopStyleVar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Quasar file", "CTRL+N")) { Machine.new_file(); }
			if (ImGui::MenuItem("Open Quasar file", "CTRL+O")) { Machine.open_file(); }
			if (ImGui::MenuItem("Import image file", "CTRL+I")) { Machine.import_file(); }
			if (ImGui::MenuItem("Export image file", "CTRL+E", false, Machine.canvas_image_ready())) { Machine.export_file(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Save", "CTRL+S")) { Machine.save_file(); }
			if (ImGui::BeginMenu("Open recent Quasar file", !Machine.recent_files.empty()))
			{
				for (const auto& recent_file : Machine.recent_files)
				{
					if (ImGui::MenuItem(recent_file.c_str())) { Machine.open_file(recent_file.c_str()); }
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Import recent image file", !Machine.recent_files.empty()))
			{
				for (const auto& recent_file : Machine.recent_image_files)
				{
					if (ImGui::MenuItem(recent_file.c_str())) { Machine.import_file(recent_file.c_str()); }
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save as", "CTRL+SHIFT+S")) { Machine.save_file_as(); }
			if (ImGui::MenuItem("Save a copy", "CTRL+ALT+S")) { Machine.save_file_copy(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT+F4")) { Machine.exit(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z", false, Machine.undo_enabled())) { Machine.undo(); }
			if (ImGui::MenuItem("Redo", "CTRL+SHIFT+Z", false, Machine.redo_enabled())) { Machine.redo(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Flip horizontally", "", false, Machine.canvas_image_ready())) { Machine.flip_horizontally(); }
			if (ImGui::MenuItem("Flip vertically", "", false, Machine.canvas_image_ready())) { Machine.flip_vertically(); }
			if (ImGui::MenuItem("Rotate 90", "", false, Machine.canvas_image_ready())) { Machine.rotate_90(); }
			if (ImGui::MenuItem("Rotate 180", "", false, Machine.canvas_image_ready())) { Machine.rotate_180(); }
			if (ImGui::MenuItem("Rotate 270", "", false, Machine.canvas_image_ready())) { Machine.rotate_270(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (Machine.brush_panel_visible())
			{
				if (ImGui::MenuItem("Close brush panel")) { Machine.close_brush_panel(); }
			}
			else
			{
				if (ImGui::MenuItem("Open brush panel")) { Machine.open_brush_panel(); }
			}
			if (Machine.palette_panel_visible())
			{
				if (ImGui::MenuItem("Close palette panel")) { Machine.close_palette_panel(); }
			}
			else
			{
				if (ImGui::MenuItem("Open palette panel")) { Machine.open_palette_panel(); }
			}
			if (Machine.views_panel_visible())
			{
				if (ImGui::MenuItem("Close views panel")) { Machine.close_views_panel(); }
			}
			else
			{
				if (ImGui::MenuItem("Open views panel")) { Machine.open_views_panel(); }
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Reset canvas view", "0 (Row)", false, Machine.canvas_image_ready())) { Machine.canvas_reset_camera(); }
			if (Machine.minor_gridlines_visible())
			{
				if (ImGui::MenuItem("Hide minor gridlines", "G", false, Machine.canvas_image_ready())) { Machine.hide_minor_gridlines(); }
			}
			else
			{
				if (ImGui::MenuItem("Show minor gridlines", "G", false, Machine.canvas_image_ready())) { Machine.show_minor_gridlines(); }
			}
			if (Machine.major_gridlines_visible())
			{
				if (ImGui::MenuItem("Hide major gridlines", "SHIFT+G", false, Machine.canvas_image_ready())) { Machine.hide_major_gridlines(); }
			}
			else
			{
				if (ImGui::MenuItem("Show major gridlines", "SHIFT+G", false, Machine.canvas_image_ready())) { Machine.show_major_gridlines(); }
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Download user manual")) { Machine.download_user_manual(); }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void render_gui()
{
	render_main_menu();
}
