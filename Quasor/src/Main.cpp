#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

int main()
{
	if (glfwInit() != GLFW_TRUE)
		return -1;
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
