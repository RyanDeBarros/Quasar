#include "CanvasBrushImpl.h"

#include "Easel.h"
#include "user/Machine.h"

static void standard_submit_pencil(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_2c.empty())
		Machine.history.execute(std::make_shared<OneColorPencilAction>(canvas.image, binfo.starting_pos, binfo.last_brush_pos, std::move(binfo.storage_2c)));
}

static void standard_submit_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
			binfo.starting_pos, binfo.last_brush_pos, std::move(binfo.storage_1c)));
}

static void standard_submit_eraser(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 },
			binfo.starting_pos, binfo.last_brush_pos, std::move(binfo.storage_1c)));
}

void CBImpl::brush_move_to(Canvas& canvas, int x, int y)
{
	DiscreteLineInterpolator interp;
	interp.start = canvas.binfo.image_pos;
	interp.finish = { x, y };
	interp.sync_with_endpoints();
	IPosition intermediate;
	for (unsigned int i = 1; i < interp.length; ++i)
	{
		interp.at(i, intermediate.x, intermediate.y);
		canvas.brush(intermediate.x, intermediate.y);
	}
}

void CBImpl::Camera::brush(Canvas& canvas, int x, int y)
{
	// do nothing
}

// <<<==================================<<< PAINT >>>==================================>>>
// TODO Paint should use standard submission techniques specific to pencil, pen, and eraser, in the exact same way as Line, RectFill, RectOutline, etc.

static Buffer paint_brush_prefix(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	Buffer image_shifted_buf = canvas.image->buf;
	image_shifted_buf.pixels += image_shifted_buf.byte_offset(x, y);
	
	if (x < binfo.brushing_bbox.x1)
		binfo.brushing_bbox.x1 = x;
	if (x > binfo.brushing_bbox.x2)
		binfo.brushing_bbox.x2 = x;
	if (y < binfo.brushing_bbox.y1)
		binfo.brushing_bbox.y1 = y;
	if (y > binfo.brushing_bbox.y2)
		binfo.brushing_bbox.y2 = y;

	return image_shifted_buf;
}

static void paint_brush_suffix(Canvas& canvas, int x, int y, PixelRGBA initial_c, PixelRGBA final_c)
{
	canvas.image->update_subtexture(x, y, 1, 1);
	auto iter = canvas.binfo.storage_2c.find({ x, y });
	if (iter == canvas.binfo.storage_2c.end())
		canvas.binfo.storage_2c[{ x, y }] = { initial_c, final_c };
	else
		iter->second.second = final_c;
}

void CBImpl::Paint::brush_pencil(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	Buffer image_shifted_buf = paint_brush_prefix(canvas, x, y);
	PixelRGBA initial_c{ 0, 0, 0, 0 };
	PixelRGBA final_c{ 0, 0, 0, 0 };
	PixelRGBA color = canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY ? canvas.pric_pxs : canvas.altc_pxs;
	float applied_alpha = canvas.applied_color().alpha;
	for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
	{
		initial_c.at(i) = image_shifted_buf.pixels[i];
		if (i < 3)
			image_shifted_buf.pixels[i] = std::clamp(roundi(color[i] * applied_alpha + image_shifted_buf.pixels[i] * (1 - applied_alpha)), 0, 255);
		else
			image_shifted_buf.pixels[i] = std::clamp(roundi(applied_alpha * 255 + image_shifted_buf.pixels[i] * (1 - applied_alpha)), 0, 255);
		final_c.at(i) = image_shifted_buf.pixels[i];
	}
	paint_brush_suffix(canvas, x, y, initial_c, final_c);
}

void CBImpl::Paint::brush_pen(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	Buffer image_shifted_buf = paint_brush_prefix(canvas, x, y);
	PixelRGBA initial_c{ 0, 0, 0, 0 };
	PixelRGBA color = canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY ? canvas.pric_pen_pxs : canvas.altc_pen_pxs;
	for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
	{
		initial_c.at(i) = image_shifted_buf.pixels[i];
		image_shifted_buf.pixels[i] = color[i];
	}
	paint_brush_suffix(canvas, x, y, initial_c, color);
}

