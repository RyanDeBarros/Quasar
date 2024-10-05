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

template<size_t n>
static void attrib_pointers(std::array<int, n> lens, int stride)
{
	int offset = 0;
	int str = stride * sizeof(GL_FLOAT);
	for (size_t i = 0; i < n; ++i)
	{
		QUASAR_GL(glEnableVertexAttribArray(i));
		QUASAR_GL(glVertexAttribPointer(i, lens[i], GL_FLOAT, GL_FALSE, str, (const GLvoid*)offset));
		offset += lens[i] * sizeof(GLfloat);
	}
}

int main()
{
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
	QUASAR_GL(glClearColor(0.5f, 0.5f, 0.5f, 0.5f));
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);

	Renderer renderer(window, load_shader());

	stbi_set_flip_vertically_on_load(true);
	Image img;
	img.pixels = stbi_load("ex/tux.png", &img.width, &img.height, &img.bpp, 0);
	img.deletion_policy = ImageDeletionPolicy::FROM_STBI;
	
	Sprite sprite = rect_sprite(&img, false);
	attrib_pointers(std::array<int, 6>{ 1, 2, 1, 2, 4, 4 }, sprite.buf->stride);

	sprite.tex->bind(0);
	renderer.pool_over_buffer(*sprite.buf);

	// only one renderer, so only needs to be called once, not on every draw frame. if more than one renderer, than call bind() before on_draw().
	renderer.bind();

	for (glfwPollEvents(); !glfwWindowShouldClose(window); glfwPollEvents())
	{
		renderer.on_draw();
		glfwSwapBuffers(window);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
	}

	return 0;
}
