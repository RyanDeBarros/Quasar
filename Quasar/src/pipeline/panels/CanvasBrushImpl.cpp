#include "CanvasBrushImpl.h"

#include "Easel.h"
#include "user/Machine.h"

void CBImpl::Camera::brush(Canvas& canvas, int x, int y)
{
	// do nothing
}

static Buffer paint_brush_prefix(Canvas& canvas, int x, int y)
{
	Canvas::Brush& binfo = canvas.binfo;
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
	Canvas::Brush& binfo = canvas.binfo;
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
	Canvas::Brush& binfo = canvas.binfo;
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
	Canvas::Brush& binfo = canvas.binfo;
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

static void line_brush_remove_pencil_and_pen(Canvas::Brush& binfo)
{
	auto& interp = binfo.interps.line;
	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
}

static DiscreteLineInterpolator& line_brush_setup_interp(Canvas::Brush& binfo, int x, int y)
{
	auto& interp = binfo.interps.line;
	interp.start = binfo.starting_pos;
	interp.finish = { x, y };
	interp.sync_with_endpoints();
	return interp;
}

void CBImpl::Line::brush_pencil(Canvas& canvas, int x, int y)
{
	Canvas::Brush& binfo = canvas.binfo;
	binfo.storage_2c.clear();
	line_brush_remove_pencil_and_pen(binfo);
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
	Canvas::Brush& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	line_brush_remove_pencil_and_pen(binfo);
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
	Canvas::Brush& binfo = canvas.binfo;
	binfo.storage_1c.clear();
	auto& interp_remove = binfo.interps.line;
	IPosition pos{};
	for (unsigned int i = 0; i < interp_remove.length; ++i)
	{
		interp_remove.at(i, pos);
		// TODO here and for rect fill, use eraser_arr_w/h instead of 2
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, 2, 2);
	}
	binfo.eraser_preview_image->update_subtexture(abs_rect(interp_remove.start, interp_remove.finish, 2, 2));

	auto& interp = line_brush_setup_interp(binfo, x, y);
	RGBA preview_color;
	PixelRGBA new_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos);
		old_color = canvas.pixel_color_at(pos);
		preview_color = canvas.applied_color();
		new_color = preview_color.get_pixel_rgba();
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, 2, 2);
		binfo.storage_1c[pos] = old_color;
	}
	binfo.eraser_preview_image->update_subtexture(abs_rect(interp.start, interp.finish, 2, 2));
}

void CBImpl::Line::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}

void CBImpl::Line::start(Canvas& canvas)
{
	canvas.binfo.show_preview = true;
}

void CBImpl::Line::submit_pencil(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_2c.empty())
		Machine.history.execute(std::make_shared<OneColorPencilAction>(canvas.image, binfo.starting_pos, binfo.brush_pos, std::move(binfo.storage_2c)));
}

void CBImpl::Line::submit_pen(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
			binfo.starting_pos, binfo.brush_pos, std::move(binfo.storage_1c)));
}

void CBImpl::Line::submit_eraser(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 },
			binfo.starting_pos, binfo.brush_pos, std::move(binfo.storage_1c)));
}

void CBImpl::Line::submit_select(Canvas& canvas)
{
	// LATER
}

static void rect_fill_brush_setup_interps(Canvas::Brush& binfo, int x, int y, DiscreteRectFillInterpolator& new_interp, DiscreteRectFillDifference*& diff)
{
	new_interp.start = binfo.starting_pos;
	new_interp.finish = { x, y };
	new_interp.sync_with_endpoints();
	diff = &binfo.interp_diffs.rect_fill;
	diff->first = binfo.interps.rect_fill;
	diff->second = new_interp;
	diff->sync_with_interpolators();
}

static void rect_fill_brush_suffix(Canvas::Brush& binfo, const std::shared_ptr<Image>& image,
	const DiscreteRectFillInterpolator& new_interp, IntRect first_rect, IntRect second_rect)
{
	image->update_subtexture(first_rect);
	image->update_subtexture(second_rect);
	binfo.interps.rect_fill = new_interp;
	binfo.interps.rect_fill.sync_with_endpoints();
}

static void rect_fill_brush_pen_and_pencil_remove_loop(Canvas::Brush& binfo, DiscreteRectFillDifference* diff,
	void(*storage_remove)(Canvas::Brush& binfo, IPosition pos))
{
	IPosition pos{};
	for (unsigned int i = 0; i < diff->remove_length; ++i)
	{
		diff->remove_at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
		storage_remove(binfo, pos);
	}
}

static void rect_fill_brush_pen_and_pencil_insert_loop(Canvas& canvas, DiscreteRectFillDifference* diff, 
	void(*storage_insert)(Canvas::Brush& binfo, IPosition pos, PixelRGBA old_color, RGBA preview_color))
{
	IPosition pos{};
	RGBA preview_color;
	PixelRGBA old_color;
	for (unsigned int i = 0; i < diff->insert_length; ++i)
	{
		diff->insert_at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		preview_color = canvas.applied_color();
		storage_insert(canvas.binfo, pos, old_color, preview_color);
	}
}

