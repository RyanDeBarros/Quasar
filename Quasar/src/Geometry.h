#pragma once

#include <glm/glm.hpp>

struct Position : glm::vec2
{
	Position() : glm::vec2(0.0f, 0.0f) {}
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
	Scale() : glm::vec2(1.0f, 1.0f) {}
};

struct Transform
{
	Position position;
	Rotation rotation;
	Scale scale;

	glm::mat3 inverse() const
	{
		return glm::mat3(cosf(rotation) / scale.x, -sinf(rotation) / scale.x, 0, sinf(rotation) / scale.y, cosf(rotation) / scale.y, 0, -position.x, -position.y, 1);
	}
};
