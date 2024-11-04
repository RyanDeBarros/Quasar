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

inline void bind_vao_buffers(GLuint vao, GLuint vb, GLuint ib)
{
	QUASAR_GL(glBindVertexArray(vao));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, vb));
	QUASAR_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib));
}

inline void bind_vao_buffers(GLuint vao, GLuint vb)
{
	QUASAR_GL(glBindVertexArray(vao));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, vb));
}

inline void unbind_vao_buffers()
{
	QUASAR_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	QUASAR_GL(glBindVertexArray(0));
}

inline void gen_dynamic_vao(GLuint& vao, GLuint& vb, GLuint& ib, size_t vertex_count, unsigned short vertex_stride, size_t index_count,
	const GLfloat* varr, const GLuint* iarr, const std::vector<unsigned short>& attributes)
{
	QUASAR_GL(glGenVertexArrays(1, &vao));
	QUASAR_GL(glGenBuffers(1, &vb));
	QUASAR_GL(glGenBuffers(1, &ib));
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, vertex_count * vertex_stride * sizeof(GLfloat), varr, GL_DYNAMIC_DRAW));
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), iarr, GL_DYNAMIC_DRAW));
	attrib_pointers(attributes, vertex_stride);
	unbind_vao_buffers();
}

inline void gen_dynamic_vao(GLuint& vao, GLuint& vb, size_t vertex_count, unsigned short vertex_stride, const GLfloat* varr, const std::vector<unsigned short>& attributes)
{
	QUASAR_GL(glGenVertexArrays(1, &vao));
	QUASAR_GL(glGenBuffers(1, &vb));
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, vertex_count * vertex_stride * sizeof(GLfloat), varr, GL_DYNAMIC_DRAW));
	attrib_pointers(attributes, vertex_stride);
	unbind_vao_buffers();
}

inline void buffer_data(GLuint vao, GLuint vb, GLuint ib, size_t vertex_count, unsigned short vertex_stride, size_t index_count, const GLfloat* varr, const GLuint* iarr)
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, vertex_count * vertex_stride * sizeof(GLfloat), varr, GL_DYNAMIC_DRAW));
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), iarr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

inline void buffer_data(GLuint vao, GLuint vb, size_t vertex_count, unsigned short vertex_stride, const GLfloat* varr)
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, vertex_count * vertex_stride * sizeof(GLfloat), varr, GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

inline void delete_vao_buffers(GLuint vao, GLuint vb, GLuint ib)
{
	QUASAR_GL(glDeleteBuffers(1, &vao));
	QUASAR_GL(glDeleteBuffers(1, &vb));
	QUASAR_GL(glDeleteBuffers(1, &ib));
}

inline void delete_vao_buffers(GLuint vao, GLuint vb)
{
	QUASAR_GL(glDeleteBuffers(1, &vao));
	QUASAR_GL(glDeleteBuffers(1, &vb));
}

inline static GLuint _currently_bound_shader = 0;

inline void update_currently_bound_shader()
{
	QUASAR_GL(glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&_currently_bound_shader)));
}

inline void bind_shader(GLuint shader)
{
	if (shader != _currently_bound_shader)
	{
		QUASAR_GL(glUseProgram(shader));
		_currently_bound_shader = shader;
	}
}

inline void unbind_shader()
{
	if (_currently_bound_shader)
	{
		QUASAR_GL(glUseProgram(0));
		_currently_bound_shader = 0;
	}
}
