#pragma once

#include "PixelBuffer.h"

struct HorizontalLine : public Path
{
	Dim x0 = 0, x1 = 0, y = 0;
	void first(PathIterator& pit) const override { pit.x = x0; pit.y = y; }
	void last(PathIterator& pit) const override { pit.x = x1; pit.y = y; }
	void prev(PathIterator& pit) const override
	{
		if (pit.x == x0)
			return;
		if (x0 < x1)
			--pit.x;
		else
			++pit.x;
	}
	void next(PathIterator& pit) const override
	{
		if (pit.x == x1)
			return;
		if (x0 < x1)
			++pit.x;
		else
			--pit.x;
	}
};

struct VerticalLine : public Path
{
	Dim x = 0, y0 = 0, y1 = 0;
	void first(PathIterator& pit) const override { pit.x = x; pit.y = y0; }
	void last(PathIterator& pit) const override { pit.x = x; pit.y = y1; }
	void prev(PathIterator& pit) const override
	{
		if (pit.y == y0)
			return;
		if (y0 < y1)
			--pit.y;
		else
			++pit.y;
	}
	void next(PathIterator& pit) const override
	{
		if (pit.y == y1)
			return;
		if (y0 < y1)
			++pit.y;
		else
			--pit.y;
	}
};

struct UprightRect : public Path
{
	Dim x0 = 0, x1 = 0, y0 = 0, y1 = 0;

	UprightRect() = default;
	UprightRect(Dim width, Dim height) : x0(0), x1(width - 1), y0(0), y1(height - 1) {}

	void first(PathIterator& pit) const override { pit.x = x0; pit.y = y0; }
	void last(PathIterator& pit) const override { pit.x = x1; pit.y = y1; }
	void prev(PathIterator& pit) const override
	{
		if (pit.x == x0)
		{
			if (pit.y == y0)
				return;
			pit.x = x1;
			--pit.y;
		}
		else
			--pit.x;
	}
	void next(PathIterator& pit) const override
	{
		if (pit.x == x1)
		{
			if (pit.y == y1)
				return;
			pit.x = x0;
			++pit.y;
		}
		else
			++pit.x;
	}
};

struct Ring : public Path
{
	HorizontalLine bottom;
	VerticalLine right;
	HorizontalLine top;
	VerticalLine left;
	static constexpr char HORIZ = 0b01;
	static constexpr char VERTI = 0b10;
	static constexpr char FULL = HORIZ | VERTI;
	char shape = 0;

	Ring(Dim x0, Dim x1, Dim y0, Dim y1);

	void first(PathIterator& pit) const override;
	void last(PathIterator& pit) const override;
	void prev(PathIterator& pit) const override;
	void next(PathIterator& pit) const override;

	bool to_inner();
	bool to_outer(Dim min_x, Dim max_x, Dim min_y, Dim max_y);
	Dim width() const;
	Dim height() const;
	Dim length() const;
	bool valid() const;
};
