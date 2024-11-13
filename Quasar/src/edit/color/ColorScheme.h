#pragma once

#include <vector>

#include "Color.h"

struct ColorSubscheme
{
	std::vector<RGBA> colors;

	ColorSubscheme(const std::string& name, const std::vector<RGBA>& colors = {}) : name(name), colors(colors) {}
	ColorSubscheme(const std::string& name, std::vector<RGBA>&& colors) : name(name), colors(std::move(colors)) {}
	ColorSubscheme(std::string&& name, const std::vector<RGBA>& colors = {}) : name(std::move(name)), colors(colors) {}
	ColorSubscheme(std::string&& name, std::vector<RGBA>&& colors) : name(std::move(name)), colors(std::move(colors)) {}
	
	static const size_t MAX_NAME_LENGTH = 24;

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
	void remove(size_t i);
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

	ColorScheme() = default;
	ColorScheme(const std::vector<std::shared_ptr<ColorSubscheme>>& subschemes) : subschemes(subschemes) {}
	ColorScheme(std::vector<std::shared_ptr<ColorSubscheme>>&& subschemes) : subschemes(std::move(subschemes)) {}
};
