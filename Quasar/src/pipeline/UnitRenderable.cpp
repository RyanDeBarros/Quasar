#include "UnitRenderable.h"

#include "variety/GLutility.h"

UnitRenderable::UnitRenderable(Shader& _shader, unsigned char num_vertices)
	: shader(_shader), num_vertices(num_vertices)
{
	varr = new GLfloat[num_vertices * shader.stride];
	std::memset(varr, 0, size_t(num_vertices) * shader.stride * sizeof(GLfloat));
	gen_dynamic_vao(vao, vb, num_vertices, shader.stride, varr, shader.attributes);
}

UnitRenderable::~UnitRenderable()
{
	delete_vao_buffers(vao, vb);
	delete[] varr;
}

void UnitRenderable::set_attribute(size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader.attributes[attrib];
	GLfloat* buf = varr + shader.attribute_offsets[attrib];
	for (char i = 0; i < num_vertices; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader.stride;
	}
}

void UnitRenderable::set_attribute_single_vertex(unsigned char vertex, size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader.attributes[attrib];
	memcpy(varr + shader.attribute_offsets[attrib] + vertex * shader.stride, v, attrib_len * sizeof(GLfloat));
}

void UnitRenderable::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, size_t(num_vertices) * shader.stride * sizeof(GLfloat), varr));
	unbind_vao_buffers();
}

void UnitRenderable::send_single_vertex(unsigned char vertex) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, vertex * shader.stride, shader.stride * sizeof(GLfloat), varr + vertex * shader.stride));
	unbind_vao_buffers();
}

void UnitRenderable::draw() const
{
	bind_shader(shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices));
	unbind_vao_buffers();
}

UnitMultiRenderable::UnitMultiRenderable(Shader& _shader, unsigned short num_units, unsigned char unit_num_vertices)
	: shader(_shader), num_units(num_units), unit_num_vertices(unit_num_vertices)
{
	auto num_vertices = num_units * unit_num_vertices;
	varr = new GLfloat[num_vertices * shader.stride];
	std::memset(varr, 0, size_t(num_vertices) * shader.stride * sizeof(GLfloat));
	gen_dynamic_vao(vao, vb, num_vertices, shader.stride, varr, shader.attributes);

	first = new GLint[num_units];
	count = new GLsizei[num_units];
	for (unsigned short i = 0; i < num_units; ++i)
	{
		first[i] = i * unit_num_vertices;
		count[i] = unit_num_vertices;
	}
}

UnitMultiRenderable::~UnitMultiRenderable()
{
	delete_vao_buffers(vao, vb);
	delete[] varr;
	delete[] first;
	delete[] count;
}

void UnitMultiRenderable::set_attribute(unsigned short unit, size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader.attributes[attrib];
	GLfloat* buf = varr + unit * unit_num_vertices * shader.stride + shader.attribute_offsets[attrib];
	for (char i = 0; i < unit_num_vertices; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader.stride;
	}
}

void UnitMultiRenderable::set_attribute_single_vertex(unsigned short unit, unsigned char vertex, size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader.attributes[attrib];
	memcpy(varr + (unit * unit_num_vertices + vertex) * shader.stride + shader.attribute_offsets[attrib], v, attrib_len * sizeof(GLfloat));
}

void UnitMultiRenderable::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, size_t(unit_num_vertices) * num_units * shader.stride * sizeof(GLfloat), varr));
	unbind_vao_buffers();
}

void UnitMultiRenderable::send_single_unit(unsigned short unit) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, unit * unit_num_vertices * shader.stride, size_t(unit_num_vertices) * shader.stride * sizeof(GLfloat), varr + unit * unit_num_vertices * shader.stride));
	unbind_vao_buffers();
}

void UnitMultiRenderable::send_single_vertex(unsigned short unit, unsigned char vertex) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, (unit * unit_num_vertices + vertex) * shader.stride, shader.stride * sizeof(GLfloat), varr + (unit * unit_num_vertices + vertex) * shader.stride));
	unbind_vao_buffers();
}

void UnitMultiRenderable::draw() const
{
	bind_shader(shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, first, count, num_units));
	unbind_vao_buffers();
}