void CBImpl::Paint::brush_eraser(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	Buffer image_shifted_buf = paint_brush_prefix(canvas, x, y);
	PixelRGBA initial_c{ 0, 0, 0, 0 };
	PixelRGBA final_c{ 0, 0, 0, 0 };
	// LATER define more utilities for these sorts of things, like swap/get/set.
	for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
	{
		initial_c.at(i) = image_shifted_buf.pixels[i];
		image_shifted_buf.pixels[i] = 0;
	}
	paint_brush_suffix(canvas, x, y, initial_c, final_c);
}

void CBImpl::Paint::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}

void CBImpl::Paint::brush_submit(Canvas& canvas)
{
	if (canvas.image && !canvas.binfo.storage_2c.empty())
		Machine.history.push(std::make_shared<PaintToolAction>(canvas.image, canvas.binfo.brushing_bbox, std::move(canvas.binfo.storage_2c)));
}

// <<<==================================<<< LINE >>>==================================>>>

static void line_brush_remove_pencil_and_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	auto& interp = binfo.interps.line;
	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

static DiscreteLineInterpolator& line_brush_setup_interp(BrushInfo& binfo, int x, int y)
{
	auto& interp = binfo.interps.line;
	interp.finish = { x, y };
	interp.sync_with_endpoints();
	return interp;
}

void CBImpl::Line::brush_pencil(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_2c.clear();
	line_brush_remove_pencil_and_pen(canvas);
	auto& interp = line_brush_setup_interp(binfo, x, y);
	IPosition pos{};
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		new_color = canvas.applied_color().get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, new_color);
		new_color.blend_over(old_color);
		binfo.storage_2c[pos] = {new_color, old_color};
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

void CBImpl::Line::brush_pen(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	line_brush_remove_pencil_and_pen(canvas);
	auto& interp = line_brush_setup_interp(binfo, x, y);
	IPosition pos{};
	RGBA preview_color;
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		preview_color = canvas.applied_color();
		new_color = preview_color.get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.no_alpha_equivalent().get_pixel_rgba());
		binfo.storage_1c[pos] = old_color;
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

