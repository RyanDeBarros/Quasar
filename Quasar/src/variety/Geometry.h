#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

#include <functional>
#include <algorithm>

#include "Macros.h"

constexpr bool on_interval(float val, float min_inclusive, float max_inclusive)
{
	return val >= min_inclusive && val <= max_inclusive;
}

struct Position : glm::vec2
{
	Position(float p = 0.0f) : glm::vec2(p) {}
	Position(float x, float y) : glm::vec2(x, y) {}
	Position(int x, int y) : glm::vec2(x, y) {}
	Position(int x, float y) : glm::vec2(x, y) {}
	Position(float x, int y) : glm::vec2(x, y) {}
	Position(const glm::vec2& vec) : glm::vec2(vec) {}
	Position(const glm::ivec2& vec) : glm::vec2(vec.x, vec.y) {}

	Position operator+(Position r) const { return { x + r.x, y + r.y }; }
	Position& operator+=(Position r) { x += r.x; y += r.y; ; return *this; }
	Position operator-(Position r) const { return { x - r.x, y - r.y }; }
	Position& operator-=(Position r) { x -= r.x; y -= r.y; ; return *this; }

	Position operator+(glm::vec2 r) const { return { x + r.x, y + r.y }; }
	Position& operator+=(glm::vec2 r) { x += r.x; y += r.y; ; return *this; }
	Position operator-(glm::vec2 r) const { return { x - r.x, y - r.y }; }
	Position& operator-=(glm::vec2 r) { x -= r.x; y -= r.y; ; return *this; }

	Position operator*(Position r) const { return { x * r.x, y * r.y }; }
	Position& operator*=(Position r) { x *= r.x; y *= r.y; ; return *this; }
	Position operator/(Position r) const { return { x / r.x, y / r.y }; }
	Position& operator/=(Position r) { x /= r.x; y /= r.y; ; return *this; }

	Position operator*(glm::vec2 r) const { return { x * r.x, y * r.y }; }
	Position& operator*=(glm::vec2 r) { x *= r.x; y *= r.y; ; return *this; }
	Position operator/(glm::vec2 r) const { return { x / r.x, y / r.y }; }
	Position& operator/=(glm::vec2 r) { x /= r.x; y /= r.y; ; return *this; }

	Position operator+(float r) const { return { x + r, y + r }; }
	Position& operator+=(float r) { x += r; y += r; return *this; }
	Position operator-(float r) const { return { x - r, y - r }; }
	Position& operator-=(float r) { x -= r; y -= r; return *this; }
	Position operator*(float r) const { return { x * r, y * r }; }
	Position& operator*=(float r) { x *= r; y *= r; return *this; }
	Position operator/(float r) const { return { x / r, y / r }; }
	Position& operator/=(float r) { x /= r; y /= r; return *this; }
};

inline bool in_diagonal_rect(Position pos, Position bl, Position tr)
{
	return on_interval(pos.x, bl.x, tr.x) && on_interval(pos.y, bl.y, tr.y);
}

struct IPosition : glm::ivec2
{
	IPosition(int p = 0) : glm::ivec2(p) {}
	IPosition(int x, int y) : glm::ivec2(x, y) {}
	IPosition(glm::ivec2 v) : glm::ivec2(v) {}
	IPosition(glm::vec2 v) : glm::ivec2((int)v.x, (int)v.y) {}
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
	Scale(int x, int y) : glm::vec2(x, y) {}
	Scale(int x, float y) : glm::vec2(x, y) {}
	Scale(float x, int y) : glm::vec2(x, y) {}
	Scale(const glm::vec2& vec) : glm::vec2(vec) {}
	Scale(const glm::ivec2& vec) : glm::vec2(vec.x, vec.y) {}

	Scale reciprocal() const { return { x ? 1.0f / x : 0.0f, y ? 1.0f / y : 0.0f }; }

	Scale operator+(Scale r) const { return { x + r.x, y + r.y }; }
	Scale& operator+=(Scale r) { x += r.x; y += r.y; ; return *this; }
	Scale operator-(Scale r) const { return { x - r.x, y - r.y }; }
	Scale& operator-=(Scale r) { x -= r.x; y -= r.y; ; return *this; }

	Scale operator+(glm::vec2 r) const { return { x + r.x, y + r.y }; }
	Scale& operator+=(glm::vec2 r) { x += r.x; y += r.y; ; return *this; }
	Scale operator-(glm::vec2 r) const { return { x - r.x, y - r.y }; }
	Scale& operator-=(glm::vec2 r) { x -= r.x; y -= r.y; ; return *this; }

	Scale operator*(Scale r) const { return { x * r.x, y * r.y }; }
	Scale& operator*=(Scale r) { x *= r.x; y *= r.y; ; return *this; }
	Scale operator/(Scale r) const { return { x / r.x, y / r.y }; }
	Scale& operator/=(Scale r) { x /= r.x; y /= r.y; ; return *this; }

	Scale operator*(glm::vec2 r) const { return { x * r.x, y * r.y }; }
	Scale& operator*=(glm::vec2 r) { x *= r.x; y *= r.y; ; return *this; }
	Scale operator/(glm::vec2 r) const { return { x / r.x, y / r.y }; }
	Scale& operator/=(glm::vec2 r) { x /= r.x; y /= r.y; ; return *this; }

	Scale operator+(float r) const { return { x + r, y + r }; }
	Scale& operator+=(float r) { x += r; y += r; return *this; }
	Scale operator-(float r) const { return { x - r, y - r }; }
	Scale& operator-=(float r) { x -= r; y -= r; return *this; }
	Scale operator*(float r) const { return { x * r, y * r }; }
	Scale& operator*=(float r) { x *= r; y *= r; return *this; }
	Scale operator/(float r) const { return { x / r, y / r }; }
	Scale& operator/=(float r) { x /= r; y /= r; return *this; }
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

	bool contains_point(Position pos) const { return on_interval(pos.x, left(), right()) && on_interval(pos.y, bottom(), top()); }

	float left() const { return position.x - 0.5f * scale.x; }
	float right() const { return position.x + 0.5f * scale.x; }
	float bottom() const { return position.y - 0.5f * scale.y; }
	float top() const { return position.y + 0.5f * scale.y; }
};

extern Logger& operator<<(Logger&, const Position&);
extern Logger& operator<<(Logger&, const IPosition&);
extern Logger& operator<<(Logger&, const Scale&);
extern Logger& operator<<(Logger&, const FlatTransform&);

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

struct DiscreteLineInterpolator
{
	IPosition start;
	IPosition finish;
	IPosition delta;
	unsigned int length;

	DiscreteLineInterpolator(IPosition start, IPosition finish);

	IPosition at(int i) const;
	void at(int i, IPosition& pos) const;
};
