#pragma once

#include <vector>

#include "Color.h"

class ColorSubscheme
{
	std::vector<RGBA> colors;

public:
	ColorSubscheme() = default;
	ColorSubscheme(const std::vector<RGBA>& colors) : colors(colors) {}
	ColorSubscheme(std::vector<RGBA>&& colors) noexcept : colors(std::move(colors)) {}
	
	const std::vector<RGBA>& get_colors() const { return colors; }

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

	RGBA* at(size_t i);
	const RGBA* at(size_t i) const;
	void remove(size_t i);
	void insert(RGBA color);
	void insert(RGBA color, size_t pos);
	void sort(Sort sort);
	size_t first_index_of(RGBA color);
	bool predicate(RGBA a, RGBA b);
	void move(size_t from, size_t to);

private:
	bool(*compare)(float, float) = nullptr;
};

struct ColorScheme
{
	std::vector<std::shared_ptr<ColorSubscheme>> subschemes;
};
