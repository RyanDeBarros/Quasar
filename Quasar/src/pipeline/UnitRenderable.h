#pragma once

#include <gl/glew.h>

#include "Shader.h"

struct UnitRenderable
{
	GLfloat* varr = nullptr;
	GLuint vao = 0, vb = 0;
	const Shader& shader;
	const unsigned char num_vertices;

	UnitRenderable(const Shader& shader, unsigned char num_vertices = 4);
	UnitRenderable(const UnitRenderable&) = delete;
	UnitRenderable(UnitRenderable&&) noexcept = delete;
	~UnitRenderable();

	bool set_attribute(size_t attrib, const float* v) const;
	bool set_attribute_single_vertex(unsigned char vertex, size_t attrib, const float* v) const;
	void send_buffer() const;
	void send_single_vertex(unsigned char vertex) const;
	void draw() const;
};

struct UnitMultiRenderable
{
	GLfloat* varr = nullptr;
	GLuint vao = 0, vb = 0;
	const Shader& shader;
	GLint* first;
	GLsizei* count;
	const unsigned short num_units;
	const unsigned char unit_num_vertices;

	UnitMultiRenderable(const Shader& shader, unsigned short num_units, unsigned char unit_num_vertices = 4);
	UnitMultiRenderable(const UnitMultiRenderable&) = delete;
	UnitMultiRenderable(UnitMultiRenderable&&) noexcept = delete;
	~UnitMultiRenderable();

	bool set_attribute(unsigned short unit, size_t attrib, const float* v) const;
	bool set_attribute_single_vertex(unsigned short unit, unsigned char vertex, size_t attrib, const float* v) const;
	void send_buffer() const;
	void send_single_unit(unsigned short unit) const;
	void send_single_vertex(unsigned short unit, unsigned char vertex) const;
	void draw() const;
};
