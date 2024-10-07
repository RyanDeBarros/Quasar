#pragma once

#include <vector>

#include "Macros.h"

inline void bind_texture(GLuint texture, GLuint slot = 0)
{
	QUASAR_GL(glActiveTexture(GL_TEXTURE0 + slot));
	QUASAR_GL(glBindTexture(GL_TEXTURE_2D, texture));
}

inline void attrib_pointers(const std::vector<unsigned short> attrib_lengths, unsigned short stride)
{
	int offset = 0;
	int stride_bytes = stride * sizeof(GL_FLOAT);
	for (GLuint i = 0; i < attrib_lengths.size(); ++i)
	{
		QUASAR_GL(glEnableVertexAttribArray(i));
#pragma warning(push)
#pragma warning(disable : 4312)
		QUASAR_GL(glVertexAttribPointer(i, attrib_lengths[i], GL_FLOAT, GL_FALSE, stride_bytes, (GLvoid*)offset));
#pragma warning(pop)
		offset += attrib_lengths[i] * sizeof(GLfloat);
	}
}
