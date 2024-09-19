#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#if QUASAR_DEBUG == 1
#ifndef QUASAR_ASSERT
#define QUASAR_ASSERT(x) if (!(x)) __debugbreak();
#endif
#ifndef QUASAR_GL
#define QUASAR_GL(x) QUASAR_ASSERT(no_gl_error(#x, __FILE__, __LINE__)) x; QUASAR_ASSERT(no_gl_error(#x, __FILE__, __LINE__))
#endif
#else
#ifndef QUASAR_ASSERT
#define QUASAR_ASSERT(x)
#endif
#ifndef QUASAR_GL
#define QUASAR_GL(x) x;
#endif
#endif

constexpr unsigned short QUASAR_MAX_VERTICES = 1024;
constexpr unsigned short QUASAR_MAX_INDEXES = 2048;

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "[GLFW ERROR " << error << "]: " << description << std::endl;
	QUASAR_ASSERT(false);
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
	GLFWwindow* window = glfwCreateWindow(1440, 1080, "Quasor", nullptr, nullptr);
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

	GLfloat* vertex_pool = new GLfloat[QUASAR_MAX_VERTICES];
	GLuint* index_pool = new GLuint[QUASAR_MAX_INDEXES];
	GLuint vao, vb, ib;

	QUASAR_GL(glGenVertexArrays(1, &vao));
	QUASAR_GL(glBindVertexArray(vao));
	
	QUASAR_GL(glGenBuffers(1, &vb));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, vb));
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, QUASAR_MAX_VERTICES * sizeof(GLfloat), vertex_pool, GL_DYNAMIC_DRAW));
	QUASAR_GL(glGenBuffers(1, &ib));
	QUASAR_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib));
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, QUASAR_MAX_INDEXES * sizeof(GLuint), index_pool, GL_DYNAMIC_DRAW));

	QUASAR_GL(glEnableVertexAttribArray(0));
	QUASAR_GL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (const GLvoid*)0));

	GLfloat varr[8] = {
		0.5f, 0.5f,
		-0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f
	};
	GLuint iarr[6] = {
		0, 1, 2,
		2, 3, 0
	};

	memcpy(vertex_pool, varr, 8 * sizeof(GLfloat));
	memcpy(index_pool, iarr, 6 * sizeof(GLuint));

	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(GLfloat), vertex_pool));
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 6 * sizeof(GLuint), index_pool));

	for (glfwPollEvents(); !glfwWindowShouldClose(window); glfwPollEvents())
	{

		QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));




		glfwSwapBuffers(window);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
	}


	delete[] vertex_pool;

	return 0;
}