void CBImpl::Line::brush_eraser(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	auto& interp_remove = binfo.interps.line;
	IPosition pos{};
	for (unsigned int i = 0; i < interp_remove.length; ++i)
	{
		interp_remove.at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	}
	binfo.eraser_preview_image->update_subtexture(abs_rect(interp_remove.start, interp_remove.finish, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));

	auto& interp = line_brush_setup_interp(binfo, x, y);
	RGBA preview_color;
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos);
		preview_color = canvas.applied_color();
		new_color = preview_color.get_pixel_rgba();
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
		binfo.storage_1c[pos] = old_color;
	}
	binfo.eraser_preview_image->update_subtexture(abs_rect(interp.start, interp.finish, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

void CBImpl::Line::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}

void CBImpl::Line::start(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.show_preview = true;
	auto& interp = binfo.interps.line;
	interp.start = interp.finish = binfo.starting_pos;
	interp.sync_with_endpoints();
}

void CBImpl::Line::submit_pencil(Canvas& canvas)
{
	standard_submit_pencil(canvas);
}

void CBImpl::Line::submit_pen(Canvas& canvas)
{
	standard_submit_pen(canvas);
}

void CBImpl::Line::submit_eraser(Canvas& canvas)
{
	standard_submit_eraser(canvas);
}

static void line_reset_pencil_and_pen(BrushInfo& binfo)
{
	auto& interp = binfo.interps.line;
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

void CBImpl::Line::reset_pencil(BrushInfo& binfo)
{
	line_reset_pencil_and_pen(binfo);
}

void CBImpl::Line::reset_pen(BrushInfo& binfo)
{
	line_reset_pencil_and_pen(binfo);
}

void CBImpl::Line::reset_eraser(BrushInfo& binfo)
{
	auto& interp = binfo.interps.line;
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, BrushInfo::eraser_preview_img_sx * pos.x, BrushInfo::eraser_preview_img_sy * pos.y,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy, 0);
	}
	binfo.eraser_preview_image->update_subtexture(abs_rect(interp.start, interp.finish, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

// <<<==================================<<< RECT OUTLINE >>>==================================>>>

static void rect_outline_update_subtexture_pen_and_pencil(BrushInfo& binfo)
{
	auto rgns = binfo.interps.rect_outline.lines();
	for (IntRect r : rgns)
		binfo.preview_image->update_subtexture(r);
}

static void rect_outline_update_subtexture_eraser(BrushInfo& binfo)
{
	auto rgns = binfo.interps.rect_outline.lines();
	for (IntRect r : rgns)
		binfo.eraser_preview_image->update_subtexture(r.scaled(BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

static void rect_outline_brush_remove_pencil_and_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	auto& interp = binfo.interps.rect_outline;
	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	rect_outline_update_subtexture_pen_and_pencil(binfo);
}

static DiscreteRectOutlineInterpolator& rect_outline_brush_setup_interp(BrushInfo& binfo, int x, int y)
{
	auto& interp = binfo.interps.rect_outline;
	interp.finish = { x, y };
	interp.sync_with_endpoints();
	return interp;
}

void CBImpl::RectOutline::brush_pencil(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_2c.clear();
	rect_outline_brush_remove_pencil_and_pen(canvas);
	auto& interp = rect_outline_brush_setup_interp(binfo, x, y);
	IPosition pos{};
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		new_color = canvas.applied_color().get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, new_color);
		new_color.blend_over(old_color);
		binfo.storage_2c[pos] = { new_color, old_color };
	}
	rect_outline_update_subtexture_pen_and_pencil(binfo);
}

void CBImpl::RectOutline::brush_pen(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	rect_outline_brush_remove_pencil_and_pen(canvas);
	auto& interp = rect_outline_brush_setup_interp(binfo, x, y);
	IPosition pos{};
	RGBA preview_color;
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		preview_color = canvas.applied_color();
		new_color = preview_color.get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.no_alpha_equivalent().get_pixel_rgba());
		binfo.storage_1c[pos] = old_color;
	}
	rect_outline_update_subtexture_pen_and_pencil(binfo);
}

void CBImpl::RectOutline::brush_eraser(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	auto& interp_remove = binfo.interps.rect_outline;
	IPosition pos{};
	for (unsigned int i = 0; i < interp_remove.length; ++i)
	{
		interp_remove.at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	}
	rect_outline_update_subtexture_eraser(binfo);

	auto& interp = rect_outline_brush_setup_interp(binfo, x, y);
	RGBA preview_color;
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos);
		preview_color = canvas.applied_color();
		new_color = preview_color.get_pixel_rgba();
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
		binfo.storage_1c[pos] = old_color;
	}
	rect_outline_update_subtexture_eraser(binfo);
}

void CBImpl::RectOutline::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}

void CBImpl::RectOutline::start(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.show_preview = true;
	auto& interp = binfo.interps.rect_outline;
	interp.start = interp.finish = binfo.starting_pos;
	interp.sync_with_endpoints();
}

void CBImpl::RectOutline::submit_pencil(Canvas& canvas)
{
	standard_submit_pencil(canvas);
}

void CBImpl::RectOutline::submit_pen(Canvas& canvas)
{
	standard_submit_pen(canvas);
}

void CBImpl::RectOutline::submit_eraser(Canvas& canvas)
{
	standard_submit_eraser(canvas);
}

