#include "GUI.h"

#include "Machine.h"

static void render_main_menu()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::PopStyleVar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New file", "CTRL+N")) { Machine.new_file(); }
			if (ImGui::MenuItem("Open file", "CTRL+O")) { Machine.open_file(); }
			if (ImGui::MenuItem("Import file", "CTRL+I")) { Machine.import_file(); }
			if (ImGui::MenuItem("Export file", "CTRL+E", false, Machine.canvas_image_ready())) { Machine.export_file(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Save", "CTRL+S")) { Machine.save_file(); }
			if (ImGui::BeginMenu("Open recent", !Machine.recent_files.empty()))
			{
				for (const auto& recent_file : Machine.recent_files)
				{
					if (ImGui::MenuItem(recent_file.c_str())) { Machine.open_file(recent_file.c_str()); }
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Import recent", !Machine.recent_files.empty()))
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
