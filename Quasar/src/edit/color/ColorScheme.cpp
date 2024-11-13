#include "ColorScheme.h"

void ColorSubscheme::remove(size_t i)
{
	if (i < colors.size())
		colors.erase(colors.begin() + i);
}

void ColorSubscheme::insert(RGBA color, size_t pos)
{
	colors.insert(colors.begin() + pos, color);
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
	std::sort(colors.begin(), colors.end(), [this](RGBA a, RGBA b) { return predicate(a, b); });
}

size_t ColorSubscheme::first_index_of(RGBA color)
{
	auto iter = std::lower_bound(colors.begin(), colors.end(), color, [this](RGBA a, RGBA b) { return predicate(a, b); });
	if (iter == colors.end() || *iter != color)
		return -1;
	else
		return iter - colors.begin();
}

bool ColorSubscheme::predicate(RGBA a, RGBA b)
{
	switch (_sort.policy)
	{
	case SortingPolicy::HUE:
	{
		HSV ac = a.rgb.to_hsv(), bc = b.rgb.to_hsv();
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
		HSV ac = a.rgb.to_hsv(), bc = b.rgb.to_hsv();
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
		HSL ac = a.rgb.to_hsl(), bc = b.rgb.to_hsl();
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
		HSV ac = a.rgb.to_hsv(), bc = b.rgb.to_hsv();
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
		HSL ac = a.rgb.to_hsl(), bc = b.rgb.to_hsl();
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
		RGB ac = a.rgb, bc = b.rgb;
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
		RGB ac = a.rgb, bc = b.rgb;
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
		RGB ac = a.rgb, bc = b.rgb;
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
		HSV ac = a.rgb.to_hsv(), bc = b.rgb.to_hsv();
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

void ColorSubscheme::move(size_t from, size_t to)
{
	if (from < colors.size() && to < colors.size())
	{
		RGBA c = colors[from];
		colors.erase(colors.begin() + from);
		colors.insert(colors.begin() + to, c);
	}
}
