#include "Quasar.h"

#include <array>

#include <stb/stb_image.h>
#include <imgui/imgui.h>

#include "Macros.h"
#include "pipeline/Shader.h"
#include "pipeline/Sprite.h"
#include "pipeline/Renderer.h"
#include "variety/Geometry.h"
#include "user/Platform.h"
#include "edit/Color.h"
#include "user/UserInput.h"
#include "variety/Debug.h"
#include "user/Machine.h"
#include "user/GUI.h"

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "[GLFW ERROR " << error << "]: " << description << std::endl;
	QUASAR_ASSERT(false);
}

int main()
{
	QuasarSettings::load_settings();
	if (glfwInit() != GLFW_TRUE)
		return -1;
	glfwSetErrorCallback(glfw_error_callback);
	Machine.main_window = new Window("Quasar", 1440, 1080, true);
	if (!Machine.main_window)
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

	Machine.canvas_renderer = new Renderer(Machine.main_window, Shader());
	attach_canvas_controls();
	Machine.main_window->set_raw_mouse_motion(true); // TODO settable from user settings
	attach_global_user_controls();

	auto tux = Machine.images.construct(ImageConstructor("ex/einstein.png"));
	Machine.canvas_image = Machine.images.get(tux);
	Sprite sprite(tux);
	Machine.canvas_renderer->sprites().push_back(&sprite);
	
	Machine.canvas_renderer->set_app_scale(1.5f, 1.5f);

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
	Machine.canvas_renderer->sprites().push_back(&p1);
	Machine.canvas_renderer->sprites().push_back(&p2);
	Machine.canvas_renderer->sprites().push_back(&p3);
	Machine.canvas_renderer->sprites().push_back(&p4);

	Machine.canvas_renderer->clipping_rect().window_size_to_bounds = [](int w, int h) -> glm::ivec4 { return {
		w / 10, h / 10, 8 * w / 10, 8 * h / 10
	}; };
	Machine.canvas_renderer->clipping_rect().update_window_size(Machine.main_window->width(), Machine.main_window->height());

	// NOTE only one window, so no need to call bind_gui() at each frame.
	Machine.main_window->bind_gui();
	for (;;)
	{
		glfwPollEvents();
		if (Machine.main_window->should_close())
			break;
		on_render();
	}

	Machine.destroy();
	glfwTerminate();
	return 0;
}

void on_render()
{
	Machine.main_window->new_frame();
	Machine.canvas_renderer->frame_cycle();
	render_gui();
	Machine.main_window->end_frame();
}
