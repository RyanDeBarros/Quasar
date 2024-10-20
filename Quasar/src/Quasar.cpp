#include <array>

#include <stb/stb_image.h>
#include <imgui/imgui.h>

#include "Macros.h"
#include "pipeline/Shader.h"
#include "pipeline/Sprite.h"
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
	//QuasarSettings::load_settings();
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

	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);

	Machine.init_renderer();

	Machine.recent_files = {"a.qua", "b.qua", "c.qua"};
	Machine.recent_image_files = {"1.png", "2.gif", "3.jpg"};

	//Machine.import_file("ex/einstein.png");
	Machine.set_easel_scale(1.5f, 1.5f);

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
