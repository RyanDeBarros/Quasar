#pragma once

#include "Macros.h"

inline void bind_texture(GLuint texture, GLuint slot)
{
	QUASAR_GL(glActiveTexture(GL_TEXTURE0 + slot));
	QUASAR_GL(glBindTexture(GL_TEXTURE_2D, texture));
}
