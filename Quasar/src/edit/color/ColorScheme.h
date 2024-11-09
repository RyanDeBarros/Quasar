#pragma once

#include <vector>

#include "Color.h"

class ColorSubscheme
{
	std::vector<ColorFrame> colors;

public:
	ColorSubscheme() = default;

	static const size_t MAX_COLORS = 256;

	std::string name;

	enum class SortingPolicy : char
	{
		NONE,
		HUE,
		SAT_HSV,
		SAT_HSL,
		VALUE,
		LIGHT,
		RED,
		GREEN,
		BLUE,
		ALPHA
	};
	struct Sort
	{
		SortingPolicy policy;
		bool topfirst;

		bool operator==(const Sort&) const = default;
	};

private:
	Sort _sort = { SortingPolicy::NONE, true };
public:

	ColorFrame* at(size_t i);
	const ColorFrame* at(size_t i) const;
	void remove(size_t i);
	size_t insert(ColorFrame color, Sort sort = { SortingPolicy::NONE, true });
	void sort(Sort sort);
	size_t first_index_of(ColorFrame color);
	bool predicate(ColorFrame a, ColorFrame b);

private:
	bool(*compare)(float, float);
};

struct ColorScheme
{
	std::vector<ColorSubscheme> subschemes;
	
	static const size_t MAX_SUBSCHEMES = 16;
};
