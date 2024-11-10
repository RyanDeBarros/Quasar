#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

#include <functional>
#include <algorithm>

#include "Macros.h"

struct Position : glm::vec2
{
	Position(float x = 0.0f, float y = 0.0f) : glm::vec2(x, y) {}
	Position(const glm::vec2& vec) : glm::vec2(vec) {}
};

struct Rotation
{
	float _v;
	Rotation(float v = 0.0f) : _v(v) {}
	Rotation(const Rotation&) = default;
	Rotation(Rotation&&) noexcept = default;
	Rotation& operator=(const Rotation&) = default;
	Rotation& operator=(Rotation&&) = default;
	operator float() const { return _v; }
};

struct Scale : glm::vec2
{
	Scale(float s = 1.0f) : glm::vec2(s) {}
	Scale(float x, float y) : glm::vec2(x, y) {}
	Scale(const glm::vec2& vec) : glm::vec2(vec) {}

	Scale reciprocal() const { return { x ? 1.0f / x : 0.0f, y ? 1.0f / y : 0.0f }; }
};

struct Transform
{
	Position position;
	Rotation rotation;
	Scale scale;

	glm::mat3 camera() const { return inverse().matrix(); }

	Transform inverse() const
	{
		return { -position / scale, -rotation, 1.0f / scale };
	}

	glm::mat3 matrix() const
	{
		float cos = glm::cos(rotation), sin = glm::sin(rotation);
		return glm::mat3({ scale.x * cos, scale.x * sin, 0.0f },
			{ -scale.y * sin, scale.y * cos, 0.0f }, { position.x, position.y, 1.0f });
	}

	glm::vec2 packed_p() const { return position; }
	glm::vec4 packed_rs() const { return { scale.x * glm::cos(rotation), scale.x * glm::sin(rotation),
		-scale.y * glm::sin(rotation), scale.y * glm::cos(rotation) }; }
};

struct FlatTransform
{
	Position position;
	Scale scale;

	bool operator==(const FlatTransform&) const = default;

	glm::mat3 camera() const { return inverse().matrix(); }
	
	FlatTransform inverse() const
	{
		return { -position / scale, 1.0f / scale };
	}

	glm::mat3 matrix() const
	{
		return glm::mat3({ scale.x, 0.0f, 0.0f }, { 0.0f, scale.y, 0.0f }, { position.x, position.y, 1.0f });
	}

	glm::vec4 packed() const { return { position.x, position.y, scale.x, scale.y }; }

	FlatTransform relative_to(const FlatTransform& parent) const
	{
		return { parent.position + parent.scale * position, parent.scale * scale };
	}

	FlatTransform get_relative(const FlatTransform& absolute) const
	{
		return { (absolute.position - position) * scale.reciprocal(), absolute.scale * scale.reciprocal() };
	}

	Position get_relative_pos(Position absolute) const { return (absolute - position) * scale.reciprocal(); }
};

constexpr bool on_interval(float val, float min_inclusive, float max_inclusive)
{
	return val >= min_inclusive && val <= max_inclusive;
}

inline bool in_diagonal_rect(Position pos, Position bl, Position tr)
{
	return on_interval(pos.x, bl.x, tr.x) && on_interval(pos.y, bl.y, tr.y);
}

struct ClippingRect
{
	GLint x = 0, y = 0;
	GLsizei screen_w = 0, screen_h = 0;

	ClippingRect() = default;
	ClippingRect(GLint x, GLint y, GLsizei screen_w, GLsizei screen_h) : x(x), y(y), screen_w(screen_w), screen_h(screen_h) {}

	bool contains_point(const glm::vec2& point) const
	{
		return on_interval(point.x - x, 0.0f, float(screen_w)) && on_interval(point.y - y, 0.0f, float(screen_h));
	}

	glm::vec2 center_point() const
	{
		return { x + 0.5f * screen_w, y + 0.5f * screen_h };
	}

	void scissor() const
	{
		QUASAR_GL(glScissor(x, y, screen_w, screen_h));
	}
};

struct Bounds
{
	GLfloat x1, x2;
	GLfloat y1, y2;

	void pass_uvs(GLfloat* buffer_at_uvs, size_t stride) const
	{
		buffer_at_uvs[0] = x1;
		buffer_at_uvs[1] = y1;
		buffer_at_uvs += stride;
		buffer_at_uvs[0] = x2;
		buffer_at_uvs[1] = y1;
		buffer_at_uvs += stride;
		buffer_at_uvs[0] = x2;
		buffer_at_uvs[1] = y2;
		buffer_at_uvs += stride;
		buffer_at_uvs[0] = x1;
		buffer_at_uvs[1] = y2;
	}
};

struct IntBounds
{
	int x1, x2;
	int y1, y2;

	ClippingRect clip() const
	{
		return ClippingRect(x1, y1, std::max(x2 - x1, 0), std::max(y2 - y1, 0));
	}
};

enum class Cardinal
{
	RIGHT,
	UP,
	LEFT,
	DOWN
};

constexpr float mean2d1d(float a, float b)
{
	return (a + b + std::max(a, b)) / 3.0f;
}
