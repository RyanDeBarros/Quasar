#include "ImplUtility.h"

#include <glm/gtc/type_ptr.hpp>

#include "variety/Utils.h"
#include "user/Platform.h"
#include "edit/image/PixelBuffer.h"
#include "pipeline/widgets/Widget.h"

std::shared_ptr<Cursor> Utils::cursor_from_buffer(const Buffer& buffer, float xhot, float yhot)
{
	return std::make_shared<Cursor>(buffer.pixels, buffer.width, buffer.height, roundi(buffer.width * xhot), roundi(buffer.height * yhot));
}

std::shared_ptr<Cursor> Utils::cursor_from_buffer(const Buffer& buffer, int xhot, int yhot)
{
	return std::make_shared<Cursor>(buffer.pixels, buffer.width, buffer.height, xhot, yhot);
}

void Utils::set_vertex_pos_attributes(UnitRenderable& ur, FlatTransform tr, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ur, tr.left(), tr.right(), tr.bottom(), tr.top(), vertex_offset, attrib, send_buffer);
}

void Utils::set_vertex_pos_attributes(IndexedRenderable& ir, FlatTransform tr, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ir, tr.left(), tr.right(), tr.bottom(), tr.top(), vertex_offset, attrib, send_buffer);
}

void Utils::set_vertex_pos_attributes(UnitRenderable& ur, const WidgetPlacement& wp, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ur, wp.left(), wp.right(), wp.bottom(), wp.top(), vertex_offset, attrib, send_buffer);
}

void Utils::set_vertex_pos_attributes(IndexedRenderable& ir, const WidgetPlacement& wp, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ir, wp.left(), wp.right(), wp.bottom(), wp.top(), vertex_offset, attrib, send_buffer);
}

void Utils::set_vertex_pos_attributes(UnitRenderable& ur, const glm::mat3& mat3, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ur, wp_left(mat3), wp_right(mat3), wp_bottom(mat3), wp_top(mat3), vertex_offset, attrib, send_buffer);
}

void Utils::set_vertex_pos_attributes(IndexedRenderable& ir, const glm::mat3& mat3, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ir, wp_left(mat3), wp_right(mat3), wp_bottom(mat3), wp_top(mat3), vertex_offset, attrib, send_buffer);
}

void Utils::set_uv_attributes(UnitRenderable& ur, float x1, float x2, float y1, float y2, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	ur.set_attribute_single_vertex(vertex_offset + 0, attrib, glm::value_ptr(glm::vec2{ x1, y1 }))
		.set_attribute_single_vertex(vertex_offset + 1, attrib, glm::value_ptr(glm::vec2{ x2, y1 }))
		.set_attribute_single_vertex(vertex_offset + 2, attrib, glm::value_ptr(glm::vec2{ x1, y2 }))
		.set_attribute_single_vertex(vertex_offset + 3, attrib, glm::value_ptr(glm::vec2{ x2, y2 }));
	if (send_buffer)
		ur.send_buffer();
}

void Utils::set_uv_attributes(UnitRenderable& ur, Bounds bounds, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ur, bounds.x1, bounds.x2, bounds.y1, bounds.y2, vertex_offset, attrib, send_buffer);
}

void Utils::set_uv_attributes(IndexedRenderable& ir, float x1, float x2, float y1, float y2, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	ir.set_attribute_single_vertex(vertex_offset + 0, attrib, glm::value_ptr(glm::vec2{ x1, y1 }))
		.set_attribute_single_vertex(vertex_offset + 1, attrib, glm::value_ptr(glm::vec2{ x2, y1 }))
		.set_attribute_single_vertex(vertex_offset + 2, attrib, glm::value_ptr(glm::vec2{ x2, y2 }))
		.set_attribute_single_vertex(vertex_offset + 3, attrib, glm::value_ptr(glm::vec2{ x1, y2 }));
	if (send_buffer)
		ir.send_single_vertex(vertex_offset + 0)
			.send_single_vertex(vertex_offset + 1)
			.send_single_vertex(vertex_offset + 2)
			.send_single_vertex(vertex_offset + 3);
}

