#pragma once

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

	glm::mat3 inverse() const
	{
		return glm::mat3(glm::cos(rotation) / scale.x, -glm::sin(rotation) / scale.x, 0, glm::sin(rotation) / scale.y, glm::cos(rotation) / scale.y, 0, -position.x, -position.y, 1);
	}

	glm::vec2 packed_p() const { return position; }
	glm::vec4 packed_rs() const { return { scale.x * glm::cos(rotation), scale.x * glm::sin(rotation), -scale.y * glm::sin(rotation), scale.y * glm::cos(rotation)}; }
};
