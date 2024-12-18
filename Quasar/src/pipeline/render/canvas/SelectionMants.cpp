#include "SelectionMants.h"

#include <glm/gtc/type_ptr.hpp>

#include "../Uniforms.h"

// LATER re-add parallel offset in geom shader, but provide additional data about which ends should use that offset.
SelectionMants::SelectionMants()
	: W_UnitRenderable(nullptr),
	shader(FileSystem::shader_path("canvas/marching_ants.vert"), FileSystem::shader_path("canvas/marching_ants.geom"), FileSystem::shader_path("canvas/marching_ants.frag"))
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

bool SelectionMants::add(IPosition pos)
{
	if (points.contains(pos))
		return false;
	points.insert(pos);
	if (pos.x >= 0 && pos.x < cols - 1 && pos.y >= 0 && pos.y < rows - 1)
		shader_add(pos);
	return true;
}

bool SelectionMants::remove(IPosition pos)
{
	if (!points.contains(pos))
		return false;
	points.erase(pos);
	if (pos.x >= 0 && pos.x < cols - 1 && pos.y >= 0 && pos.y < rows - 1)
		shader_remove(pos);
	return true;
}

void SelectionMants::shader_add(IPosition pos)
{
	static const float on_pos = 1.0f;
	static const float on_neg = -1.0f;
	static const float off = 0.0f;
	if (pos.x > 0 && points.contains({ pos.x - 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &on_pos);
	if (pos.x < cols - 2 && points.contains({ pos.x + 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &on_neg);
	if (pos.y > 0 && points.contains({ pos.x, pos.y - 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &on_neg);
	if (pos.y < rows - 2 && points.contains({ pos.x, pos.y + 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &off);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &on_pos);
}

void SelectionMants::shader_remove(IPosition pos)
{
	static const float on_pos = 1.0f;
	static const float on_neg = -1.0f;
	static const float off = 0.0f;
	if (pos.x > 0 && points.contains({ pos.x - 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &on_neg);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x, pos.y), 1, &off);
	if (pos.x < cols - 2 && points.contains({ pos.x + 1, pos.y }))
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &on_pos);
	else
		ur->set_attribute_single_vertex(vertex_vertical(pos.x + 1, pos.y), 1, &off);
	if (pos.y > 0 && points.contains({ pos.x, pos.y - 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &on_pos);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y), 1, &off);
	if (pos.y < rows - 2 && points.contains({ pos.x, pos.y + 1 }))
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &on_neg);
	else
		ur->set_attribute_single_vertex(vertex_horizontal(pos.x, pos.y + 1), 1, &off);
}

IntBounds SelectionMants::clear()
{
	IntBounds bbox = IntBounds::INVALID;
	IPosition pos{};
	while (!points.empty())
	{
		pos = *points.begin();
		remove(pos);
		update_bbox(bbox, pos.x, pos.y);
	}
	return bbox;
}

void SelectionMants::draw()
{
	ur->draw_as_lines();
}

void SelectionMants::send_uniforms() const
{
	Uniforms::send_4(shader, "uColor1", color1.as_vec());
	Uniforms::send_4(shader, "uColor2", color2.as_vec());
	Uniforms::send_1(shader, "uSpeed", speed);
}

void SelectionMants::send_buffer(IntBounds bbox)
{
	if (bbox != IntBounds::INVALID)
	{
		if (!intersection(bbox, { 0, cols - 2, 0, rows - 2 }, bbox))
			return;
		int subrows = bbox.y2 - bbox.y1 + 2;
		int subcols = bbox.x2 - bbox.x1 + 2;
		for (int dy = 0; dy < subrows; ++dy)
			ur->send_subbuffer(vertex_horizontal(bbox.x1, bbox.y1 + dy), subcols);
		for (int dx = 0; dx < subcols; ++dx)
			ur->send_subbuffer(vertex_vertical(bbox.x1 + dx, bbox.y1), subrows);
	}
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

void SelectionMants::flip_direction()
{
	speed *= -1;
	Uniforms::send_1(shader, "uSpeed", speed);
}

unsigned int SelectionMants::vertex_horizontal(int x, int y) const
{
	return y * cols + x;
}

unsigned int SelectionMants::vertex_vertical(int x, int y) const
{
	return y + (x + cols) * rows;
}
