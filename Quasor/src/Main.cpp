#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#if QUASOR_DEBUG == 1
#ifndef QUASOR_ASSERT
#define QUASOR_ASSERT(x) if (!(x)) __debugbreak();
#endif
#ifndef QUASOR_GL
#define QUASOR_GL(x) QUASOR_ASSERT(no_gl_error(#x, __FILE__, __LINE__)) x; QUASOR_ASSERT(no_gl_error(#x, __FILE__, __LINE__))
#endif
#else
#ifndef QUASOR_ASSERT
#define QUASOR_ASSERT(x)
#endif
#ifndef QUASOR_GL
#define QUASOR_GL(x) x;
#endif
#endif

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "[GLFW ERROR " << error << "]: " << description << std::endl;
	QUASOR_ASSERT(false);
}

static bool no_gl_error(const char* function, const char* file, int line)
{
	bool no_err = true;
	while (GLenum error = glGetError())
	{
		std::cerr << "[OpenGL ERROR " << error << "]: " << function << " " << file << ":" << line << std::endl;
		no_err = false;
	}
	return no_err;
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
	GLFWwindow* window = glfwCreateWindow(2048, 1024, "Quasor", nullptr, nullptr);
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
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	for (glfwPollEvents(); !glfwWindowShouldClose(window); glfwPollEvents())
	{
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	return 0;
}
