#include "Quasar.h"

#include <array>

#include <stb/stb_image.h>
#include <imgui/imgui.h>
#include <tinyfd/tinyfiledialogs.h>

#include "Macros.h"
#include "pipeline/Shader.h"
#include "pipeline/Sprite.h"
#include "pipeline/Renderer.h"
#include "variety/Geometry.h"
#include "user/Platform.h"
#include "edit/Color.h"
#include "user/UserInput.h"
#include "variety/Debug.h"

struct Quasar
{
	Window* main_window = nullptr;
	Renderer* canvas_renderer = nullptr;

	int exec();
	void on_render();
};

Quasar quasar;

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "[GLFW ERROR " << error << "]: " << description << std::endl;
	QUASAR_ASSERT(false);
}

int main()
{
	return quasar.exec();
}

int Quasar::exec()
{
	QuasarSettings::load_settings();
	if (glfwInit() != GLFW_TRUE)
		return -1;
	glfwSetErrorCallback(glfw_error_callback);
	main_window = new Window("Quasar", 1440, 1080, true);
	if (!main_window)
	{
		glfwTerminate();
		return -1;
	}
	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	glfwSwapInterval(1);
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));
	glEnable(GL_SCISSOR_TEST);

	canvas_renderer = new Renderer(main_window, Shader());
	attach_canvas_controls(canvas_renderer);
	main_window->set_raw_mouse_motion(true); // TODO settable from user settings

	auto tux = ImageRegistry.construct(ImageConstructor("ex/einstein.png"));
	auto tux_img = ImageRegistry.get(tux);
	Sprite sprite(tux);
	canvas_renderer->sprites().push_back(&sprite);
	tux_img->rotate_180();

	canvas_renderer->set_app_scale(1.5f, 1.5f);

	Sprite p1(tux);
	p1.transform = { { 200.0f, 100.0f }, 0.0f, { 0.1f, 0.1f } };
	p1.sync_transform();
	Sprite p2(tux);
	p2.transform = { { -100.0f, 100.0f }, 0.0f, { 0.1f, 0.1f } };
	p2.sync_transform();
	Sprite p3(tux);
	p3.transform = { { -100.0f, -100.0f }, 0.0f, { 0.1f, 0.1f } };
	p3.sync_transform();
	Sprite p4(tux);
	p4.transform = { { 100.0f, -100.0f }, 0.0f, { 0.1f, 0.1f } };
	p4.sync_transform();
	canvas_renderer->sprites().push_back(&p1);
	canvas_renderer->sprites().push_back(&p2);
	canvas_renderer->sprites().push_back(&p3);
	canvas_renderer->sprites().push_back(&p4);

	canvas_renderer->clipping_rect().window_size_to_bounds = [](int w, int h) -> glm::ivec4 { return {
		w / 10, h / 10, 8 * w / 10, 8 * h / 10
	}; };
	canvas_renderer->clipping_rect().update_window_size(main_window->width(), main_window->height());

	// NOTE only one window, so no need to call bind_gui() at each frame.
	main_window->bind_gui();
	for (;;)
	{
		glfwPollEvents();
		if (main_window->should_close())
			break;
		on_render();
	}

	ImageRegistry.clear();
	ShaderRegistry.clear();
	delete canvas_renderer;
	canvas_renderer = nullptr;
	delete main_window;
	main_window = nullptr;
	glfwTerminate();
	return 0;
}

void Quasar::on_render()
{
	main_window->new_frame();
	canvas_renderer->frame_cycle();
	gui_render();
	main_window->end_frame();
}

void on_render()
{
	quasar.on_render();
}

void gui_render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::PopStyleVar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New file", "CTRL+N")) {}
			if (ImGui::MenuItem("Open file", "CTRL+O")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Save", "CTRL+S"))
			{
				const char* filters[3] = { "*.qua", "*.png", "*.gif" };
				//const char* savefile = tinyfd_saveFileDialog("Save file", "", 3, filters, "");
				const char* savefile = tinyfd_openFileDialog("Open file", "", 3, filters, "", true);
				if (savefile)
					std::cout << savefile << std::endl;
			}
			if (ImGui::MenuItem("Save as", "CTRL+SHIFT+S")) {}
			if (ImGui::MenuItem("Save a copy", "CTRL+ALT+S")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+SHIFT+Z", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
