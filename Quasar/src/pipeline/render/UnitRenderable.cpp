#include "UnitRenderable.h"

#include "variety/GLutility.h"

UnitRenderable::UnitRenderable(Shader* shader, unsigned short num_vertices)
	: shader(shader), num_vertices(num_vertices)
{
	varr = new GLfloat[num_vertices * shader->stride];
	std::memset(varr, 0, size_t(num_vertices) * shader->stride * sizeof(GLfloat));

	gen_dynamic_vao(vao, vb);
	if (shader)
	{
		buffer_data(vao, vb, num_vertices, shader->stride, varr);
		attrib_pointers(shader->attributes, shader->stride);
	}
	unbind_vao_buffers();
}

UnitRenderable::UnitRenderable(UnitRenderable&& other) noexcept
	: shader(other.shader), num_vertices(other.num_vertices), vao(other.vao), vb(other.vb), varr(other.varr)
{
	other.vao = 0;
	other.vb = 0;
	other.varr = nullptr;
}

UnitRenderable::~UnitRenderable()
{
	delete_vao_buffers(vao, vb);
	delete[] varr;
}

void UnitRenderable::set_shader(Shader* shader_)
{
	bind_vao_buffers(vao, vb);
	if (shader)
		attrib_new_pointers(shader->attributes, shader->stride, shader_->attributes, shader_->stride);
	else
		attrib_pointers(shader_->attributes, shader_->stride);
	unbind_vao_buffers();
	shader = shader_;
}

void UnitRenderable::set_attribute(size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	GLfloat* buf = varr + shader->attribute_offsets[attrib];
	for (unsigned short i = 0; i < num_vertices; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader->stride;
	}
}

void UnitRenderable::set_attribute_single_vertex(unsigned short vertex, size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(varr + shader->attribute_offsets[attrib] + vertex * shader->stride, v, attrib_len * sizeof(GLfloat));
}

void UnitRenderable::get_attribute(unsigned short vertex, size_t attrib, float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(v, varr + shader->attribute_offsets[attrib] + vertex * shader->stride, attrib_len * sizeof(GLfloat));
}

void UnitRenderable::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, size_t(num_vertices) * shader->stride * sizeof(GLfloat), varr));
	unbind_vao_buffers();
}

void UnitRenderable::send_single_vertex(unsigned short vertex) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, vertex * shader->stride, shader->stride * sizeof(GLfloat), varr + vertex * shader->stride));
	unbind_vao_buffers();
}

void UnitRenderable::send_buffer_resized() const
{
	buffer_data(vao, vb, num_vertices, shader->stride, varr);
}

void UnitRenderable::draw(unsigned short num_vertices_to_draw) const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices_to_draw == decltype(num_vertices_to_draw)(-1) ? num_vertices : num_vertices_to_draw));
	unbind_vao_buffers();
}

void UnitRenderable::set_num_vertices(unsigned short num_vertices_)
{
	num_vertices = num_vertices_;
	delete[] varr;
	varr = new GLfloat[num_vertices * shader->stride];
	std::memset(varr, 0, size_t(num_vertices) * shader->stride * sizeof(GLfloat));
	buffer_data(vao, vb, num_vertices, shader->stride, varr);
}

UnitMultiRenderable::UnitMultiRenderable(Shader* _shader, unsigned short num_units, unsigned short unit_num_vertices)
	: shader(_shader), num_units(num_units), unit_num_vertices(unit_num_vertices)
{
	auto num_vertices = num_units * unit_num_vertices;
	varr = new GLfloat[num_vertices * shader->stride];
	std::memset(varr, 0, size_t(num_vertices) * shader->stride * sizeof(GLfloat));
	
	gen_dynamic_vao(vao, vb);
	if (shader)
	{
		buffer_data(vao, vb, num_vertices, shader->stride, varr);
		attrib_pointers(shader->attributes, shader->stride);
	}
	unbind_vao_buffers();

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

void UnitMultiRenderable::set_shader(Shader* shader_)
{
	bind_vao_buffers(vao, vb);
	if (shader)
		attrib_new_pointers(shader->attributes, shader->stride, shader_->attributes, shader_->stride);
	else
		attrib_pointers(shader_->attributes, shader_->stride);
	unbind_vao_buffers();
	shader = shader_;
}

void UnitMultiRenderable::set_attribute(unsigned short unit, size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	GLfloat* buf = varr + unit * unit_num_vertices * shader->stride + shader->attribute_offsets[attrib];
	for (unsigned short i = 0; i < unit_num_vertices; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader->stride;
	}
}

void UnitMultiRenderable::set_attribute_single_vertex(unsigned short unit, unsigned short vertex, size_t attrib, const float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(varr + (unit * unit_num_vertices + vertex) * shader->stride + shader->attribute_offsets[attrib], v, attrib_len * sizeof(GLfloat));
}

void UnitMultiRenderable::get_attribute(unsigned short unit, unsigned short vertex, size_t attrib, float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(v, varr + (unit * unit_num_vertices + vertex) * shader->stride + shader->attribute_offsets[attrib], attrib_len * sizeof(GLfloat));
}

void UnitMultiRenderable::send_buffer() const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, size_t(unit_num_vertices) * num_units * shader->stride * sizeof(GLfloat), varr));
	unbind_vao_buffers();
}

