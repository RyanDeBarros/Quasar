#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

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
