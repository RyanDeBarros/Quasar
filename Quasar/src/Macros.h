#pragma once

#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

inline bool no_gl_error(const char* function, const char* file, int line)
{
	bool no_err = true;
	while (GLenum error = glGetError())
	{
		std::cerr << "[OpenGL ERROR " << error << "]: " << function << " " << file << ":" << line << std::endl;
		no_err = false;
	}
	return no_err;
}
