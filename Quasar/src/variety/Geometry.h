#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

#include <functional>

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
	Scale(float x = 1.0f, float y = 1.0f) : glm::vec2(x, y) {}
	Scale(const glm::vec2& vec) : glm::vec2(vec) {}
};

struct Transform
{
	Position position;
	Rotation rotation;
	Scale scale;

	glm::mat3 camera() const { return inverse().matrix(); }

	Transform inverse() const
	{
		return { -position, -rotation, 1.0f / scale };
	}

	glm::mat3 matrix() const
	{
		float cos = glm::cos(rotation), sin = glm::sin(rotation);
		return glm::mat3({ scale.x * cos, scale.x * sin, 0.0f }, { -scale.y * sin, scale.y * cos, 0.0f }, { position.x, position.y, 1.0f });
	}

	glm::vec2 packed_p() const { return position; }
	glm::vec4 packed_rs() const { return { scale.x * glm::cos(rotation), scale.x * glm::sin(rotation), -scale.y * glm::sin(rotation), scale.y * glm::cos(rotation) }; }
};

struct FlatTransform
{
	Position position;
	Scale scale;

	glm::mat3 camera() const { return inverse().matrix(); }
	
	FlatTransform inverse() const
	{
		return { -position, 1.0f / scale };
	}

	glm::mat3 matrix() const
	{
		return glm::mat3({ scale.x, 0.0f, 0.0f }, { 0.0f, scale.y, 0.0f }, { position.x, position.y, 1.0f });
	}

	glm::vec4 packed() const { return { position.x, position.y, scale.x, scale.y }; }
};

struct ClippingRect
{
	GLint x = 0, y = 0;
	GLsizei screen_w = 0, screen_h = 0;

	ClippingRect() = default;
	ClippingRect(GLint x, GLint y, GLsizei screen_w, GLsizei screen_h) : x(x), y(y), screen_w(screen_w), screen_h(screen_h) {}

	bool contains_point(const glm::vec2& point) const
	{
		return x <= point.x && point.x < x + screen_w && y <= point.y && point.y < y + screen_h;
	}

	glm::vec2 center_point() const
	{
		return { 0.5f * (x + screen_w), 0.5f * (y + screen_h) };
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
