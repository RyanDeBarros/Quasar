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

template<std::integral T, T min_, T max_>
class PercentageRange
{
	float fraction;
	T num;

public:
	static constexpr T min = min_;
	static constexpr T max = max_;

	PercentageRange(T num) { set_num(num); }
	PercentageRange(float fraction) { set_fraction(fraction); }

	float get_fraction() const { return fraction; }
	T get_num() const { return num; }
	void set_fraction(float fraction_) { fraction = fraction_; num = fraction * (max - min) + min; }
	void set_num(T num_) { num = num_; fraction = (num - min) / (max - min); }
};

typedef PercentageRange<int, 0, 255> PR255;
typedef PercentageRange<int, 0, 359> PR359;
