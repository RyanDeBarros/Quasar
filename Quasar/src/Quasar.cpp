#include "Quasar.h"

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

struct Quasar
{
	Window* main_window = nullptr;
	Renderer* canavas_renderer = nullptr;

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
	glfwSwapInterval(1);
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));

	canavas_renderer = new Renderer(main_window, Shader());
	attach_canvas_controls(canavas_renderer);
	main_window->set_raw_mouse_motion(true); // TODO settable from user settings

	// TODO for now, only one renderer, so only needs to be called once, not on every draw frame.
	// if more than one renderer, than call bind() before on_draw(). This will be the case for UI renderer.
	canavas_renderer->bind();

	auto tux = ImageRegistry.construct(ImageConstructor("ex/einstein.png"));
	auto tux_img = ImageRegistry.get(tux);
	Sprite sprite(tux);
	canavas_renderer->sprites().push_back(&sprite);
	tux_img->rotate_180();

	main_window->set_cursor(create_cursor(StandardCursor::CROSSHAIR));

	canavas_renderer->set_app_scale(1.2f, 1.2f);

	for (;;)
	{
		glfwPollEvents();
		if (main_window->should_close())
			break;
		on_render();

		sprite.set_modulation(ColorFrame(HSV(modulo(0.25f * glfwGetTime(), 1.0f), 0.2f, 1.0f), 255));
	}

	ImageRegistry.clear();
	ShaderRegistry.clear();
	delete canavas_renderer;
	canavas_renderer = nullptr;
	delete main_window;
	main_window = nullptr;
	return 0;
}

void Quasar::on_render()
{
	canavas_renderer->on_render();
}

void on_render()
{
	quasar.on_render();
}
