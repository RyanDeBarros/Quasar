#include "PixelBufferPaths.h"

static void setup_ring(Ring& ring, Dim x0, Dim x1, Dim y0, Dim y1)
{
	ring.bottom = {};
	ring.right = {};
	ring.top = {};
	ring.left = {};
	ring.shape = 0;
	if (x1 > x0)
		ring.shape |= Ring::HORIZ;
	if (y1 > y0)
		ring.shape |= Ring::VERTI;
	if (ring.shape == Ring::FULL)
	{
		ring.bottom.x0 = x0;
		ring.bottom.x1 = x1 - 1;
		ring.bottom.y = y0;
		ring.right.x = x1;
		ring.right.y0 = y0;
		ring.right.y1 = y1 - 1;
		ring.top.x0 = x1;
		ring.top.x1 = x0 + 1;
		ring.top.y = y1;
		ring.left.x = x0;
		ring.left.y0 = y1;
		ring.left.y1 = y0 + 1;
	}
	else if (ring.shape == Ring::HORIZ)
	{
		ring.bottom.x0 = x0;
		ring.bottom.x1 = x1;
		ring.bottom.y = y0;
	}
	else if (ring.shape == Ring::VERTI)
	{
		ring.right.x = x0;
		ring.right.y0 = y0;
		ring.right.y1 = y1;
	}
	else
	{
		ring.bottom.x0 = x0;
		ring.bottom.y = y0;
	}
}

Ring::Ring(Dim x0, Dim x1, Dim y0, Dim y1)
{
	setup_ring(*this, x0, x1, y0, y1);
}

void Ring::first(PathIterator& pit) const
{
	if (shape == FULL)
		bottom.first(pit);
	else if (shape == HORIZ)
		bottom.first(pit);
	else if (shape == VERTI)
		right.first(pit);
	else
		bottom.first(pit);
}

void Ring::last(PathIterator& pit) const
{
	if (shape == FULL)
		left.last(pit);
	else if (shape == HORIZ)
		bottom.last(pit);
	else if (shape == VERTI)
		right.last(pit);
	else
		bottom.last(pit);
}

void Ring::prev(PathIterator& pit) const
{
	if (shape == FULL)
	{
		if (pit.y == bottom.y && pit.x != right.x)
		{
			if (pit == bottom.first_iter())
				left.last(pit);
			else
				bottom.prev(pit);
		}
		else if (pit.x == right.x && pit.y != top.y)
		{
			if (pit == right.first_iter())
				bottom.last(pit);
			else
				right.prev(pit);
		}
		else if (pit.y == top.y && pit.x != left.x)
		{
			if (pit == top.first_iter())
				right.last(pit);
			else
				top.prev(pit);
		}
		else
		{
			if (pit == left.first_iter())
				top.last(pit);
			else
				left.prev(pit);
		}
	}
	else if (shape == HORIZ)
		bottom.prev(pit);
	else if (shape == VERTI)
		right.prev(pit);
}

void Ring::next(PathIterator& pit) const
{
	if (shape == FULL)
	{
		if (pit.y == bottom.y && pit.x != right.x)
		{
			if (pit == bottom.last_iter())
				right.first(pit);
			else
				bottom.next(pit);
		}
		else if (pit.x == right.x && pit.y != top.y)
		{
			if (pit == right.last_iter())
				top.first(pit);
			else
				right.next(pit);
		}
		else if (pit.y == top.y && pit.x != left.x)
		{
			if (pit == top.last_iter())
				left.first(pit);
			else
				top.next(pit);
		}
		else
		{
			if (pit == left.last_iter())
				bottom.first(pit);
			else
				left.next(pit);
		}
	}
	else if (shape == HORIZ)
		bottom.next(pit);
	else if (shape == VERTI)
		right.next(pit);
}

bool Ring::to_inner()
{
	if (!valid())
		return false;
	if (shape == FULL)
	{
		setup_ring(*this, left.x + 1, right.x - 1, bottom.y + 1, top.y - 1);
		return valid();
	}
	return false;
}

bool Ring::to_outer(Dim min_x, Dim max_x, Dim min_y, Dim max_y)
{
	if (!valid())
		return false;
	if (shape == FULL)
		setup_ring(*this, std::max(left.x - 1, min_x), std::min(right.x + 1, max_x), std::max(bottom.y - 1, min_y), std::min(top.y + 1, max_y));
	else if (shape == HORIZ)
		setup_ring(*this, std::max(bottom.x0 - 1, min_x), std::min(bottom.x1 + 1, max_x), std::max(bottom.y - 1, min_y), std::min(bottom.y + 1, max_y));
	else if (shape == VERTI)
		setup_ring(*this, std::max(right.x - 1, min_x), std::min(right.x + 1, max_x), std::max(right.y0 - 1, min_y), std::min(right.y1 + 1, max_y));
	else
		setup_ring(*this, std::max(bottom.x0 - 1, min_x), std::min(bottom.x0 + 1, max_x), std::max(bottom.y - 1, min_y), std::min(bottom.y + 1, max_y));
	return valid();
}

Dim Ring::width() const
{
	if (!valid()) return 0;
	if (shape == FULL)
		return right.x - left.x + 1;
	else if (shape == HORIZ)
		return bottom.x1 - bottom.x0 + 1;
	else
		return 1;
}

Dim Ring::height() const
{
	if (!valid()) return 0;
	if (shape == FULL)
		return top.y - bottom.y + 1;
	else if (shape == VERTI)
		return right.y1 - right.y0 + 1;
	else
		return 1;
}

Dim Ring::length() const
{
	if (!valid()) return 0;
	if (shape == FULL)
		return 2 * (right.x - left.x + top.y - bottom.y);
	else if (shape == HORIZ)
		return bottom.x1 - bottom.x0 + 1;
	else if (shape == VERTI)
		return right.y1 - right.y0 + 1;
	else
		return 1;
}

bool Ring::valid() const
{
	if (shape == FULL)
		return top.y >= bottom.y && left.x <= right.x;
	else if (shape == HORIZ)
		return bottom.x1 >= bottom.x0;
	else if (shape == VERTI)
		return right.y1 >= right.y0;
	else
		return true;
}