static void rect_outline_reset_pencil_and_pen(BrushInfo& binfo)
{
	auto& interp = binfo.interps.rect_outline;
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	rect_outline_update_subtexture_pen_and_pencil(binfo);
}

void CBImpl::RectOutline::reset_pencil(BrushInfo& binfo)
{
	rect_outline_reset_pencil_and_pen(binfo);
}

void CBImpl::RectOutline::reset_pen(BrushInfo& binfo)
{
	rect_outline_reset_pencil_and_pen(binfo);
}

void CBImpl::RectOutline::reset_eraser(BrushInfo& binfo)
{
	auto& interp = binfo.interps.rect_outline;
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, BrushInfo::eraser_preview_img_sx * pos.x, BrushInfo::eraser_preview_img_sy * pos.y,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy, 0);
	}
	rect_outline_update_subtexture_eraser(binfo);
}

// <<<==================================<<< RECT FILL >>>==================================>>>

static void rect_fill_brush_setup_interps(BrushInfo& binfo, int x, int y)
{
	auto& new_interp = binfo.interps.temp_rect_fill;
	new_interp.start = binfo.starting_pos;
	new_interp.finish = { x, y };
	new_interp.sync_with_endpoints();
	auto& diff = binfo.interp_diffs.rect_fill;
	diff.first = &binfo.interps.rect_fill;
	diff.second = &new_interp;
	diff.sync_with_interpolators();
}

static void rect_fill_brush_pen_and_pencil_remove_loop(BrushInfo& binfo, void(*storage_remove)(BrushInfo& binfo, IPosition pos))
{
	IPosition pos{};
	auto& diff = binfo.interp_diffs.rect_fill;
	for (unsigned int i = 0; i < diff.remove_length; ++i)
	{
		diff.remove_at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
		storage_remove(binfo, pos);
	}
}

static void rect_fill_brush_pen_and_pencil_insert_loop(Canvas& canvas, void(*storage_insert)(BrushInfo& binfo, IPosition pos, PixelRGBA old_color, RGBA preview_color))
{
	IPosition pos{};
	RGBA preview_color;
	PixelRGBA old_color;
	auto& diff = canvas.binfo.interp_diffs.rect_fill;
	for (unsigned int i = 0; i < diff.insert_length; ++i)
	{
		diff.insert_at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		preview_color = canvas.applied_color();
		storage_insert(canvas.binfo, pos, old_color, preview_color);
	}
}

static void rect_fill_brush_suffix(BrushInfo& binfo, const std::shared_ptr<Image>& image, int sx = 1, int sy = 1)
{
	auto rgns = binfo.interp_diffs.rect_fill.modified_regions();
	for (IntRect r : rgns)
		image->update_subtexture(r.scaled(sx, sy));
	binfo.interps.rect_fill = binfo.interps.temp_rect_fill;
	binfo.interps.rect_fill.sync_with_endpoints();
}

void CBImpl::RectFill::brush_pencil(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	rect_fill_brush_setup_interps(binfo, x, y);
	rect_fill_brush_pen_and_pencil_remove_loop(binfo, [](BrushInfo& binfo, IPosition pos) {
		binfo.storage_2c.erase(pos);
		});
	rect_fill_brush_pen_and_pencil_insert_loop(canvas, [](BrushInfo& binfo, IPosition pos, PixelRGBA old_color, RGBA preview_color) {
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.get_pixel_rgba());
		PixelRGBA new_color = preview_color.get_pixel_rgba();
		new_color.blend_over(old_color);
		binfo.storage_2c[pos] = { new_color, old_color };
		});
	rect_fill_brush_suffix(binfo, binfo.preview_image);
}

void CBImpl::RectFill::brush_pen(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	rect_fill_brush_setup_interps(binfo, x, y);
	rect_fill_brush_pen_and_pencil_remove_loop(binfo, [](BrushInfo& binfo, IPosition pos) {
		binfo.storage_1c.erase(pos);
		});
	rect_fill_brush_pen_and_pencil_insert_loop(canvas, [](BrushInfo& binfo, IPosition pos, PixelRGBA old_color, RGBA preview_color) {
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.no_alpha_equivalent().get_pixel_rgba());
		binfo.storage_1c[pos] = old_color;
		});
	rect_fill_brush_suffix(binfo, binfo.preview_image);
}

