#pragma once

struct Canvas;
struct BrushInfo;
struct PixelRGBA;
enum class BrushTip;

namespace CBImpl
{
	extern void brush_move_to(Canvas& canvas, int x, int y);

	extern void fill_selection_pencil(Canvas& canvas, PixelRGBA color);
	extern void fill_selection_pen(Canvas& canvas, PixelRGBA color);
	extern void fill_selection_eraser(Canvas& canvas);

	extern void batch_move_selection_with_pixels(Canvas& canvas, int dx, int dy);
	extern void batch_move_selection_start_with_pixels(Canvas& canvas);
	extern void batch_move_selection_submit_with_pixels(Canvas& canvas);
	extern void batch_move_selection_cancel_with_pixels(Canvas& canvas);

	extern void batch_move_selection_transition(Canvas& canvas, bool to_pencil);

	extern void batch_move_selection_without_pixels(Canvas& canvas, int dx, int dy);
	extern void batch_move_selection_start_without_pixels(Canvas& canvas);
	extern void batch_move_selection_submit_without_pixels(Canvas& canvas);
	extern void batch_move_selection_cancel_without_pixels(Canvas& canvas);

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
