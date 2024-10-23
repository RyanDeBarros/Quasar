#pragma once

#include "PixelBuffer.h"

struct HorizontalLine : public Path
{
	Dim x0 = 0, x1 = 0, y = 0;
	void first(PathIterator& pit) const override { pit.x = x0; pit.y = y; }
	void last(PathIterator& pit) const override { pit.x = x1; pit.y = y; }
	void prev(PathIterator& pit) const override { --pit.x; }
	void next(PathIterator& pit) const override { ++pit.x; }
};

struct VerticalLine : public Path
{
	Dim x = 0, y0 = 0, y1 = 0;
	void first(PathIterator& pit) const override { pit.x = x; pit.y = y0; }
	void last(PathIterator& pit) const override { pit.x = x; pit.y = y1; }
	void prev(PathIterator& pit) const override { --pit.y; }
	void next(PathIterator& pit) const override { ++pit.y; }
};

struct UprightRect : public Path
{
	Dim x0 = 0, x1 = 0, y0 = 0, y1 = 0;

	UprightRect() = default;
	UprightRect(Dim width, Dim height) : x0(0), x1(width - 1), y0(0), y1(height - 1) {}

	void first(PathIterator& pit) const override { pit.x = x0; pit.y = y0; }
	void last(PathIterator& pit) const override { pit.x = x1; pit.y = y1; }
	void prev(PathIterator& pit) const override { if (pit.x == x0) { pit.x = x1; --pit.y; } else { --pit.x; } }
	void next(PathIterator& pit) const override { if (pit.x == x1) { pit.x = x0; ++pit.y; } else { ++pit.x; } }
};

