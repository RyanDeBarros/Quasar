#include "Quasar.h"

#include <array>

#include <stb/stb_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
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
	main_window = new Window("Quasar", 1440, 1080);
	if (!main_window)
	{
		glfwTerminate();
		return -1;
	}
	// TODO put at end of Window constructor?
	ImGui::CreateContext();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(main_window->window, true);
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

	for (;;)
	{
		glfwPollEvents();
		if (main_window->should_close())
			break;
		on_render();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
	canvas_renderer->bind();
	canvas_renderer->on_render();
	canvas_renderer->unbind();

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
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	main_window->swap_buffers();
}

void on_render()
{
	quasar.on_render();
}
