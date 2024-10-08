#include "Renderer.h"

#include <sstream>

#include "Sprite.h"
#include "GLutility.h"
#include "IO.h"

unsigned short QuasarSettings::VERTEX_COUNT = 512;
unsigned short QuasarSettings::INDEX_COUNT = 768;
unsigned short QuasarSettings::TEXTURES_COUNT = 32;

void QuasarSettings::load_settings(const char* filepath)
{
	auto values = IO::load_asset(filepath, "settings");
	auto iter = values.find("VERTEX_COUNT");
	if (iter != values.end())
		VERTEX_COUNT = iter->second.parse<unsigned short>();
	iter = values.find("INDEX_COUNT");
	if (iter != values.end())
		INDEX_COUNT = iter->second.parse<unsigned short>();
	iter = values.find("TEXTURES_COUNT");
	if (iter != values.end())
		TEXTURES_COUNT = iter->second.parse<unsigned short>();
}

Renderer::Renderer(GLFWwindow* window, Shader&& shader_)
	: window(window), shader(shader_), vertex_pool(new GLfloat[QuasarSettings::VERTEX_COUNT]),
	index_pool(new GLuint[QuasarSettings::INDEX_COUNT]), texture_slots(new GLuint[QuasarSettings::TEXTURES_COUNT])
{
	reset();

	QUASAR_GL(glGenVertexArrays(1, &vao));
	QUASAR_GL(glBindVertexArray(vao));

	QUASAR_GL(glGenBuffers(1, &vb));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, vb));
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, QuasarSettings::VERTEX_COUNT * sizeof(GLfloat), vertex_pool, GL_DYNAMIC_DRAW));
	QUASAR_GL(glGenBuffers(1, &ib));
	QUASAR_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib));
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, QuasarSettings::INDEX_COUNT * sizeof(GLuint), index_pool, GL_DYNAMIC_DRAW));

	attrib_pointers(shader.attributes, shader.stride);
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
	delete[] texture_slots;

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
static T* advance_bytes(T* ptr, size_t bytes)
{
	return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(ptr) + bytes);
}

void Renderer::prepare_for_sprite()
{
	if (advance_bytes(vertex_pos, Sprite::VLEN_BYTES) - vertex_pool >= QuasarSettings::VERTEX_COUNT
		|| advance_bytes(index_pos, Sprite::ILEN_BYTES) - index_pool >= QuasarSettings::INDEX_COUNT)
	{
		flush();
		reset();
	}
}

void Renderer::pool_over_varr(GLfloat* varr)
{
	memcpy(vertex_pos, varr, Sprite::VLEN_BYTES);
	for (size_t i = 0; i < Sprite::NUM_INDICES; ++i)
		*index_pos++ = Sprite::IARR[i] + num_vertices;
	num_vertices += Sprite::NUM_VERTICES;
	vertex_pos = advance_bytes(vertex_pos, Sprite::VLEN_BYTES);
}

void Renderer::on_draw()
{
	reset();
	for (size_t i = 0; i < _sprites.size(); ++i)
		_sprites[i]->on_draw(this);
	flush();
}

void Renderer::flush() const
{
	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, (vertex_pos - vertex_pool) * sizeof(decltype(vertex_pool)), vertex_pool));
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, (index_pos - index_pool) * sizeof(decltype(index_pool)), index_pool));
	QUASAR_GL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_pos - index_pool), GL_UNSIGNED_INT, 0));
}

void Renderer::reset()
{
	vertex_pos = vertex_pool;
	index_pos = index_pool;
	num_vertices = 0;
	texture_slot_cap = 0;
}

void Renderer::set_view(const Transform& view_)
{
	view = view_;
	glm::mat3 cameraVP = projection * view.inverse();
	bind();
	QUASAR_GL(glUniformMatrix3fv(shader.locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
}

unsigned short Renderer::get_texture_slot(GLuint texture)
{
	for (unsigned char i = 0; i < texture_slot_cap; ++i)
	{
		if (texture_slots[i] == texture)
			return i;
	}
	if (texture_slot_cap == QuasarSettings::TEXTURES_COUNT)
	{
		flush();
		reset();
	}
	texture_slots[texture_slot_cap] = texture;
	return texture_slot_cap++;
}
