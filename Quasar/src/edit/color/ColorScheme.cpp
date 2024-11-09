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
	if (colors.size() >= MAX_COLORS)
		return -1;
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
	switch (_sort.policy)
	{
	case SortingPolicy::HUE:
	{
		HSV ac = a.hsv(), bc = b.hsv();
		if (ac.h != bc.h)
			return compare(ac.h, bc.h);
		if (ac.s != bc.s)
			return compare(bc.s, ac.s);
		if (ac.v != bc.v)
			return compare(bc.v, ac.v);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::SAT_HSV:
	{
		HSV ac = a.hsv(), bc = b.hsv();
		if (ac.s != bc.s)
			return compare(ac.s, bc.s);
		if (ac.h != bc.h)
			return compare(ac.h, bc.h);
		if (ac.v != bc.v)
			return compare(bc.v, ac.v);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::SAT_HSL:
	{
		HSL ac = a.hsl(), bc = b.hsl();
		if (ac.s != bc.s)
			return compare(ac.s, bc.s);
		if (ac.h != bc.h)
			return compare(ac.h, bc.h);
		if (ac.l != bc.l)
			return compare(bc.l, ac.l);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::VALUE:
	{
		HSV ac = a.hsv(), bc = b.hsv();
		if (ac.v != bc.v)
			return compare(bc.v, ac.v);
		if (ac.h != bc.h)
			return compare(ac.h, bc.h);
		if (ac.s != bc.s)
			return compare(ac.s, bc.s);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::LIGHT:
	{
		HSL ac = a.hsl(), bc = b.hsl();
		if (ac.l != bc.l)
			return compare(bc.l, ac.l);
		if (ac.h != bc.h)
			return compare(ac.h, bc.h);
		if (ac.s != bc.s)
			return compare(ac.s, bc.s);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::RED:
	{
		RGB ac = a.rgb(), bc = b.rgb();
		if (ac.r != bc.r)
			return compare(bc.r, ac.r);
		if (ac.g != bc.g)
			return compare(ac.g, bc.g);
		if (ac.b != bc.b)
			return compare(ac.b, bc.b);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::GREEN:
	{
		RGB ac = a.rgb(), bc = b.rgb();
		if (ac.g != bc.g)
			return compare(bc.g, ac.g);
		if (ac.b != bc.b)
			return compare(ac.b, bc.b);
		if (ac.r != bc.r)
			return compare(ac.r, bc.r);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::BLUE:
	{
		RGB ac = a.rgb(), bc = b.rgb();
		if (ac.b != bc.b)
			return compare(bc.b, ac.b);
		if (ac.r != bc.r)
			return compare(ac.r, bc.r);
		if (ac.g != bc.g)
			return compare(ac.g, bc.g);
		return compare(b.alpha, a.alpha);
	}
	case SortingPolicy::ALPHA:
	{
		HSV ac = a.hsv(), bc = b.hsv();
		if (a.alpha != b.alpha)
			return compare(b.alpha, a.alpha);
		if (ac.h != bc.h)
			return compare(ac.h, bc.h);
		if (ac.s != bc.s)
			return compare(bc.s, ac.s);
		return compare(bc.v, ac.v);
	}
	default:
		return true;
	}
}
