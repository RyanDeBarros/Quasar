#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "IO.h"

#if QUASAR_DEBUG == 1
#ifndef QUASAR_ASSERT
#define QUASAR_ASSERT(x) if (!(x)) __debugbreak();
#endif
#ifndef QUASAR_GL
#define QUASAR_GL(x) QUASAR_ASSERT(no_gl_error(#x, __FILE__, __LINE__)) x; QUASAR_ASSERT(no_gl_error(#x, __FILE__, __LINE__))
#endif
#else
#ifndef QUASAR_ASSERT
#define QUASAR_ASSERT(x) ;
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

static GLuint compile_shader(GLenum type, const char* shader, const char* filepath)
{
	QUASAR_GL(GLuint id = glCreateShader(type));
	QUASAR_GL(glShaderSource(id, 1, &shader, nullptr));
	QUASAR_GL(glCompileShader(id));

	int result;
	QUASAR_GL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		QUASAR_GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		GLchar* message = new GLchar[length];
		QUASAR_GL(glGetShaderInfoLog(id, length, &length, message));
		std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: \"" << filepath << "\"\n" << message << std::endl;
		delete[] message;
		QUASAR_GL(glDeleteShader(id));
		return 0;
	}
	return id;
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
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
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

	std::string vertex_shader;
	if (!IO::read_file("res/standard.vert", vertex_shader))
		QUASAR_ASSERT(false);
	std::string fragment_shader;
	if (!IO::read_file("res/standard.frag", fragment_shader))
		QUASAR_ASSERT(false);

	QUASAR_GL(GLuint shader = glCreateProgram());
	GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader.c_str(), "res/standard.vert");
	GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader.c_str(), "res/standard.frag");
	if (vs)
	{
		if (fs)
		{
			QUASAR_GL(glAttachShader(shader, vs));
			QUASAR_GL(glAttachShader(shader, fs));
			QUASAR_GL(glLinkProgram(shader));
			QUASAR_GL(glValidateProgram(shader));

			QUASAR_GL(glDeleteShader(vs));
			QUASAR_GL(glDeleteShader(fs));
		}
		else
		{
			QUASAR_GL(glDeleteProgram(shader));
			QUASAR_GL(glDeleteShader(vs));
			shader = 0;
		}
	}
	else
	{
		QUASAR_GL(glDeleteProgram(shader));
		shader = 0;
	}

	QUASAR_GL(glUseProgram(shader));

	constexpr int vstride = 14;
	GLfloat varr[4 * vstride] = {
		0.0f, -200.0f, -100.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f,  200.0f, -100.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		2.0f,  200.0f,  100.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		3.0f, -200.0f,  100.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};
	GLuint iarr[6] = {
		0, 1, 2,
		2, 3, 0
	};

	int offset = 0;
	int i = 0;
	int len = 1;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, vstride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 2;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, vstride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 1;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, vstride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 2;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, vstride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 4;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, vstride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 4;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, vstride * sizeof(GL_FLOAT), (const GLvoid*)offset));

	memcpy(vertex_pool, varr, 4 * vstride * sizeof(GLfloat));
	memcpy(index_pool, iarr, 6 * sizeof(GLuint));

	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * vstride * sizeof(GLfloat), vertex_pool));
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 6 * sizeof(GLuint), index_pool));

	QUASAR_GL(glEnable(GL_BLEND));
	QUASAR_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	for (glfwPollEvents(); !glfwWindowShouldClose(window); glfwPollEvents())
	{

		QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));




		glfwSwapBuffers(window);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
	}

	QUASAR_GL(glDeleteProgram(shader));
	QUASAR_GL(glDeleteBuffers(1, &vao));
	QUASAR_GL(glDeleteBuffers(1, &vb));
	QUASAR_GL(glDeleteBuffers(1, &ib));
	delete[] vertex_pool;

	return 0;
}
