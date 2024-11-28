#pragma once

struct Canvas;

namespace CBImpl
{
	namespace Camera
	{
		extern void brush(Canvas& canvas, int x, int y);
	}

	namespace Paint
	{
		extern void brush(Canvas& canvas, int x, int y);
	}

	namespace Line
	{
		extern void brush(Canvas& canvas, int x, int y);
		extern void start(Canvas& canvas);
		extern void submit(Canvas& canvas);
	}

	namespace RectFill
	{
		extern void brush(Canvas& canvas, int x, int y);
		extern void start(Canvas& canvas);
		extern void submit(Canvas& canvas);
	}
}
