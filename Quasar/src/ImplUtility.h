#pragma once

#include <memory>

#include <glm/glm.hpp>

struct Cursor;
struct Buffer;
struct UnitRenderable;
struct IndexedRenderable;
struct FlatTransform;
struct WidgetPlacement;
struct Bounds;

namespace Utils
{
	std::shared_ptr<Cursor> cursor_from_buffer(const Buffer& buffer, float xhot = 0.5f, float yhot = 0.5f);
	std::shared_ptr<Cursor> cursor_from_buffer(const Buffer& buffer, int xhot, int yhot);

	void set_vertex_pos_attributes(const UnitRenderable& ur, FlatTransform tr, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_vertex_pos_attributes(const IndexedRenderable& ir, FlatTransform tr, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_vertex_pos_attributes(const UnitRenderable& ur, const WidgetPlacement& wp, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_vertex_pos_attributes(const IndexedRenderable& ir, const WidgetPlacement& wp, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_vertex_pos_attributes(const UnitRenderable& ur, const glm::mat3& mat3, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_vertex_pos_attributes(const IndexedRenderable& ir, const glm::mat3& mat3, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_uv_attributes(const UnitRenderable& ur, float x1, float x2, float y1, float y2, unsigned short vertex_offset, size_t attrib, bool send_buffer = true);
	void set_uv_attributes(const UnitRenderable& ur, Bounds bounds, unsigned short vertex_offset, size_t attrib, bool send_buffer = true);
	void set_uv_attributes(const IndexedRenderable& ir, float x1, float x2, float y1, float y2, unsigned short vertex_offset, size_t attrib, bool send_buffer = true);
	void set_uv_attributes(const IndexedRenderable& ir, Bounds bounds, unsigned short vertex_offset, size_t attrib, bool send_buffer = true);
	void set_four_attributes(const UnitRenderable& ur, float val, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_four_attributes(const UnitRenderable& ur, const float* val, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_four_attributes(const IndexedRenderable& ir, float val, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_four_attributes(const IndexedRenderable& ir, const float* val, unsigned short vertex_offset = 0, size_t attrib = 0, bool send_buffer = true);
	void set_rect_dimension_attributes(const UnitRenderable& ur, FlatTransform tr, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer = true);
	void set_rect_dimension_attributes(const UnitRenderable& ur, const WidgetPlacement& wp, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer = true);
	void set_rect_dimension_attributes(const UnitRenderable& ur, const glm::mat3& mat3, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer = true);
	void set_rect_dimension_attributes(const UnitRenderable& ur, float left, float right, float bottom, float top, size_t bottom_left_attrib, size_t rect_size_attrib, bool send_buffer = true);
}
