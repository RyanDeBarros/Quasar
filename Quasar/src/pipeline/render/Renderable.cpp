#include "Renderable.h"

#include "variety/GLutility.h"

UnitRenderable::UnitRenderable(Shader* shader, unsigned short num_vertices)
	: shader(shader)
{
	gen_dynamic_vao(vao, vb);
	if (shader)
	{
		set_num_vertices(num_vertices);
		attrib_pointers(shader->attributes, shader->stride);
		send_buffer_resized();
	}
	unbind_vao_buffers();
}

UnitRenderable::UnitRenderable(UnitRenderable&& other) noexcept
	: shader(other.shader), vao(other.vao), vb(other.vb), varr(std::move(other.varr))
{
	other.vao = 0;
	other.vb = 0;
}

UnitRenderable::~UnitRenderable()
{
	delete_vao_buffers(vao, vb);
}

void UnitRenderable::set_shader(Shader* shader_)
{
	size_t num_vs = num_vertices();
	bind_vao_buffers(vao, vb);
	if (shader)
		attrib_new_pointers(shader->attributes, shader->stride, shader_->attributes, shader_->stride);
	else
		attrib_pointers(shader_->attributes, shader_->stride);
	shader = shader_;
	varr.resize(shader->stride ? num_vs * shader->stride : 0, 0);
	send_buffer_resized();
	unbind_vao_buffers();
}

const UnitRenderable& UnitRenderable::get_attribute(unsigned int vertex, size_t attrib, float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(v, varr.data() + shader->attribute_offsets[attrib] + vertex * shader->stride, attrib_len * sizeof(GLfloat));
	return *this;
}

UnitRenderable& UnitRenderable::set_attribute(size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	GLfloat* buf = varr.data() + shader->attribute_offsets[attrib];
	size_t num_vs = num_vertices();
	for (unsigned short i = 0; i < num_vs; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader->stride;
	}
	return *this;
}

UnitRenderable& UnitRenderable::set_attribute_single_vertex(unsigned int vertex, size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(varr.data() + shader->attribute_offsets[attrib] + vertex * shader->stride, v, attrib_len * sizeof(GLfloat));
	return *this;
}

UnitRenderable& UnitRenderable::get_attribute(unsigned int vertex, size_t attrib, float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(v, varr.data() + shader->attribute_offsets[attrib] + vertex * shader->stride, attrib_len * sizeof(GLfloat));
	return *this;
}

void UnitRenderable::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, varr.size() * sizeof(GLfloat), varr.data()));
	unbind_vao_buffers();
}

void UnitRenderable::send_subbuffer(unsigned int offset, unsigned int num_vertices) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, offset * shader->stride * sizeof(GLfloat), num_vertices * shader->stride * sizeof(GLfloat), varr.data() + offset * shader->stride));
	unbind_vao_buffers();
}

const UnitRenderable& UnitRenderable::send_single_vertex(unsigned int vertex) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, vertex * shader->stride * sizeof(GLfloat), shader->stride * sizeof(GLfloat), varr.data() + vertex * shader->stride));
	unbind_vao_buffers();
	return *this;
}

UnitRenderable& UnitRenderable::send_single_vertex(unsigned int vertex)
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, vertex * shader->stride * sizeof(GLfloat), shader->stride * sizeof(GLfloat), varr.data() + vertex * shader->stride));
	unbind_vao_buffers();
	return *this;
}

void UnitRenderable::send_buffer_resized() const
{
	buffer_data(vao, vb, num_vertices(), shader->stride, varr.data());
}

void UnitRenderable::draw() const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices()));
	unbind_vao_buffers();
}

void UnitRenderable::draw(unsigned int num_vertices_to_draw) const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices_to_draw));
	unbind_vao_buffers();
}

void UnitRenderable::draw_as_lines() const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glDrawArrays(GL_LINE_STRIP, 0, num_vertices()));
	unbind_vao_buffers();
}

void UnitRenderable::set_num_vertices(size_t num_vertices_)
{
	varr.resize(num_vertices_ * shader->stride, 0);
	send_buffer_resized();
}

UnitMultiRenderable::UnitMultiRenderable(Shader* _shader, unsigned int num_units, unsigned int unit_num_vertices)
	: shader(_shader), num_units(num_units), unit_num_vertices(unit_num_vertices)
{
	gen_dynamic_vao(vao, vb);
	if (shader)
	{
		resize();
		attrib_pointers(shader->attributes, shader->stride);
		send_buffer_resized();
	}
	unbind_vao_buffers();
}

