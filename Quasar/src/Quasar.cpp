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
	if (!Machine.create_main_window())
	{
		glfwTerminate();
		return -1;
	}
	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	glfwSwapInterval(GLFW_FALSE); // TODO off by default, but add to user settings.
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));
	glEnable(GL_SCISSOR_TEST);

	Machine.init_renderer();

	Machine.recent_files = {"a.qua", "b.qua", "c.qua"};
	Machine.recent_image_files = {"1.png", "2.gif", "3.jpg"};

	//Machine.import_file("ex/einstein.png");
	Machine.canvas_renderer->set_app_scale(1.5f, 1.5f);

	Machine.canvas_renderer->clipping_rect().window_size_to_bounds = [](int w, int h) -> glm::ivec4 { return {
		w / 10, h / 10, 8 * w / 10, 8 * h / 10
	}; };
	Machine.canvas_renderer->clipping_rect().update_window_size(Machine.main_window->width(), Machine.main_window->height());

	// NOTE only one window, so no need to call bind_gui() at each frame.
	Machine.main_window->bind_gui();
	for (;;)
	{
		glfwPollEvents();
		if (Machine.should_exit())
			break;
		Machine.on_render();
	}

	Machine.destroy();
	glfwTerminate();
	return 0;
}
