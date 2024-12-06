#include "SelectionMants.h"

#include <glm/gtc/type_ptr.hpp>

#include "Uniforms.h"

SelectionMants::SelectionMants()
	: W_UnitRenderable(nullptr),
	shader(FileSystem::shader_path("marching_ants.vert"), FileSystem::shader_path("marching_ants.geom"), FileSystem::shader_path("marching_ants.frag"))
{
	ur->set_shader(&shader);
}

void SelectionMants::set_size(int width, int height)
{
	points.clear();
	cols = width + 1;
	rows = height + 1;
	ur->varr.resize(2 * rows * cols * size_t(shader.stride), 0);
	const Position origin{ -width * 0.5f, -height * 0.5f };
	static const float off = 0.0f;
	
	// horizontal lines
	for (int y = 0; y < rows; ++y)
	{
		for (int x = 0; x < cols; ++x)
		{
			ur->set_attribute_single_vertex(vertex_horizontal(x, y), 0, glm::value_ptr(glm::vec2(origin) + glm::vec2{ x, y }));
			ur->set_attribute_single_vertex(vertex_horizontal(x, y), 1, &off);
		}
	}

	// vertical lines
	for (int x = 0; x < cols; ++x)
	{
		for (int y = 0; y < rows; ++y)
		{
			ur->set_attribute_single_vertex(vertex_vertical(x, y), 0, glm::value_ptr(glm::vec2(origin) + glm::vec2{ x, y }));
			ur->set_attribute_single_vertex(vertex_vertical(x, y), 1, &off);
		}
	}

	ur->send_buffer_resized();
}

void SelectionMants::add(IPosition pos)
{
	if (points.count(pos))
		return;
	points.insert(pos);
	static const float on_pos = 1.0f;
	static const float on_neg = -1.0f;
	static const float off = 0.0f;
	if (points.count({ pos.x - 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &on_pos);
	if (points.count({ pos.x + 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &on_neg);
	if (points.count({ pos.x, pos.y - 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &on_neg);
	if (points.count({ pos.x, pos.y + 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &on_pos);
}

void SelectionMants::remove(IPosition pos)
{
	auto iter = points.find(pos);
	if (iter == points.end())
		return;
	points.erase(iter);
	static const float on_pos = 1.0f;
	static const float on_neg = -1.0f;
	static const float off = 0.0f;
	if (points.count({ pos.x - 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &on_neg);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &off);
	if (points.count({ pos.x + 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &on_pos);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &off);
	if (points.count({ pos.x, pos.y - 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &on_pos);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &off);
	if (points.count({ pos.x, pos.y + 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &on_neg);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &off);
}

void SelectionMants::draw()
{
	ur->draw_as_lines();
}

void SelectionMants::send_vp(const glm::mat3& vp) const
{
	Uniforms::send_matrix3(shader, "uVP", vp);
}

void SelectionMants::send_time(float time) const
{
	Uniforms::send_1(shader, "uTime", time);
}

void SelectionMants::send_screen_size(glm::ivec2 size) const
{
	Uniforms::send_2(shader, "uScreenSize", size);
}

unsigned int SelectionMants::vertex_horizontal(int x, int y) const
{
	return y * cols + x;
}

unsigned int SelectionMants::vertex_vertical(int x, int y) const
{
	return y + (x + cols) * rows;
}
