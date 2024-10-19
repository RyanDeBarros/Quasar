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

inline void gen_vao_dynamic_draw(GLuint* vao, GLuint* vb, GLuint* ib, size_t vertex_count, size_t vertex_stride, size_t index_count, const GLfloat* varr, const GLuint* iarr)
{
	QUASAR_GL(glGenVertexArrays(1, vao));
	QUASAR_GL(glBindVertexArray(*vao));

	QUASAR_GL(glGenBuffers(1, vb));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, *vb));
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, vertex_count * vertex_stride * sizeof(GLfloat), varr, GL_DYNAMIC_DRAW));
	if (ib)
	{
		QUASAR_GL(glGenBuffers(1, ib));
		QUASAR_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ib));
		QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), iarr, GL_DYNAMIC_DRAW));
	}
}

inline void delete_vao_buffers(GLuint vao, GLuint vb, GLuint ib)
{
	QUASAR_GL(glDeleteBuffers(1, &vao));
	QUASAR_GL(glDeleteBuffers(1, &vb));
	if (ib != 0)
	{
		QUASAR_GL(glDeleteBuffers(1, &ib));
	}
}
