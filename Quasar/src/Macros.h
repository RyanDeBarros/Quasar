#pragma once

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

#include "Logger.h"

inline bool no_gl_error(const char* function, const char* file, int line)
{
	bool no_err = true;
	while (GLenum error = glGetError())
	{
		LOG << LOG.error << LOG.start_gl(error) << function << " " << file << ":" << line << LOG.nl;
		no_err = false;
	}
	if (!no_err)
		LOG << LOG.flush;
	return no_err;
}
