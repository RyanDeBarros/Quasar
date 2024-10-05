#pragma once

#include "Macros.h"
#include "Referencers.h"
#include "Geometry.h"
#include "Shader.h"

constexpr unsigned short QUASAR_MAX_VERTICES = 512;
constexpr unsigned short QUASAR_MAX_INDEXES = 1024;

class Renderer
{
	GLfloat* vertex_pool = nullptr;
	GLuint* index_pool = nullptr;
	GLfloat* vertex_pos = nullptr;
	GLuint* index_pos = nullptr;
	GLFWwindow* window;
	glm::mat3 projection;
	Transform view;
	GLuint vao = 0, vb = 0, ib = 0;
	Shader shader;

public:
	Renderer(GLFWwindow* window, Shader&& shader);
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	~Renderer();

	void bind() const;
	void pool_over_buffer(const BufferReferencer& buffer);
	void on_draw();
	const Transform& get_view() const { return view; }
	void set_view(const Transform& view);
};
