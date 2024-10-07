#include <array>

#include <stb/stb_image.h>

#include "Macros.h"
#include "Shader.h"
#include "Sprite.h"
#include "Renderer.h"
#include "Geometry.h"

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	int window_width = 1440;
	int window_height = 1080;
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Quasor", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}
	glfwSwapInterval(1);
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));

	Renderer renderer(window, Shader());
	// only one renderer, so only needs to be called once, not on every draw frame. if more than one renderer, than call bind() before on_draw().
	renderer.bind();

	auto tux = ImageRegistry.construct(ImageConstructor("ex/einstein.png"));
	auto tux_img = ImageRegistry.get(tux);
	Sprite sprite(tux);
	renderer.sprites().push_back(&sprite);
	tux_img->rotate_180();
	tux_img->update_texture();

	for (;;)
	{
		glfwPollEvents();
		if (glfwWindowShouldClose(window))
			break;

		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		renderer.on_draw();
		glfwSwapBuffers(window);
	}

	return 0;
}
