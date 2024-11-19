#include "Menu.h"

#include "user/Machine.h"
#include "user/ControlScheme.h"

MenuPanel::MenuPanel()
{
	key_handler.callback = [this](const KeyEvent& k) {
		if (k.key == Key::ESCAPE && k.action == IAction::PRESS && submenus_open)
		{
			k.consumed = true;
			escape_to_close_menu = true;
		}
		};
}

void MenuPanel::_send_view()
{
}

static const char* key_shortcut_in_scheme(const char* shortcut, ControlScheme scheme)
{
	return Machine.get_control_scheme() == scheme ? shortcut : nullptr;
}

static const char* key_shortcut_in_file_scheme(const char* shortcut)
{
	return key_shortcut_in_scheme(shortcut, ControlScheme::FILE);
}

void MenuPanel::draw()
{
	submenus_open = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginMainMenuBar())
	{
		_close_menu = escape_to_close_menu || (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows));
		ImGui::PopStyleVar();
		if (ImGui::BeginMenu("File"))
		{
			main_menu_setup();
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
			main_menu_setup();
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
			main_menu_setup();
			if (Machine.brushes_panel_visible())
			{
				if (ImGui::MenuItem("Close brushes panel")) { Machine.close_brushes_panel(); }
			}
			else
			{
				if (ImGui::MenuItem("Open brushes panel")) { Machine.open_brushes_panel(); }
			}
			if (Machine.palette_panel_visible())
			{
				if (ImGui::MenuItem("Close palette panel")) { Machine.close_palette_panel(); }
			}
			else
			{
				if (ImGui::MenuItem("Open palette panel")) { Machine.open_palette_panel(); }
			}
			if (Machine.scene_panel_visible())
			{
				if (ImGui::MenuItem("Close scene panel")) { Machine.close_scene_panel(); }
			}
			else
			{
				if (ImGui::MenuItem("Open scene panel")) { Machine.open_scene_panel(); }
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
			main_menu_setup();
			if (ImGui::MenuItem("Download user manual")) { Machine.download_user_manual(); }
			ImGui::EndMenu();
		}

		// LATER make into menu?
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

void MenuPanel::main_menu_setup()
{
	if (_close_menu)
	{
		ImGui::CloseCurrentPopup();
		escape_to_close_menu = false;
	}
	else
		submenus_open = true;
}
