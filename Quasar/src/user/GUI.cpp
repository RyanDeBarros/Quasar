#include "GUI.h"

#include <tinyfd/tinyfiledialogs.h>

#include "Machine.h"

void render_gui()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::PopStyleVar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New file", "CTRL+N")) {}
			if (ImGui::MenuItem("Open file", "CTRL+O")) {}
			if (ImGui::BeginMenu("Open recent"))
			{
				if (ImGui::MenuItem("a.qua")) {}
				if (ImGui::MenuItem("b.qua")) {}
				if (ImGui::MenuItem("c.qua")) {}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "CTRL+S"))
			{
				const char* filters[3] = { "*.qua", "*.png", "*.gif" };
				//const char* savefile = tinyfd_saveFileDialog("Save file", "", 3, filters, "");
				const char* savefile = tinyfd_openFileDialog("Open file", "", 3, filters, "", true);
				if (savefile)
					std::cout << savefile << std::endl;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save as", "CTRL+SHIFT+S")) {}
			if (ImGui::MenuItem("Save a copy", "CTRL+ALT+S")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "ALT+F4")) { Machine.main_window->request_close(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z", false, Machine.undo_enabled())) { Machine.undo(); }
			if (ImGui::MenuItem("Redo", "CTRL+SHIFT+Z", false, Machine.redo_enabled())) { Machine.redo(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Flip horizontally")) { Machine.flip_horizontally(); }
			if (ImGui::MenuItem("Flip vertically")) { Machine.flip_vertically(); }
			if (ImGui::MenuItem("Rotate 180")) { Machine.rotate_180(); }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
