#include "Renderer.h"

#include <sstream>

#include "Sprite.h"
#include "variety/GLutility.h"
#include "variety/IO.h"
#include "user/Machine.h"

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

Renderer::Renderer(Window* window, Shader&& shader_)
	: window(window), shader(std::move(shader_)), vertex_pool(new GLfloat[QuasarSettings::VERTEX_COUNT]),
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
	set_projection();
	shader.query_location("u_VP");
	send_view();

	clip.x = 0;
	clip.y = 0;
	clip.screen_w = window->width();
	clip.screen_h = window->height();
	clip.window_size_to_bounds = [](int width, int height) -> glm::ivec4 { return { 0, 0, width, height }; };
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

void Renderer::set_projection(float width, float height)
{
	projection = glm::ortho<float>(0.0f, width * app_scale.x, 0.0f, height * app_scale.y);
}

void Renderer::set_projection()
{
	projection = glm::ortho<float>(0.0f, window->width() * app_scale.x, 0.0f, window->height() * app_scale.y);
}

void Renderer::bind() const
{
	QUASAR_GL(glBindVertexArray(vao));
	QUASAR_GL(glUseProgram(shader.rid));
	QUASAR_GL(glEnable(GL_BLEND));
	QUASAR_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	window->focus_context();
	QUASAR_GL(glScissor(clip.x, clip.y, clip.screen_w, clip.screen_h));
}

void Renderer::unbind() const
{
	QUASAR_GL(glBindVertexArray(0));
	QUASAR_GL(glUseProgram(0));
	QUASAR_GL(glScissor(0, 0, window->width(), window->height()));
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

void Renderer::on_render()
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

void Renderer::send_view()
{
	glm::mat3 cameraVP = projection * view.camera();
	bind();
	QUASAR_GL(glUniformMatrix3fv(shader.locations["u_VP"], 1, GL_FALSE, &cameraVP[0][0]));
}

void Renderer::set_view(const Transform& view_)
{
	view = view_;
	send_view();
}

void Renderer::set_view_position(const Position& pos)
{
	view.position = pos;
	send_view();
}

void Renderer::set_view_rotation(Rotation rot)
{
	view.rotation = rot;
	send_view();
}

void Renderer::set_view_scale(const Scale& sca)
{
	view.scale = sca;
	send_view();
}

glm::vec2 Renderer::to_world_coordinates(const glm::vec2& screen_coordinates) const
{
	glm::vec3 ndc{};
	ndc.x = 1.0f - 2.0f * (screen_coordinates.x / window->width());
	ndc.y = 1.0f - 2.0f * (screen_coordinates.y / window->height());
	ndc.z = 1.0f;

	glm::mat3 invVP = glm::inverse(projection * view.camera());
	glm::vec3 world_pos = invVP * ndc;

	if (world_pos.z != 0.0f)
		world_pos / world_pos.z;

	return glm::vec2{ -world_pos.x, -world_pos.y };
}

glm::vec2 Renderer::to_screen_coordinates(const glm::vec2& world_coordinates) const
{
	glm::vec3 world_pos{ world_coordinates.x, -world_coordinates.y, 1.0f };
	glm::mat3 VP = projection * view.camera();
	glm::vec3 clip_space_pos = VP * world_pos;
	glm::vec2 screen_coo{};
	screen_coo.x = (1.0f + clip_space_pos.x) * 0.5f * window->width();
	screen_coo.y = (1.0f + clip_space_pos.y) * 0.5f * window->height();
	return screen_coo;
}

void Renderer::set_app_scale(float x, float y)
{
	app_scale.x = 1.0f / x;
	app_scale.y = 1.0f / y;
	set_projection();
	send_view();
	// TODO scale cursor? Have different discrete cursor sizes (only works for custom cursors).
}

glm::vec2 Renderer::get_app_scale() const
{
	return 1.0f / app_scale;
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

void Renderer::set_window_resize_callback()
{
	window->clbk_window_size.push_back([this](const Callback::WindowSize& ws) {
		QUASAR_GL(glViewport(0, 0, ws.width, ws.height));
		clip.update_window_size(ws.width, ws.height);
		set_projection(float(ws.width), float(ws.height));
		send_view();
		Machine.on_render();
		});
}
