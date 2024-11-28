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
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);

		extern void brush_submit(Canvas& canvas);
	}

	namespace Line
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);
		
		extern void start(Canvas& canvas);
		
		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);
	}

	namespace RectFill
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);
		
		extern void start_pencil(Canvas& canvas);
		extern void start_pen(Canvas& canvas);
		extern void start_eraser(Canvas& canvas);
		extern void start_select(Canvas& canvas);
		
		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);
	}
}
