#pragma once

struct Canvas;
struct BrushInfo;
struct PixelRGBA;

namespace CBImpl
{
	extern void brush_move_to(Canvas& canvas, int x, int y);

	extern void fill_selection_pencil(Canvas& canvas, PixelRGBA color);
	extern void fill_selection_pen(Canvas& canvas, PixelRGBA color);
	extern void fill_selection_eraser(Canvas& canvas);

	extern void move_selection_with_pixels_pencil(Canvas& canvas, int dx, int dy);
	extern void move_selection_with_pixels_pen(Canvas& canvas, int dx, int dy);
	extern void move_selection_without_pixels(Canvas& canvas, int dx, int dy);

	extern void apply_selection_pencil(Canvas& canvas);
	extern void apply_selection_pen(Canvas& canvas);

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

		extern void start_select(Canvas& canvas);

		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);

		extern void reset_pencil(BrushInfo& binfo);
		extern void reset_pen(BrushInfo& binfo);
		extern void reset_eraser(BrushInfo& binfo);
		extern void reset_select(BrushInfo& binfo);
	}

	namespace Line
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);
		
		extern void start(Canvas& canvas);
		extern void start_select(Canvas& canvas);
		
		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);

		extern void reset_pencil(BrushInfo& binfo);
		extern void reset_pen(BrushInfo& binfo);
		extern void reset_eraser(BrushInfo& binfo);
		extern void reset_select(BrushInfo& binfo);
	}

	namespace RectOutline
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);

		extern void start(Canvas& canvas);
		extern void start_select(Canvas& canvas);

		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);

		extern void reset_pencil(BrushInfo& binfo);
		extern void reset_pen(BrushInfo& binfo);
		extern void reset_eraser(BrushInfo& binfo);
		extern void reset_select(BrushInfo& binfo);
	}

	namespace RectFill
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);

		extern void start(Canvas& canvas);
		extern void start_select(Canvas& canvas);
		
		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);

		extern void reset_pencil(BrushInfo& binfo);
		extern void reset_pen(BrushInfo& binfo);
		extern void reset_eraser(BrushInfo& binfo);
		extern void reset_select(BrushInfo& binfo);
	}

	namespace EllipseOutline
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);

		extern void start(Canvas& canvas);
		extern void start_select(Canvas& canvas);

		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);

		extern void reset_pencil(BrushInfo& binfo);
		extern void reset_pen(BrushInfo& binfo);
		extern void reset_eraser(BrushInfo& binfo);
		extern void reset_select(BrushInfo& binfo);
	}

	namespace EllipseFill
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);

		extern void start(Canvas& canvas);
		extern void start_select(Canvas& canvas);

		extern void submit_pencil(Canvas& canvas);
		extern void submit_pen(Canvas& canvas);
		extern void submit_eraser(Canvas& canvas);
		extern void submit_select(Canvas& canvas);

		extern void reset_pencil(BrushInfo& binfo);
		extern void reset_pen(BrushInfo& binfo);
		extern void reset_eraser(BrushInfo& binfo);
		extern void reset_select(BrushInfo& binfo);
	}

	namespace BucketFill
	{
		extern void brush_pencil(Canvas& canvas, int x, int y);
		extern void brush_pen(Canvas& canvas, int x, int y);
		extern void brush_eraser(Canvas& canvas, int x, int y);
		extern void brush_select(Canvas& canvas, int x, int y);
		extern void reset_select(BrushInfo& binfo);
	}
}
