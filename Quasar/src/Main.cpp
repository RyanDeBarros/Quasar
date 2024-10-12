#include <array>

#include <stb/stb_image.h>

#include "Macros.h"
#include "Shader.h"
#include "Sprite.h"
#include "Renderer.h"
#include "Geometry.h"
#include "Platform.h"
#include "Color.h"
#include "UserInput.h"

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
	Window main_window("Quasar", 1440, 1080);
	if (!main_window)
	{
		glfwTerminate();
		return -1;
	}
	glfwSwapInterval(1);
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));

	Renderer renderer(&main_window, Shader());
	UserInputManager uim(&renderer);
	main_window.set_raw_mouse_motion(true); // NOTE settable from user settings

	// only one renderer, so only needs to be called once, not on every draw frame. if more than one renderer, than call bind() before on_draw().
	renderer.bind();

	auto tux = ImageRegistry.construct(ImageConstructor("ex/einstein.png"));
	auto tux_img = ImageRegistry.get(tux);
	Sprite sprite(tux);
	renderer.sprites().push_back(&sprite);
	tux_img->rotate_180();

	main_window.set_cursor(create_cursor(StandardCursor::CROSSHAIR));

	renderer.set_app_scale(1.2f, 1.2f);

	for (;;)
	{
		glfwPollEvents();
		if (main_window.should_close())
			break;
		renderer.on_draw();
		uim.update();

		sprite.set_modulation(ColorFrame(HSV(modulo(0.25f * glfwGetTime(), 1.0f), 0.15f, 1.0f), 255));
	}

	ImageRegistry.clear();
	ShaderRegistry.clear();
	return 0;
}
