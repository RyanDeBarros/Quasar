#pragma once

#include <gl/glew.h>

#include <vector>

#include "Shader.h"

struct UnitRenderable
{
	GLfloat* varr = nullptr;
	GLuint vao = 0, vb = 0;
	Shader* shader = nullptr; // LATER std::shared_ptr for shaders?
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
	const UnitRenderable& set_attribute(size_t attrib, const float* v) const;
	const UnitRenderable& set_attribute_single_vertex(unsigned short vertex, size_t attrib, const float* v) const;
	const UnitRenderable& get_attribute(unsigned short vertex, size_t attrib, float* v) const;
	UnitRenderable& set_attribute(size_t attrib, const float* v);
	UnitRenderable& set_attribute_single_vertex(unsigned short vertex, size_t attrib, const float* v);
	UnitRenderable& get_attribute(unsigned short vertex, size_t attrib, float* v);
	void send_buffer() const;
	const UnitRenderable& send_single_vertex(unsigned short vertex) const;
	UnitRenderable& send_single_vertex(unsigned short vertex);
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
	const IndexedRenderable& get_attribute(size_t vertex, size_t attrib, float* v) const;
	IndexedRenderable& get_attribute(size_t vertex, size_t attrib, float* v);
	const IndexedRenderable& send_vertex_buffer() const;
	IndexedRenderable& send_vertex_buffer();
	const IndexedRenderable& send_vertex_buffer_resized() const;
	IndexedRenderable& send_vertex_buffer_resized();
	const IndexedRenderable& send_index_buffer() const;
	IndexedRenderable& send_index_buffer();
	const IndexedRenderable& send_index_buffer_resized() const;
	IndexedRenderable& send_index_buffer_resized();
	const IndexedRenderable& send_single_vertex(size_t vertex) const;
	IndexedRenderable& send_single_vertex(size_t vertex);
	void send_both_buffers() const;
	void send_both_buffers_resized() const;
	void draw(size_t num_indexes_to_draw = -1, size_t offset = 0) const;
};