void UnitMultiRenderable::send_single_unit(unsigned short unit) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, unit * unit_num_vertices * shader->stride, size_t(unit_num_vertices) * shader->stride * sizeof(GLfloat), varr + unit * unit_num_vertices * shader->stride));
	unbind_vao_buffers();
}

void UnitMultiRenderable::send_single_vertex(unsigned short unit, unsigned short vertex) const
{
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, (unit * unit_num_vertices + vertex) * shader->stride, shader->stride * sizeof(GLfloat), varr + (unit * unit_num_vertices + vertex) * shader->stride));
	unbind_vao_buffers();
}

void UnitMultiRenderable::draw(unsigned short num_units_to_draw) const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb);
	QUASAR_GL(glMultiDrawArrays(GL_TRIANGLE_STRIP, first, count, num_units_to_draw == decltype(num_units_to_draw)(-1) ? num_units : num_units_to_draw));
	unbind_vao_buffers();
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

void IndexedRenderable::push_back_vertices(size_t num_vertices)
{
	varr.insert(varr.end(), num_vertices * shader->stride, 0.0f);
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
	if (starting_vertex == -1)
		starting_vertex = GLuint(iarr.size() / 6);
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

void IndexedRenderable::set_attribute(size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	GLfloat* buf = varr.data() + shader->attribute_offsets[attrib];
	auto num_vertices = varr.size() / shader->stride;
	for (size_t i = 0; i < num_vertices; ++i)
	{
		memcpy(buf, v, attrib_len * sizeof(GLfloat));
		buf += shader->stride;
	}
}

void IndexedRenderable::set_attribute_single_vertex(size_t vertex, size_t attrib, const float* v)
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(varr.data() + shader->attribute_offsets[attrib] + vertex * shader->stride, v, attrib_len * sizeof(GLfloat));
}

void IndexedRenderable::get_attribute(size_t vertex, size_t attrib, float* v) const
{
	unsigned short attrib_len = shader->attributes[attrib];
	memcpy(v, varr.data() + shader->attribute_offsets[attrib] + vertex * shader->stride, attrib_len * sizeof(GLfloat));
}

void IndexedRenderable::send_vertex_buffer() const
{
	// TODO extract to buffer_subdata()
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, varr.size() * sizeof(GLfloat), varr.data()));
	unbind_vao_buffers();
}

void IndexedRenderable::send_vertex_buffer_resized() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, varr.size() * sizeof(GLfloat), varr.data(), GL_DYNAMIC_DRAW)); // LATER use GL_STATIC_DRAW when not using glBufferSubData often.
	unbind_vao_buffers();
}

void IndexedRenderable::send_index_buffer() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, iarr.size() * sizeof(GLuint), iarr.data()));
	unbind_vao_buffers();
}

void IndexedRenderable::send_index_buffer_resized() const
{
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, iarr.size() * sizeof(GLuint), iarr.data(), GL_DYNAMIC_DRAW));
	unbind_vao_buffers();
}

void IndexedRenderable::send_single_vertex(size_t vertex) const
{
	bind_vao_buffers(vao, vb, ib); // LATER only need to bind index buffer ?
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, vertex * shader->stride, shader->stride * sizeof(GLfloat), varr.data() + vertex * shader->stride));
	unbind_vao_buffers();
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

void IndexedRenderable::draw(size_t num_indexes_to_draw, size_t offset) const
{
	bind_shader(*shader);
	bind_vao_buffers(vao, vb, ib);
	QUASAR_GL(glDrawElements(GL_TRIANGLES, GLuint(num_indexes_to_draw == decltype(num_indexes_to_draw)(-1) ? iarr.size() : num_indexes_to_draw), GL_UNSIGNED_INT, (void*)offset));
	unbind_vao_buffers();
}