void CBImpl::RectFill::brush_eraser(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	rect_fill_brush_setup_interps(binfo, x, y);
	auto& diff = binfo.interp_diffs.rect_fill;
	IPosition pos{};
	for (unsigned int i = 0; i < diff.remove_length; ++i)
	{
		diff.remove_at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
		binfo.storage_1c.erase(pos);
	}
	PixelRGBA old_color;
	for (unsigned int i = 0; i < diff.insert_length; ++i)
	{
		diff.insert_at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
		binfo.storage_1c[pos] = old_color;
	}
	rect_fill_brush_suffix(binfo, binfo.eraser_preview_image, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
}

void CBImpl::RectFill::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}

static void rect_fill_start_prefix(BrushInfo& binfo)
{
	binfo.show_preview = true;
	auto& interp = binfo.interps.rect_fill;
	interp.start = interp.finish = binfo.starting_pos;
	interp.sync_with_endpoints();
}

void CBImpl::RectFill::start_pencil(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	rect_fill_start_prefix(binfo);
	PixelRGBA old_color = canvas.pixel_color_at(binfo.starting_pos.x, binfo.starting_pos.y);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color);
	binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
	new_color.blend_over(old_color);
	binfo.storage_2c[binfo.starting_pos] = { new_color, old_color };
}

void CBImpl::RectFill::start_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	rect_fill_start_prefix(binfo);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color.to_rgba().no_alpha_equivalent().get_pixel_rgba());
	binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
	binfo.storage_1c[binfo.starting_pos] = new_color;
}