UnitMultiRenderable::~UnitMultiRenderable()
{
	delete_vao_buffers(vao, vb);
}

void UnitMultiRenderable::set_shader(Shader* shader_)
{
	bind_vao_buffers(vao, vb);
	if (shader)
		attrib_new_pointers(shader->attributes, shader->stride, shader_->attributes, shader_->stride);
	else
		attrib_pointers(shader_->attributes, shader_->stride);
	shader = shader_;
	if (shader)
		resize();
	send_buffer_resized();
	unbind_vao_buffers();
}

void UnitMultiRenderable::set_attribute(unsigned int unit, size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	GLfloat* buf = varr.data() + unit * unit_num_vertices * shader->stride + shader->attribute_offsets[attrib];
	for (unsigned short i = 0; i < unit_num_vertices; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader->stride;
	}
}

void UnitMultiRenderable::set_attribute_single_vertex(unsigned int unit, unsigned int vertex, size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(varr.data() + (unit * unit_num_vertices + vertex) * shader->stride + shader->attribute_offsets[attrib], v, attrib_len * sizeof(GLfloat));
}

void UnitMultiRenderable::get_attribute(unsigned int unit, unsigned int vertex, size_t attrib, float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(v, varr.data() + (unit * unit_num_vertices + vertex) * shader->stride + shader->attribute_offsets[attrib], attrib_len * sizeof(GLfloat));
}

void UnitMultiRenderable::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, varr.size() * sizeof(GLfloat), varr.data()));
	unbind_vao_buffers();
}

void UnitMultiRenderable::send_single_unit(unsigned int unit) const
{
	bind_vao_buffers(vao, vb);
	size_t unit_stride = unit_num_vertices * shader->stride;
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, unit * unit_stride * sizeof(GLfloat), unit_stride * sizeof(GLfloat), varr.data() + unit * unit_stride));
	unbind_vao_buffers();
}

void UnitMultiRenderable::send_single_vertex(unsigned int unit, unsigned int vertex) const
{
	bind_vao_buffers(vao, vb);
	size_t unit_offset = (unit * unit_num_vertices + vertex) * shader->stride;
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, unit_offset * sizeof(GLfloat), shader->stride * sizeof(GLfloat), varr.data() + unit_offset));
	unbind_vao_buffers();
}

void UnitMultiRenderable::send_buffer_resized() const
{
	buffer_data(vao, vb, varr.size(), shader->stride, varr.data());
}

void UnitMultiRenderable::draw() const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, first.data(), count.data(), num_units));
	unbind_vao_buffers();
}

void UnitMultiRenderable::draw(unsigned int num_units_to_draw) const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, first.data(), count.data(), num_units_to_draw));
	unbind_vao_buffers();
}

void UnitMultiRenderable::resize()
{
	size_t num_vertices = num_units * unit_num_vertices;
	varr.resize(num_vertices * shader->stride, 0);
	first.resize(num_units);
	count.resize(num_units);

	for (unsigned short i = 0; i < num_units; ++i)
	{
		first[i] = i * unit_num_vertices;
		count[i] = unit_num_vertices;
	}
}

IndexedRenderable::IndexedRenderable(Shader* shader)
	: shader(shader)
{
	gen_dynamic_vao(vao, vb, ib);
	if (shader)
	{
		buffer_data(vao, vb, ib, 0, shader->stride, 0, varr.data(), iarr.data());
		attrib_pointers(shader->attributes, shader->stride);
	}
	unbind_vao_buffers();
}

IndexedRenderable::~IndexedRenderable()
{
	delete_vao_buffers(vao, vb, ib);
}

void IndexedRenderable::set_shader(Shader* shader_)
{
	bind_vao_buffers(vao, vb, ib);
	if (shader)
		attrib_new_pointers(shader->attributes, shader->stride, shader_->attributes, shader_->stride);
	else
		attrib_pointers(shader_->attributes, shader_->stride);
	unbind_vao_buffers();
	shader = shader_;
}

void IndexedRenderable::set_num_vertices(size_t num_vertices)
{
	varr.resize(num_vertices * shader->stride);
}

size_t IndexedRenderable::num_vertices() const
{
	return varr.size() / shader->stride;
}

void IndexedRenderable::push_back_vertices(size_t num_vertices)
{
	varr.insert(varr.end(), num_vertices * shader->stride, 0.0f);
}

void IndexedRenderable::insert_vertices(size_t num_vertices, size_t pos)
{
	varr.insert(varr.begin() + pos * shader->stride, num_vertices * shader->stride, 0.0f);
}

