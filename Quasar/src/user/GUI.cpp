#include "GUI.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Machine.h"
#include "ControlScheme.h"

static const char* key_shortcut_in_scheme(const char* shortcut, ControlScheme scheme)
{
	return Machine.get_control_scheme() == scheme ? shortcut : nullptr;
}

static const char* key_shortcut_in_file_scheme(const char* shortcut)
{
	return key_shortcut_in_scheme(shortcut, ControlScheme::FILE);
}

// LATER put these in main menu panel, and use new imgui key handler to consume ESCAPE pressed and close menus if they are open.
void render_main_menu_bar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginMainMenuBar())
	{
		const bool close_menu = ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		ImGui::PopStyleVar();
		if (ImGui::BeginMenu("File"))
		{
			if (close_menu)
				ImGui::CloseCurrentPopup();
			if (ImGui::MenuItem("New Quasar file", key_shortcut_in_file_scheme("CTRL+N"))) { Machine.new_file(); }
			if (ImGui::MenuItem("Open Quasar file", key_shortcut_in_file_scheme("CTRL+O"))) { Machine.open_file(); }
			if (ImGui::MenuItem("Import image file", key_shortcut_in_file_scheme("CTRL+I"))) { Machine.import_file(); }
			if (ImGui::MenuItem("Export image file", key_shortcut_in_file_scheme("CTRL+E"), false, Machine.canvas_image_ready())) { Machine.export_file(); }
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
			if (close_menu)
				ImGui::CloseCurrentPopup();
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
			if (close_menu)
				ImGui::CloseCurrentPopup();
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
			if (close_menu)
				ImGui::CloseCurrentPopup();
			if (ImGui::MenuItem("Download user manual")) { Machine.download_user_manual(); }
			ImGui::EndMenu();
		}

		// TODO make into menu?
		ImGui::SameLine(ImGui::GetCursorPosX() + 100);
		ImGui::Text("Control Scheme:");
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("PALETTE").x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetFontSize() + ImGui::GetStyle().ItemInnerSpacing.x * 2);
		static const char* control_scheme_display_names[] = { "FILE", "PALETTE" };
		static const char* control_scheme_names[] = {
			"FILE    (CTRL+1) (Row)",
			"PALETTE (CTRL+2) (Row)"
		};
		static ControlScheme schemes[] = { ControlScheme::FILE, ControlScheme::PALETTE };
		if (ImGui::BeginCombo("##ControlSchemes", control_scheme_display_names[(int)Machine.get_control_scheme()]))
		{
			for (auto scheme : schemes)
			{
				if (ImGui::Selectable(control_scheme_names[(int)scheme]))
					Machine.set_control_scheme(scheme);
				if (Machine.get_control_scheme() == scheme)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::EndMainMenuBar();
	}
}
