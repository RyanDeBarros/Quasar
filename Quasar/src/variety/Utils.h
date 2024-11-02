#pragma once

#include <climits>
#include <cmath>
#include <concepts>

constexpr int roundi(float x)
{
	return static_cast<int>(x + 0.5f);
}

constexpr float modulo(float x, float y)
{
	return y != 0.0f ? x - y * std::floorf(x / y) : 0.0f;
}

constexpr float absolute(float x)
{
	return x >= 0.0f ? x : -x;
}

template<char i>
constexpr unsigned char hex_to_byte(unsigned int hex)
{
	if constexpr (i == 0)
		return hex & 0x000000FF;
	else if constexpr (i == 1)
		return (hex & 0x0000FF00) >> 8;
	else if constexpr (i == 2)
		return (hex & 0x00FF0000) >> 16;
	else if constexpr (i == 3)
		return (hex & 0xFF000000) >> 24;
	else
		static_assert(false);
}

template<typename Held>
struct PolyHolder
{
	Held held;

	virtual ~PolyHolder() = default;
};
