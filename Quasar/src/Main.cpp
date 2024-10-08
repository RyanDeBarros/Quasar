#include <array>

#include <stb/stb_image.h>

#include "Macros.h"
#include "Shader.h"
#include "Sprite.h"
#include "Renderer.h"
#include "Geometry.h"
#include "Window.h"

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
	Window window("Quasar", 1440, 1080);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwSwapInterval(1);
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));

	Renderer renderer(&window, Shader());
	// only one renderer, so only needs to be called once, not on every draw frame. if more than one renderer, than call bind() before on_draw().
	renderer.bind();

	auto tux = ImageRegistry.construct(ImageConstructor("ex/einstein.png"));
	auto tux_img = ImageRegistry.get(tux);
	Sprite sprite(tux);
	renderer.sprites().push_back(&sprite);
	tux_img->rotate_180();

	window.set_cursor(create_cursor(StandardCursor::CROSSHAIR));

	for (;;)
	{
		glfwPollEvents();
		if (window.should_close())
			break;
		renderer.on_draw();
	}

	ImageRegistry.clear();
	ShaderRegistry.clear();
	return 0;
}
