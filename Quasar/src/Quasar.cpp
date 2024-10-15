#include "Quasar.h"

#include <array>

#include <stb/stb_image.h>

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

	// NOTE for now, only one renderer, so only needs to be called once, not on every draw frame.
	// if more than one renderer, than call bind() before on_draw(). This will be the case for UI renderer.
	canvas_renderer->bind();
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
	delete canvas_renderer;
	canvas_renderer = nullptr;
	delete main_window;
	main_window = nullptr;
	return 0;
}

void Quasar::on_render()
{
	canvas_renderer->on_render();
}

void on_render()
{
	quasar.on_render();
}
