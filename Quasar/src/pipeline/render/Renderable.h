#pragma once

#include <gl/glew.h>

#include <vector>

#include "Shader.h"

struct UnitRenderable
{
	std::vector<GLfloat> varr;
	GLuint vao = 0, vb = 0;
	Shader* shader = nullptr; // LATER std::shared_ptr for shaders?

	UnitRenderable(Shader* shader, unsigned short num_vertices = 4);
	UnitRenderable(const UnitRenderable&) = delete;
	UnitRenderable(UnitRenderable&&) noexcept;
	UnitRenderable& operator=(UnitRenderable&&) noexcept = delete;
	~UnitRenderable();

	void set_shader(Shader* shader);
	const UnitRenderable& get_attribute(unsigned int vertex, size_t attrib, float* v) const;
	UnitRenderable& set_attribute(size_t attrib, const float* v);
	UnitRenderable& set_attribute_single_vertex(unsigned int vertex, size_t attrib, const float* v);
	UnitRenderable& get_attribute(unsigned int vertex, size_t attrib, float* v);
	void send_buffer() const;
	const UnitRenderable& send_single_vertex(unsigned int vertex) const;
	UnitRenderable& send_single_vertex(unsigned int vertex);
	void send_buffer_resized() const;
	void draw() const;
	void draw(unsigned int num_vertices_to_draw) const;
	void draw_as_lines() const;
	size_t num_vertices() const { return shader ? varr.size() / shader->stride : 0; }
	void set_num_vertices(size_t num_vertices);
};

struct UnitMultiRenderable
{
	std::vector<GLfloat> varr;
	GLuint vao = 0, vb = 0;
	Shader* shader;
	std::vector<GLint> first;
	std::vector<GLsizei> count;
	unsigned int num_units;
	unsigned int unit_num_vertices;

	UnitMultiRenderable(Shader* shader, unsigned int num_units, unsigned int unit_num_vertices = 4);
	UnitMultiRenderable(const UnitMultiRenderable&) = delete;
	UnitMultiRenderable(UnitMultiRenderable&&) noexcept = delete;
	~UnitMultiRenderable();

	void set_shader(Shader* shader);
	void set_attribute(unsigned int unit, size_t attrib, const float* v);
	void set_attribute_single_vertex(unsigned int unit, unsigned int vertex, size_t attrib, const float* v);
	void get_attribute(unsigned int unit, unsigned int vertex, size_t attrib, float* v) const;
	void send_buffer() const;
	void send_single_unit(unsigned int unit) const;
	void send_single_vertex(unsigned int unit, unsigned int vertex) const;
	void send_buffer_resized() const;
	void draw() const;
	void draw(unsigned int num_units_to_draw) const;
	void resize();
};

struct IndexedRenderable
{
	std::vector<GLfloat> varr;
	std::vector<GLuint> iarr;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader* shader;

	IndexedRenderable(Shader* shader);
	IndexedRenderable(const IndexedRenderable&) = delete; // LATER copy/move semantics throughout project
	IndexedRenderable(IndexedRenderable&&) noexcept = delete;
	~IndexedRenderable();

	void set_shader(Shader* shader);
	void set_num_vertices(size_t num_vertices);
	size_t num_vertices() const;
	void push_back_vertices(size_t num_vertices);
	void insert_vertices(size_t num_vertices, size_t pos);
	void fill_iarr_with_quads(size_t num_quads);
	void push_back_quads(size_t num_quads);
	void push_back_quads(size_t num_quads, GLuint starting_vertex);
	void remove_from_varr(size_t pos);
	void remove_from_iarr(size_t pos);
	IndexedRenderable& set_attribute(size_t attrib, const float* v);
	IndexedRenderable& set_attribute_single_vertex(size_t vertex, size_t attrib, const float* v);
	IndexedRenderable& get_attribute(size_t vertex, size_t attrib, float* v);
	IndexedRenderable& send_vertex_buffer();
	IndexedRenderable& send_vertex_buffer_resized();
	IndexedRenderable& send_index_buffer();
	IndexedRenderable& send_index_buffer_resized();
	IndexedRenderable& send_single_vertex(size_t vertex);
	void send_both_buffers() const;
	void send_both_buffers_resized() const;
	void draw() const;
	void draw(GLuint num_indexes_to_draw, size_t offset) const;
};