void CBImpl::RectFill::start_eraser(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	rect_fill_start_prefix(binfo);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_rect_alpha(binfo.eraser_preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	binfo.eraser_preview_image->update_subtexture(binfo.starting_pos.x * BrushInfo::eraser_preview_img_sx, binfo.starting_pos.y * BrushInfo::eraser_preview_img_sy,
		BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	binfo.storage_1c[binfo.starting_pos] = new_color;
}

void CBImpl::RectFill::submit_pencil(Canvas& canvas)
{
	standard_submit_pencil(canvas);
}

void CBImpl::RectFill::submit_pen(Canvas& canvas)
{
	standard_submit_pen(canvas);
}

void CBImpl::RectFill::submit_eraser(Canvas& canvas)
{
	standard_submit_eraser(canvas);
}

static void rect_fill_reset_pencil_and_pen(BrushInfo& binfo)
{
	IntBounds bbox = binfo.interp_diffs.rect_fill.second_bbox;
	DiscreteRectFillInterpolator drfi;
	drfi.start = { bbox.x1, bbox.y1 };
	drfi.finish = { bbox.x2, bbox.y2 };
	drfi.sync_with_endpoints();
	auto& buf = binfo.preview_image->buf;
	int x, y;
	for (unsigned int i = 0; i < drfi.length; ++i)
	{
		drfi.at(i, x, y);
		buffer_set_pixel_alpha(buf, x, y, 0);
	}
	binfo.preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());
}

void CBImpl::RectFill::reset_pencil(BrushInfo& binfo)
{
	rect_fill_reset_pencil_and_pen(binfo);
}

void CBImpl::RectFill::reset_pen(BrushInfo& binfo)
{
	rect_fill_reset_pencil_and_pen(binfo);
}

void CBImpl::RectFill::reset_eraser(BrushInfo& binfo)
{
	IntBounds bbox = binfo.interp_diffs.rect_fill.second_bbox;
	DiscreteRectFillInterpolator drfi;
	drfi.start = { bbox.x1, bbox.y1 };
	drfi.finish = { bbox.x2, bbox.y2 };
	drfi.sync_with_endpoints();
	auto& buf = binfo.eraser_preview_image->buf;
	int x, y;
	for (unsigned int i = 0; i < drfi.length; ++i)
	{
		drfi.at(i, x, y);
		buffer_set_rect_alpha(buf, x, y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	}
	binfo.eraser_preview_image->update_subtexture(bbox.x1 * BrushInfo::eraser_preview_img_sx, bbox.y1 * BrushInfo::eraser_preview_img_sy,
		bbox.width_no_abs() * BrushInfo::eraser_preview_img_sx, bbox.height_no_abs() * BrushInfo::eraser_preview_img_sy);
}

// <<<==================================<<< ELLIPSE OUTLINE >>>==================================>>>

static void ellipse_outline_brush_remove_pencil_and_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	auto& interp = binfo.interps.ellipse_outline;
	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

static DiscreteEllipseOutlineInterpolator& ellipse_outline_brush_setup_interp(BrushInfo& binfo, int x, int y)
{
	auto& interp = binfo.interps.ellipse_outline;
	interp.finish = { x, y };
	interp.sync_with_endpoints();
	return interp;
}

static void ellipse_outline_brush_setup_interps(BrushInfo& binfo, int x, int y)
{
	auto& new_interp = binfo.interps.temp_ellipse_outline;
	new_interp.start = binfo.starting_pos;
	new_interp.finish = { x, y };
	//new_interp.sync_with_endpoints(); // TODO remove interp_diffs.ellipse_outline, and just define utility function to compute modified regions of rect diff.
	auto& diff = binfo.interp_diffs.ellipse_outline;
	diff.first = &binfo.interps.ellipse_outline;
	diff.second = &new_interp;
	diff.sync_with_interpolators();
}

// TODO for ellipse at the very least, cursor should be able to exit canvas while brushing. This would be in Canvas::brush()

void CBImpl::EllipseOutline::brush_pencil(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_2c.clear();
	ellipse_outline_brush_setup_interps(binfo, x, y);
	ellipse_outline_brush_remove_pencil_and_pen(canvas);
	auto& interp = ellipse_outline_brush_setup_interp(binfo, x, y);
	// TODO put the following loop as a standard_brush_insert_loop, which takes a pointer to interp.
	IPosition pos{};
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		new_color = canvas.applied_color().get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, new_color);
		new_color.blend_over(old_color);
		binfo.storage_2c[pos] = { new_color, old_color };
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

void CBImpl::EllipseOutline::brush_pen(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	ellipse_outline_brush_setup_interps(binfo, x, y);
	ellipse_outline_brush_remove_pencil_and_pen(canvas);
	auto& interp = ellipse_outline_brush_setup_interp(binfo, x, y);
	IPosition pos{};
	RGBA preview_color;
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		preview_color = canvas.applied_color();
		new_color = preview_color.get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.no_alpha_equivalent().get_pixel_rgba());
		binfo.storage_1c[pos] = old_color;
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

void CBImpl::EllipseOutline::brush_eraser(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	ellipse_outline_brush_setup_interps(binfo, x, y);
	auto& interp_remove = binfo.interps.ellipse_outline;
	IPosition pos{};
	for (unsigned int i = 0; i < interp_remove.length; ++i)
	{
		interp_remove.at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	}
	binfo.eraser_preview_image->update_subtexture(abs_rect(interp_remove.start, interp_remove.finish).scaled(BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));

	auto& interp = ellipse_outline_brush_setup_interp(binfo, x, y);
	RGBA preview_color;
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos);
		preview_color = canvas.applied_color();
		new_color = preview_color.get_pixel_rgba();
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
		binfo.storage_1c[pos] = old_color;
	}
	binfo.eraser_preview_image->update_subtexture(abs_rect(interp.start, interp.finish).scaled(BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

void CBImpl::EllipseOutline::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}

static void ellipse_outline_start_prefix(BrushInfo& binfo)
{
	binfo.show_preview = true;
	auto& interp = binfo.interps.ellipse_outline;
	interp.start = interp.finish = binfo.starting_pos;
	interp.sync_with_endpoints();
}

void CBImpl::EllipseOutline::start_pencil(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	ellipse_outline_start_prefix(binfo);
	PixelRGBA old_color = canvas.pixel_color_at(binfo.starting_pos.x, binfo.starting_pos.y);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color);
	binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
	new_color.blend_over(old_color);
	binfo.storage_2c[binfo.starting_pos] = { new_color, old_color };
}

void CBImpl::EllipseOutline::start_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	ellipse_outline_start_prefix(binfo);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color.to_rgba().no_alpha_equivalent().get_pixel_rgba());
	binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
	binfo.storage_1c[binfo.starting_pos] = new_color;
}