void CBImpl::RectFill::brush_pencil(Canvas& canvas, int x, int y)
{
	Canvas::Brush& binfo = canvas.binfo;
	DiscreteRectFillInterpolator new_interp;
	DiscreteRectFillDifference* diff;
	rect_fill_brush_setup_interps(binfo, x, y, new_interp, diff);
	rect_fill_brush_pen_and_pencil_remove_loop(binfo, diff, [](Canvas::Brush& binfo, IPosition pos) {
		binfo.storage_2c.erase(pos);
		});
	rect_fill_brush_pen_and_pencil_insert_loop(canvas, diff, [](Canvas::Brush& binfo, IPosition pos, PixelRGBA old_color, RGBA preview_color) {
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.get_pixel_rgba());
		PixelRGBA new_color = preview_color.get_pixel_rgba();
		new_color.blend_over(old_color);
		binfo.storage_2c[pos] = { new_color, old_color };
		});
	rect_fill_brush_suffix(binfo, binfo.preview_image, new_interp, bounds_to_rect(diff->first_bbox), bounds_to_rect(diff->second_bbox));
}

void CBImpl::RectFill::brush_pen(Canvas& canvas, int x, int y)
{
	Canvas::Brush& binfo = canvas.binfo;
	DiscreteRectFillInterpolator new_interp;
	DiscreteRectFillDifference* diff;
	rect_fill_brush_setup_interps(binfo, x, y, new_interp, diff);
	rect_fill_brush_pen_and_pencil_remove_loop(binfo, diff, [](Canvas::Brush& binfo, IPosition pos) {
		binfo.storage_1c.erase(pos);
		});
	rect_fill_brush_pen_and_pencil_insert_loop(canvas, diff, [](Canvas::Brush& binfo, IPosition pos, PixelRGBA old_color, RGBA preview_color) {
		buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.no_alpha_equivalent().get_pixel_rgba());
		binfo.storage_1c[pos] = old_color;
		});
	rect_fill_brush_suffix(binfo, binfo.preview_image, new_interp, bounds_to_rect(diff->first_bbox), bounds_to_rect(diff->second_bbox));
}

void CBImpl::RectFill::brush_eraser(Canvas& canvas, int x, int y)
{
	Canvas::Brush& binfo = canvas.binfo;
	DiscreteRectFillInterpolator new_interp;
	DiscreteRectFillDifference* diff;
	rect_fill_brush_setup_interps(binfo, x, y, new_interp, diff);
	IPosition pos{};
	for (unsigned int i = 0; i < diff->remove_length; ++i)
	{
		diff->remove_at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, 2, 2);
		binfo.storage_1c.erase(pos);
	}
	PixelRGBA old_color;
	for (unsigned int i = 0; i < diff->insert_length; ++i)
	{
		diff->insert_at(i, pos.x, pos.y);
		old_color = canvas.pixel_color_at(pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, 2, 2);
		binfo.storage_1c[pos] = old_color;
	}
	rect_fill_brush_suffix(binfo, binfo.eraser_preview_image, new_interp, bounds_to_rect(diff->first_bbox, 2, 2), bounds_to_rect(diff->second_bbox, 2, 2));
}

void CBImpl::RectFill::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}

static void rect_fill_start_prefix(Canvas::Brush& binfo)
{
	binfo.show_preview = true;
	binfo.interps.rect_fill.start = binfo.interps.rect_fill.finish = binfo.starting_pos;
	binfo.interps.rect_fill.sync_with_endpoints();
}

void CBImpl::RectFill::start_pencil(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	rect_fill_start_prefix(binfo);
	if (binfo.starting_pos != IPosition{ -1, -1 })
	{
		PixelRGBA old_color = canvas.pixel_color_at(binfo.starting_pos.x, binfo.starting_pos.y);
		PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color);
		binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
		new_color.blend_over(old_color);
		binfo.storage_2c[binfo.starting_pos] = { new_color, old_color };
	}
}

void CBImpl::RectFill::start_pen(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	rect_fill_start_prefix(binfo);
	if (binfo.starting_pos != IPosition{ -1, -1 }) // TODO if starting brush, would this really ever fail? If it can fail, consider putting the above in if-statement as well?
	{
		PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
		buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color.to_rgba().no_alpha_equivalent().get_pixel_rgba());
		binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
		binfo.storage_1c[binfo.starting_pos] = new_color;
	}
}

void CBImpl::RectFill::start_eraser(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	rect_fill_start_prefix(binfo);
	if (binfo.starting_pos != IPosition{ -1, -1 })
	{
		PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, 1, 1, 255, 2, 2);
		binfo.eraser_preview_image->update_subtexture(binfo.starting_pos.x * 2, binfo.starting_pos.y * 2, 2, 2);
		binfo.storage_1c[binfo.starting_pos] = new_color;
	}
}

void CBImpl::RectFill::start_select(Canvas& canvas)
{
	// LATER
}

void CBImpl::RectFill::submit_pencil(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_2c.empty())
		Machine.history.execute(std::make_shared<OneColorPencilAction>(canvas.image, binfo.starting_pos, binfo.brush_pos, std::move(binfo.storage_2c)));
}

void CBImpl::RectFill::submit_pen(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
			binfo.starting_pos, binfo.brush_pos, std::move(binfo.storage_1c)));
}

void CBImpl::RectFill::submit_eraser(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 },
			binfo.starting_pos, binfo.brush_pos, std::move(binfo.storage_1c)));
}

void CBImpl::RectFill::submit_select(Canvas& canvas)
{
	// LATER
}
