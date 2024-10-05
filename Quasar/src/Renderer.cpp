#include "Renderer.h"

#include "Shader.h"

Renderer::Renderer(GLFWwindow* window, Shader&& shader_)
	: window(window), shader(shader_)
{
	vertex_pool = new GLfloat[QUASAR_MAX_VERTICES];
	index_pool = new GLuint[QUASAR_MAX_INDEXES];
	vertex_pos = vertex_pool;
	index_pos = index_pool;

	QUASAR_GL(glGenVertexArrays(1, &vao));
	QUASAR_GL(glBindVertexArray(vao));

	QUASAR_GL(glGenBuffers(1, &vb));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, vb));
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, QUASAR_MAX_VERTICES * sizeof(GLfloat), vertex_pool, GL_DYNAMIC_DRAW));
	QUASAR_GL(glGenBuffers(1, &ib));
	QUASAR_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib));
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, QUASAR_MAX_INDEXES * sizeof(GLuint), index_pool, GL_DYNAMIC_DRAW));

	int ww, wh;
	glfwGetWindowSize(window, &ww, &wh);
	projection = glm::ortho<float>(0.0f, static_cast<float>(ww), 0.0f, static_cast<float>(wh));
	shader.query_location("u_VP");
	set_view(view);
}

Renderer::~Renderer()
{
	delete[] vertex_pool;
	delete[] index_pool;

	QUASAR_GL(glDeleteBuffers(1, &vao));
	QUASAR_GL(glDeleteBuffers(1, &vb));
	QUASAR_GL(glDeleteBuffers(1, &ib));
}

void Renderer::bind() const
{
	QUASAR_GL(glBindVertexArray(vao));
	QUASAR_GL(glUseProgram(shader.rid));
	QUASAR_GL(glEnable(GL_BLEND));
	QUASAR_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

template<typename T>
static void advance_bytes(T*& ptr, size_t bytes)
{
	ptr = reinterpret_cast<T*>(reinterpret_cast<std::byte*>(ptr) + bytes);
}

void Renderer::pool_over_buffer(const BufferReferencer& buffer)
{
	// TODO bounds checking
	memcpy(vertex_pos, buffer.varr, buffer.vlen_bytes);
	memcpy(index_pos, buffer.iarr, buffer.ilen_bytes);
	advance_bytes(vertex_pos, buffer.vlen_bytes);
	advance_bytes(index_pos, buffer.ilen_bytes);
}

void Renderer::on_draw()
{
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, (vertex_pos - vertex_pool) * sizeof(decltype(vertex_pool)), vertex_pool));
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, (index_pos - index_pool) * sizeof(decltype(index_pool)), index_pool));
	QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}

void Renderer::set_view(const Transform& view_)
{
	view = view_;
	glm::mat3 cameraVP = projection * view.inverse();
	bind();
	QUASAR_GL(glUniformMatrix3fv(shader.locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
}