void CBImpl::EllipseOutline::start_eraser(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	ellipse_outline_start_prefix(binfo);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_rect_alpha(binfo.eraser_preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	binfo.eraser_preview_image->update_subtexture(binfo.starting_pos.x * BrushInfo::eraser_preview_img_sx, binfo.starting_pos.y * BrushInfo::eraser_preview_img_sy,
		BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	binfo.storage_1c[binfo.starting_pos] = new_color;
}

void CBImpl::EllipseOutline::submit_pencil(Canvas& canvas)
{
	standard_submit_pencil(canvas);
}

void CBImpl::EllipseOutline::submit_pen(Canvas& canvas)
{
	standard_submit_pen(canvas);
}

void CBImpl::EllipseOutline::submit_eraser(Canvas& canvas)
{
	standard_submit_eraser(canvas);
}

static void ellipse_outline_reset_pencil_and_pen(BrushInfo& binfo)
{
	IntBounds bbox = binfo.interp_diffs.ellipse_outline.second_bbox;
	DiscreteEllipseOutlineInterpolator deoi;
	deoi.start = { bbox.x1, bbox.y1 };
	deoi.finish = { bbox.x2, bbox.y2 };
	deoi.sync_with_endpoints();
	auto& buf = binfo.preview_image->buf;
	int x, y;
	for (unsigned int i = 0; i < deoi.length; ++i)
	{
		deoi.at(i, x, y);
		buffer_set_pixel_alpha(buf, x, y, 0);
	}
	binfo.preview_image->update_subtexture(bbox.x1, bbox.y1, bbox.width_no_abs(), bbox.height_no_abs());
}

void CBImpl::EllipseOutline::reset_pencil(BrushInfo& binfo)
{
	ellipse_outline_reset_pencil_and_pen(binfo);
}

void CBImpl::EllipseOutline::reset_pen(BrushInfo& binfo)
{
	ellipse_outline_reset_pencil_and_pen(binfo);
}

void CBImpl::EllipseOutline::reset_eraser(BrushInfo& binfo)
{
	IntBounds bbox = binfo.interp_diffs.ellipse_outline.second_bbox;
	DiscreteEllipseOutlineInterpolator deoi;
	deoi.start = { bbox.x1, bbox.y1 };
	deoi.finish = { bbox.x2, bbox.y2 };
	deoi.sync_with_endpoints();
	auto& buf = binfo.eraser_preview_image->buf;
	int x, y;
	for (unsigned int i = 0; i < deoi.length; ++i)
	{
		deoi.at(i, x, y);
		buffer_set_rect_alpha(buf, x, y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	}
	binfo.eraser_preview_image->update_subtexture(bbox.x1 * BrushInfo::eraser_preview_img_sx, bbox.y1 * BrushInfo::eraser_preview_img_sy,
		bbox.width_no_abs() * BrushInfo::eraser_preview_img_sx, bbox.height_no_abs() * BrushInfo::eraser_preview_img_sy);
}