void Utils::set_uv_attributes(IndexedRenderable& ir, Bounds bounds, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	set_uv_attributes(ir, bounds.x1, bounds.x2, bounds.y1, bounds.y2, vertex_offset, attrib, send_buffer);
}

void Utils::set_four_attributes(UnitRenderable& ur, float val, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	ur.set_attribute_single_vertex(vertex_offset + 0, attrib, &val)
		.set_attribute_single_vertex(vertex_offset + 1, attrib, &val)
		.set_attribute_single_vertex(vertex_offset + 2, attrib, &val)
		.set_attribute_single_vertex(vertex_offset + 3, attrib, &val);
	if (send_buffer)
		ur.send_buffer();
}

void Utils::set_four_attributes(UnitRenderable& ur, const float* val, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	ur.set_attribute_single_vertex(vertex_offset + 0, attrib, val)
		.set_attribute_single_vertex(vertex_offset + 1, attrib, val)
		.set_attribute_single_vertex(vertex_offset + 2, attrib, val)
		.set_attribute_single_vertex(vertex_offset + 3, attrib, val);
	if (send_buffer)
		ur.send_buffer();
}

void Utils::set_four_attributes(IndexedRenderable& ir, float val, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	ir.set_attribute_single_vertex(vertex_offset + 0, attrib, &val)
		.set_attribute_single_vertex(vertex_offset + 1, attrib, &val)
		.set_attribute_single_vertex(vertex_offset + 2, attrib, &val)
		.set_attribute_single_vertex(vertex_offset + 3, attrib, &val);
	if (send_buffer)
		ir.send_single_vertex(vertex_offset + 0)
		.send_single_vertex(vertex_offset + 1)
		.send_single_vertex(vertex_offset + 2)
		.send_single_vertex(vertex_offset + 3);
}

void Utils::set_four_attributes(IndexedRenderable& ir, const float* val, unsigned short vertex_offset, size_t attrib, bool send_buffer)
{
	ir.set_attribute_single_vertex(vertex_offset + 0, attrib, val)
		.set_attribute_single_vertex(vertex_offset + 1, attrib, val)
		.set_attribute_single_vertex(vertex_offset + 2, attrib, val)
		.set_attribute_single_vertex(vertex_offset + 3, attrib, val);
	if (send_buffer)
		ir.send_single_vertex(vertex_offset + 0)
		.send_single_vertex(vertex_offset + 1)
		.send_single_vertex(vertex_offset + 2)
		.send_single_vertex(vertex_offset + 3);
}

void Utils::set_rect_dimension_attributes(UnitRenderable& ur, FlatTransform tr, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer)
{
	set_rect_dimension_attributes(ur, tr.left(), tr.right(), tr.bottom(), tr.top(), bottom_left_attrib, rect_size_attrib, send_buffer);
}

void Utils::set_rect_dimension_attributes(UnitRenderable& ur, const WidgetPlacement& wp, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer)
{
	set_rect_dimension_attributes(ur, wp.left(), wp.right(), wp.bottom(), wp.top(), bottom_left_attrib, rect_size_attrib, send_buffer);
}

void Utils::set_rect_dimension_attributes(UnitRenderable& ur, const glm::mat3& mat3, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer)
{
	set_rect_dimension_attributes(ur, wp_left(mat3), wp_right(mat3), wp_bottom(mat3), wp_top(mat3), bottom_left_attrib, rect_size_attrib, send_buffer);
}

void Utils::set_rect_dimension_attributes(UnitRenderable& ur, float left, float right, float bottom, float top, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer)
{
	ur.set_attribute(bottom_left_attrib, glm::value_ptr(glm::vec2{ left, bottom }));
	ur.set_attribute(rect_size_attrib, glm::value_ptr(glm::vec2{ right - left,  top - bottom }));
	if (send_buffer)
		ur.send_buffer();
}
