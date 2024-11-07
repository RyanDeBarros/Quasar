#pragma once

#include <gl/glew.h>

#include <vector>

#include "Shader.h"

struct UnitRenderable
{
	GLfloat* varr = nullptr;
	GLuint vao = 0, vb = 0;
	Shader* shader = nullptr;
private:
	unsigned short num_vertices;
public:
	unsigned short get_num_vertices() const { return num_vertices; }

	UnitRenderable(Shader* shader, unsigned short num_vertices = 4);
	UnitRenderable(const UnitRenderable&) = delete;
	UnitRenderable(UnitRenderable&&) noexcept;
	UnitRenderable& operator=(UnitRenderable&&) noexcept = delete;
	~UnitRenderable();

	void set_shader(Shader* shader);
	void set_attribute(size_t attrib, const float* v) const;
	void set_attribute_single_vertex(unsigned short vertex, size_t attrib, const float* v) const;
	void get_attribute(unsigned short vertex, size_t attrib, float* v) const;
	void send_buffer() const;
	void send_single_vertex(unsigned short vertex) const;
	void send_buffer_resized() const;
	void draw(unsigned short num_vertices_to_draw = -1) const;
	void set_num_vertices(unsigned short num_vertices);
};

struct UnitMultiRenderable
{
	GLfloat* varr = nullptr;
	GLuint vao = 0, vb = 0;
	Shader* shader;
	GLint* first;
	GLsizei* count;
	// LATER variable num_units/unit_num_vertices
	const unsigned short num_units;
	const unsigned short unit_num_vertices;

	UnitMultiRenderable(Shader* shader, unsigned short num_units, unsigned short unit_num_vertices = 4);
	UnitMultiRenderable(const UnitMultiRenderable&) = delete;
	UnitMultiRenderable(UnitMultiRenderable&&) noexcept = delete;
	~UnitMultiRenderable();

	void set_shader(Shader* shader);
	void set_attribute(unsigned short unit, size_t attrib, const float* v) const;
	void set_attribute_single_vertex(unsigned short unit, unsigned short vertex, size_t attrib, const float* v) const;
	void get_attribute(unsigned short unit, unsigned short vertex, size_t attrib, float* v) const;
	void send_buffer() const;
	void send_single_unit(unsigned short unit) const;
	void send_single_vertex(unsigned short unit, unsigned short vertex) const;
	void send_buffer_resized() const;
	void draw(unsigned short num_units_to_draw = -1) const;
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
	void push_back_vertices(size_t num_vertices);
	void fill_iarr_with_quads(size_t num_quads);
	void push_back_quads(size_t num_quads, GLuint starting_vertex = -1);
	void set_attribute(size_t attrib, const float* v);
	void set_attribute_single_vertex(size_t vertex, size_t attrib, const float* v);
	void get_attribute(size_t vertex, size_t attrib, float* v) const;
	void send_vertex_buffer() const;
	void send_vertex_buffer_resized() const;
	void send_index_buffer() const;
	void send_index_buffer_resized() const;
	void send_single_vertex(size_t vertex) const;
	void send_both_buffers() const;
	void send_both_buffers_resized() const;
	void draw(size_t num_indexes_to_draw = -1, size_t offset = 0) const;
};
