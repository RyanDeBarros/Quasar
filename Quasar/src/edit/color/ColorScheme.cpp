#include "ColorScheme.h"

ColorFrame* ColorSubscheme::at(size_t i)
{
	if (i < colors.size())
		return &colors[i];
	else
		return nullptr;
}

const ColorFrame* ColorSubscheme::at(size_t i) const
{
	if (i < colors.size())
		return &colors[i];
	else
		return nullptr;
}

void ColorSubscheme::remove(size_t i)
{
	if (i < colors.size())
		colors.erase(colors.begin() + i);
}

size_t ColorSubscheme::insert(ColorFrame color, Sort sort)
{
	this->sort(sort);
	// TODO test that this works
	auto iter = std::lower_bound(colors.begin(), colors.end(), color, [this](ColorFrame a, ColorFrame b) { return predicate(a, b); });
	auto index = iter - colors.begin();
	colors.insert(iter, color);
	return index;
}

static constexpr bool less(float a, float b) { return a < b; }
static constexpr bool greater(float a, float b) { return a > b; }

void ColorSubscheme::sort(Sort sort)
{
	if (sort == _sort)
		return;
	_sort = sort;
	if (sort.policy == SortingPolicy::NONE)
		return;
	compare = sort.topfirst ? &greater : &less;
	std::sort(colors.begin(), colors.end(), [this](ColorFrame a, ColorFrame b) { return predicate(a, b); });
}

size_t ColorSubscheme::first_index_of(ColorFrame color)
{
	auto iter = std::lower_bound(colors.begin(), colors.end(), color, [this](ColorFrame a, ColorFrame b) { return predicate(a, b); });
	if (iter == colors.end() || *iter != color)
		return -1;
	else
		return iter - colors.begin();
}

bool ColorSubscheme::predicate(ColorFrame a, ColorFrame b)
{
	// TODO
	switch (_sort.policy)
	{
	case SortingPolicy::HUE:
	{
		HSV ac = a.hsv(), bc = b.hsv();
		if (compare(ac.h, bc.h))
			return true;
		else if (compare(bc.h, ac.h))
			return false;
		else if (compare(bc.s, ac.s))
			return true;
		else if (compare(ac.s, bc.s))
			return false;
		else
			return compare(bc.v, ac.v);
	}
	default:
		return true;
	}
}
