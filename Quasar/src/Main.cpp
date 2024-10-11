#include <array>

#include <stb/stb_image.h>

#include "Macros.h"
#include "Shader.h"
#include "Sprite.h"
#include "Renderer.h"
#include "Geometry.h"
#include "Window.h"
#include "Color.h"

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "[GLFW ERROR " << error << "]: " << description << std::endl;
	QUASAR_ASSERT(false);
}

int main()
{
	// RGB(230, 153, 187)  HSV(334, 43, 230)  HSL(334, 77, 192)

	auto prgb = [](RGB rgb) { std::cout << int(rgb.r) << " " << int(rgb.g) << " " << int(rgb.b) << std::endl; };
	auto phsv = [](HSV hsv) { std::cout << int(hsv.get_hue()) << " " << int(hsv.s) << " " << int(hsv.v) << std::endl; };
	auto phsl = [](HSL hsl) { std::cout << int(hsl.get_hue()) << " " << int(hsl.s) << " " << int(hsl.l) << std::endl; };


	//HSL hsl(334, 77, 192);
	//std::cout << int(hsl.get_hue()) << " " << int(hsl.s) << " " << int(hsl.l) << std::endl;
	//HSV hsv = to_hsv(hsl);
	//std::cout << int(hsv.get_hue()) << " " << int(hsv.s) << " " << int(hsv.v) << std::endl;
	//RGB rgb = to_rgb(hsl);
	RGB rgb(230, 153, 187);
	prgb(rgb);
	//HSL hsl2 = to_hsl(hsv);
	//std::cout << int(hsl2.get_hue()) << " " << int(hsl2.s) << " " << int(hsl2.l) << std::endl;
	//HSV hsv2 = to_hsv(hsl2);
	HSV hsv2 = to_hsv(rgb);
	phsv(hsv2);
	RGB rgb2 = to_rgb(hsv2);
	prgb(rgb2);
	HSV hsv3 = to_hsv(rgb2);
	phsv(hsv3);
	RGB rgb3 = to_rgb(hsv3);
	prgb(rgb3);
	//HSL hsl3 = to_hsl(hsv2);
	//std::cout << int(hsl3.get_hue()) << " " << int(hsl3.s) << " " << int(hsl3.l) << std::endl;
	std::cout << std::endl;

	ColorFrame color(HSL(334, 77, 192));
	prgb(color.rgb());
	phsv(color.hsv());
	phsl(color.hsl());

	std::cin.get();
	return 0;

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

	sprite.set_modulation({ 1.0f, 0.0f, 0.0f, 1.0f });
	
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