void IndexedRenderable::fill_iarr_with_quads(size_t num_quads)
{
	iarr.resize(num_quads * 6);
	for (GLuint i = 0; i < num_quads; ++i)
	{
		iarr[i * 6    ] = 0 + 4 * i;
		iarr[i * 6 + 1] = 1 + 4 * i;
		iarr[i * 6 + 2] = 2 + 4 * i;
		iarr[i * 6 + 3] = 2 + 4 * i;
		iarr[i * 6 + 4] = 3 + 4 * i;
		iarr[i * 6 + 5] = 0 + 4 * i;
	}
}

void IndexedRenderable::push_back_quads(size_t num_quads, GLuint starting_vertex)
{
	for (GLuint i = starting_vertex; i < starting_vertex + num_quads; ++i)
	{
		iarr.insert(iarr.end(), {
			0 + 4 * i,
			1 + 4 * i,
			2 + 4 * i,
			2 + 4 * i,
			3 + 4 * i,
			0 + 4 * i
			});
	}
}

void IndexedRenderable::remove_from_varr(size_t pos)
{
	varr.erase(varr.begin() + pos * shader->stride, varr.end());
}

void IndexedRenderable::remove_from_iarr(size_t pos)
{
	iarr.erase(iarr.begin() + pos, iarr.end());
}

void IndexedRenderable::push_back_quads(size_t num_quads)
{
	GLuint starting_vertex = GLuint(iarr.size() / 6);
	for (GLuint i = starting_vertex; i < starting_vertex + num_quads; ++i)
	{
		iarr.insert(iarr.end(), {
			0 + 4 * i,
			1 + 4 * i,
			2 + 4 * i,
			2 + 4 * i,
			3 + 4 * i,
			0 + 4 * i
			});
	}
}

IndexedRenderable& IndexedRenderable::set_attribute(size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	GLfloat* buf = varr.data() + shader->attribute_offsets[attrib];
	auto num_vertices = varr.size() / shader->stride;
	for (size_t i = 0; i < num_vertices; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader->stride;
	}
	return *this;
}

IndexedRenderable& IndexedRenderable::set_attribute_single_vertex(size_t vertex, size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(varr.data() + shader->attribute_offsets[attrib] + vertex * shader->stride, v, attrib_len * sizeof(GLfloat));
	return *this;
}

IndexedRenderable& IndexedRenderable::get_attribute(size_t vertex, size_t attrib, float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(v, varr.data() + shader->attribute_offsets[attrib] + vertex * shader->stride, attrib_len * sizeof(GLfloat));
	return *this;
}

IndexedRenderable& IndexedRenderable::send_vertex_buffer()
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, varr.size() * sizeof(GLfloat), varr.data()));
	unbind_vao_buffers();
	return *this;
}

IndexedRenderable& IndexedRenderable::send_vertex_buffer_resized()
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, varr.size() * sizeof(GLfloat), varr.data(), GL_DYNAMIC_DRAW)); // LATER use GL_STATIC_DRAW when not using glBufferSubData often.
	unbind_vao_buffers();
	return *this;
}

IndexedRenderable& IndexedRenderable::send_index_buffer()
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, iarr.size() * sizeof(GLuint), iarr.data()));
	unbind_vao_buffers();
	return *this;
}

IndexedRenderable& IndexedRenderable::send_index_buffer_resized()
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, iarr.size() * sizeof(GLuint), iarr.data(), GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
	return *this;
}

IndexedRenderable& IndexedRenderable::send_single_vertex(size_t vertex)
{
	bind_vao_buffers(vao, vb, ib); // LATER only need to bind index buffer ?
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, vertex * shader->stride * sizeof(GLfloat), shader->stride * sizeof(GLfloat), varr.data() + vertex * shader->stride));
	unbind_vao_buffers();
	return *this;
}

void IndexedRenderable::send_both_buffers() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, varr.size() * sizeof(GLfloat), varr.data()));
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, iarr.size() * sizeof(GLuint), iarr.data()));
	unbind_vao_buffers();
}

void IndexedRenderable::send_both_buffers_resized() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, varr.size() * sizeof(GLfloat), varr.data(), GL_DYNAMIC_DRAW));
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, iarr.size() * sizeof(GLuint), iarr.data(), GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void IndexedRenderable::draw() const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glDrawElements(GL_TRIANGLES, GLuint(iarr.size()), GL_UNSIGNED_INT, (void*)0));
	unbind_vao_buffers();
}

void IndexedRenderable::draw(GLuint num_indexes_to_draw, size_t offset) const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glDrawElements(GL_TRIANGLES, num_indexes_to_draw, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint))));
	unbind_vao_buffers();
}
