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

constexpr bool on_interval(int val, int min_inclusive, int max_inclusive)
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
	IPosition(float x, float y) : glm::ivec2((int)x, (int)y) {}
	IPosition(int x, float y) : glm::ivec2(x, (int)y) {}
	IPosition(float x, int y) : glm::ivec2((int)x, y) {}
	IPosition(glm::ivec2 v) : glm::ivec2(v) {}
	IPosition(glm::vec2 v) : glm::ivec2((int)v.x, (int)v.y) {}
};

struct IScale : glm::ivec2
{
	IScale(int p = 1) : glm::ivec2(p) {}
	IScale(int x, int y) : glm::ivec2(x, y) {}
	IScale(float x, float y) : glm::ivec2((int)x, (int)y) {}
	IScale(int x, float y) : glm::ivec2(x, (int)y) {}
	IScale(float x, int y) : glm::ivec2((int)x, y) {}
	IScale(glm::ivec2 v) : glm::ivec2(v) {}
	IScale(glm::vec2 v) : glm::ivec2((int)v.x, (int)v.y) {}
};

template<>
struct std::hash<IPosition>
{
	size_t operator()(const IPosition& pos) const { return std::hash<int>{}(pos.x) ^ std::hash<int>{}(pos.y); }
};

inline bool in_diagonal_rect(IPosition pos, IPosition bl, IPosition tr)
{
	return on_interval(pos.x, bl.x, tr.x) && on_interval(pos.y, bl.y, tr.y);
}

inline bool intersection(int l1, int r1, int l2, int r2, int& l3, int& r3)
{
	if (r1 < l2 || r2 < l1)
		return false;
	l3 = std::max(l1, l2);
	r3 = std::min(r1, r2);
	return true;
}

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

	bool operator==(const IntBounds&) const = default;

	IPosition delta() const { return { x2 - x1, y2 - y1 }; }
	int width() const { return std::abs(x2 - x1) + 1; }
	int height() const { return std::abs(y2 - y1) + 1; }
	int width_no_abs() const { return x2 - x1 + 1; }
	int height_no_abs() const { return y2 - y1 + 1; }
	bool valid() const { return x2 >= x1 && y2 >= y1; }

	static const IntBounds NADIR;
};

inline const IntBounds IntBounds::NADIR = { INT_MAX, INT_MIN, INT_MAX, INT_MIN };

inline void update_bbox(IntBounds& bbox, int x, int y)
{
	if (x < bbox.x1)
		bbox.x1 = x;
	if (x > bbox.x2)
		bbox.x2 = x;
	if (y < bbox.y1)
		bbox.y1 = y;
	if (y > bbox.y2)
		bbox.y2 = y;
}

inline void update_bbox(IntBounds& bbox, IntBounds with)
{
	update_bbox(bbox, with.x1, with.y1);
	update_bbox(bbox, with.x2, with.y2);
}

inline IntBounds abs_bounds(IPosition p1, IPosition p2)
{
	return { std::min(p1.x, p2.x), std::max(p1.x, p2.x), std::min(p1.y, p2.y), std::max(p1.y, p2.y) };
}

inline bool intersection(IntBounds b1, IntBounds b2, IntBounds& b3)
{
	return intersection(b1.x1, b1.x2, b2.x1, b2.x2, b3.x1, b3.x2) && intersection(b1.y1, b1.y2, b2.y1, b2.y2, b3.y1, b3.y2);
}

struct IntRect
{
	int x = 0, y = 0, w = 0, h = 0;

	bool intersect(IntRect other, IntRect& result) const;
	IntRect scaled(int sx, int sy) const { return { sx * x, sy * y, sx * w, sy * h }; }
};

inline bool IntRect::intersect(IntRect other, IntRect& result) const
{
	if (x >= other.x)
	{
		int diff = other.x + other.w - x;
		if (diff < 0)
			return false;
		result.x = x;
		result.w = std::min(diff, w);
	}
	else
	{
		int diff = x + w - other.x;
		if (diff < 0)
			return false;
		result.x = other.x;
		result.w = std::min(diff, other.w);
	}
	if (y >= other.y)
	{
		int diff = other.y + other.h - y;
		if (diff < 0)
			return false;
		result.y = y;
		result.h = std::min(diff, h);
	}
	else
	{
		int diff = y + h - other.y;
		if (diff < 0)
			return false;
		result.y = other.y;
		result.h = std::min(diff, other.h);
	}
	return true;
}

inline IntRect abs_rect(IPosition p1, IPosition p2, int sx = 1, int sy = 1)
{
	p1.x *= sx;
	p1.y *= sy;
	p2.x *= sx;
	p2.y *= sy;
	IntRect rect;
	rect.x = std::min(p1.x, p2.x);
	rect.w = (std::abs(p2.x - p1.x) + 1) + sx;
	rect.y = std::min(p1.y, p2.y);
	rect.h = (std::abs(p2.y - p1.y) + 1) + sy;
	return rect;
}

inline IntRect bounds_to_rect(IntBounds bb, int sx = 1, int sy = 1)
{
	return { bb.x1 * sx, bb.y1 * sy, bb.width() * sx, bb.height() * sy };
}

enum Cardinal
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

extern Logger& operator<<(Logger&, const Position&);
extern Logger& operator<<(Logger&, const IPosition&);
extern Logger& operator<<(Logger&, const Scale&);
extern Logger& operator<<(Logger&, const FlatTransform&);
extern Logger& operator<<(Logger&, const ClippingRect&);
extern Logger& operator<<(Logger&, const Bounds&);
extern Logger& operator<<(Logger&, const IntBounds&);
extern Logger& operator<<(Logger&, const IntRect&);
