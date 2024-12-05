#include "CanvasBrushImpl.h"

#include "Easel.h"
#include "user/Machine.h"

static void update_bbox(IntBounds& bbox, int x, int y)
{
	if (x < bbox.x1)
		bbox.x1 = x;
	if (x > bbox.x2)
		bbox.x2 = x;
	if (y < bbox.y1)
		bbox.y1 = y;
	if (y > bbox.y2)
		bbox.y2 = y;
}

static void _standard_outline_brush_pencil_looperand_update_storage(Canvas& canvas, IPosition pos, void(*update_subtexture)(BrushInfo& binfo))
{
	PixelRGBA old_color = canvas.pixel_color_at(pos.x, pos.y);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_pixel_color(canvas.binfo.preview_image->buf, pos.x, pos.y, new_color);
	new_color.blend_over(old_color);
	canvas.binfo.storage_2c[pos] = { new_color, old_color };
}

static void _standard_outline_brush_pencil_looperand_dont_update_storage(Canvas& canvas, IPosition pos, void(*update_subtexture)(BrushInfo& binfo))
{
	buffer_set_pixel_color(canvas.binfo.preview_image->buf, pos.x, pos.y, canvas.applied_color().get_pixel_rgba());
}

static void standard_outline_brush_pencil(Canvas& canvas, int x, int y, DiscreteInterpolator& interp, void(*update_subtexture)(BrushInfo& binfo), bool update_storage = true)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.last_brush_pos = { x, y };
	if (update_storage)
		binfo.storage_2c.clear();

	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	update_subtexture(binfo);

	interp.finish = { x, y };
	interp.sync_with_endpoints();
	auto looperand = update_storage ? &_standard_outline_brush_pencil_looperand_update_storage : &_standard_outline_brush_pencil_looperand_dont_update_storage;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		looperand(canvas, pos, update_subtexture);
	}
	update_subtexture(binfo);
}

static void _standard_outline_brush_pen_looperand_dont_update_storage(Canvas& canvas, IPosition pos, void(*update_subtexture)(BrushInfo& binfo))
{
	buffer_set_pixel_color(canvas.binfo.preview_image->buf, pos.x, pos.y, canvas.applied_color().no_alpha_equivalent().get_pixel_rgba());
}

static void _standard_outline_brush_pen_looperand_update_storage(Canvas& canvas, IPosition pos, void(*update_subtexture)(BrushInfo& binfo))
{
	_standard_outline_brush_pen_looperand_dont_update_storage(canvas, pos, update_subtexture);
	canvas.binfo.storage_1c[pos] = canvas.pixel_color_at(pos.x, pos.y);
}

static void standard_outline_brush_pen(Canvas& canvas, int x, int y, DiscreteInterpolator& interp, void(*update_subtexture)(BrushInfo& binfo), bool update_storage = true)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.last_brush_pos = { x, y };
	if (update_storage)
		binfo.storage_1c.clear();

	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	update_subtexture(binfo);

	interp.finish = { x, y };
	interp.sync_with_endpoints();
	auto looperand = update_storage ? &_standard_outline_brush_pen_looperand_update_storage : &_standard_outline_brush_pen_looperand_dont_update_storage;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		looperand(canvas, pos, update_subtexture);
	}
	update_subtexture(binfo);
}

static void _standard_outline_brush_eraser_looperand_dont_update_storage(Canvas& canvas, IPosition pos, void(*update_subtexture)(BrushInfo& binfo))
{
	buffer_set_rect_alpha(canvas.binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
}

static void _standard_outline_brush_eraser_looperand_update_storage(Canvas& canvas, IPosition pos, void(*update_subtexture)(BrushInfo& binfo))
{
	_standard_outline_brush_eraser_looperand_dont_update_storage(canvas, pos, update_subtexture);
	canvas.binfo.storage_1c[pos] = canvas.pixel_color_at(pos.x, pos.y);
}

static void standard_outline_brush_eraser(Canvas& canvas, int x, int y, DiscreteInterpolator& interp, void(*update_subtexture)(BrushInfo& binfo), bool update_storage = true)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.last_brush_pos = { x, y };
	if (update_storage)
		binfo.storage_1c.clear();

	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	}
	update_subtexture(binfo);

	interp.finish = { x, y };
	interp.sync_with_endpoints();
	auto looperand = update_storage ? &_standard_outline_brush_eraser_looperand_update_storage : &_standard_outline_brush_eraser_looperand_dont_update_storage;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		looperand(canvas, pos, update_subtexture);
	}
	update_subtexture(binfo);
}

static void standard_submit_pencil(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_2c.empty())
		Machine.history.execute(std::make_shared<OneColorPencilAction>(canvas.image, binfo.starting_pos, binfo.last_brush_pos, std::move(binfo.storage_2c)));
}

static void standard_push_pencil(Canvas& canvas, IntBounds bbox)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_2c.empty())
		Machine.history.push(std::make_shared<OneColorPencilAction>(canvas.image, IPosition{ bbox.x1, bbox.y1 }, IPosition{ bbox.x2, bbox.y2 }, std::move(binfo.storage_2c)));
}

static void standard_submit_filled_pencil(Canvas& canvas, DiscreteInterpolator& interp)
{
	BrushInfo& binfo = canvas.binfo;
	if (binfo.last_brush_pos != IPosition{ -1, -1 })
	{
		binfo.storage_2c.clear();
		interp.start = binfo.starting_pos;
		interp.finish = binfo.last_brush_pos;
		interp.sync_with_endpoints();
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos.x, pos.y);
			PixelRGBA old_color = canvas.pixel_color_at(pos.x, pos.y);
			PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
			new_color.blend_over(old_color);
			canvas.binfo.storage_2c[pos] = { new_color, old_color };
		}
		standard_submit_pencil(canvas);
	}
}

static void standard_submit_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
			binfo.starting_pos, binfo.last_brush_pos, std::move(binfo.storage_1c)));
}

static void standard_push_pen(Canvas& canvas, IntBounds bbox)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.push(std::make_shared<OneColorPenAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
			IPosition{ bbox.x1, bbox.y1 }, IPosition{ bbox.x2, bbox.y2 }, std::move(binfo.storage_1c)));
}

static void standard_submit_filled_pen(Canvas& canvas, DiscreteInterpolator& interp)
{
	BrushInfo& binfo = canvas.binfo;
	if (binfo.last_brush_pos != IPosition{ -1, -1 })
	{
		binfo.storage_1c.clear();
		interp.start = binfo.starting_pos;
		interp.finish = binfo.last_brush_pos;
		interp.sync_with_endpoints();
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos.x, pos.y);
			canvas.binfo.storage_1c[pos] = canvas.pixel_color_at(pos.x, pos.y);
		}
		standard_submit_pen(canvas);
	}
}

static void standard_submit_eraser(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 },
			binfo.starting_pos, binfo.last_brush_pos, std::move(binfo.storage_1c)));
}

static void standard_push_eraser(Canvas& canvas, IntBounds bbox)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.push(std::make_shared<OneColorPenAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 },
			IPosition{ bbox.x1, bbox.y1 }, IPosition{ bbox.x2, bbox.y2 }, std::move(binfo.storage_1c)));
}

static void standard_submit_filled_eraser(Canvas& canvas, DiscreteInterpolator& interp)
{
	BrushInfo& binfo = canvas.binfo;
	if (binfo.last_brush_pos != IPosition{ -1, -1 })
	{
		binfo.storage_1c.clear();
		interp.start = binfo.starting_pos;
		interp.finish = binfo.last_brush_pos;
		interp.sync_with_endpoints();
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos.x, pos.y);
			canvas.binfo.storage_1c[pos] = canvas.pixel_color_at(pos.x, pos.y);
		}
		standard_submit_eraser(canvas);
	}
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
// LATER Paint should use standard submission techniques specific to pencil, pen, and eraser, in the exact same way as Line, RectFill, RectOutline, etc.

static Buffer paint_brush_prefix(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	Buffer image_shifted_buf = canvas.image->buf;
	image_shifted_buf.pixels += image_shifted_buf.byte_offset(x, y);
	update_bbox(binfo.brushing_bbox, x, y);
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
	PixelRGBA color = canvas.applied_pencil_rgba();
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
	PixelRGBA color = canvas.applied_pen_rgba();
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

// LATER use storage_1c for pen and eraser, and separate submit function into one for each brush tip. use standard submit
void CBImpl::Paint::brush_submit(Canvas& canvas)
{
	if (canvas.image && !canvas.binfo.storage_2c.empty())
		Machine.history.push(std::make_shared<PaintToolAction>(canvas.image, canvas.binfo.brushing_bbox, std::move(canvas.binfo.storage_2c)));
}

// <<<==================================<<< LINE >>>==================================>>>

static void line_alternate_apply(IPosition starting_pos, int& x, int& y)
{
	int dx = x - starting_pos.x;
	if (dx == 0)
		return;
	int dy = y - starting_pos.y;
	if (dy == 0)
		return;
	int adx = std::abs(dx);
	int ady = std::abs(dy);
	if (adx < ady)
		y = starting_pos.y + glm::sign(dy) * (adx * int(ady / adx));
	else
		x = starting_pos.x + glm::sign(dx) * (ady * int(adx / ady));
}

void CBImpl::Line::brush_pencil(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		line_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_pencil(canvas, x, y, canvas.binfo.interps.line, [](BrushInfo& binfo) {
		binfo.preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish));
		});
}

void CBImpl::Line::brush_pen(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		line_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_pen(canvas, x, y, canvas.binfo.interps.line, [](BrushInfo& binfo) {
		binfo.preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish));
		});
}

void CBImpl::Line::brush_eraser(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		line_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_eraser(canvas, x, y, canvas.binfo.interps.line, [](BrushInfo& binfo) {
		binfo.eraser_preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
		});
}

void CBImpl::Line::brush_select(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		line_alternate_apply(canvas.binfo.starting_pos, x, y);
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

static void line_reset(DiscreteInterpolator& interp, Buffer& buf, void(*buffer_remove)(Buffer& buf, IPosition pos))
{
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_remove(buf, pos);
	}
}

void CBImpl::Line::reset_pencil(BrushInfo& binfo)
{
	line_reset(binfo.interps.line, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish));
}

void CBImpl::Line::reset_pen(BrushInfo& binfo)
{
	line_reset(binfo.interps.line, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish));
}

void CBImpl::Line::reset_eraser(BrushInfo& binfo)
{
	line_reset(binfo.interps.line, binfo.eraser_preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_rect_alpha(buf, BrushInfo::eraser_preview_img_sx * pos.x, BrushInfo::eraser_preview_img_sy * pos.y,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy, 0);
		});
	binfo.eraser_preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish,
		BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

// <<<==================================<<< RECT OUTLINE >>>==================================>>>

static void rect_alternate_apply(IPosition starting_pos, int& x, int& y)
{
	int dx = x - starting_pos.x;
	int dy = y - starting_pos.y;
	int adx = std::abs(dx);
	int ady = std::abs(dy);
	if (adx < ady)
		y = starting_pos.y + glm::sign(dy) * adx;
	else if (adx > ady)
		x = starting_pos.x + glm::sign(dx) * ady;
}

static void rect_outline_pencil_and_pen_update_subtexture(BrushInfo& binfo)
{
	auto rgns = binfo.interps.rect_outline.lines();
	for (IntRect r : rgns)
		binfo.preview_image->update_subtexture(r);
}

static void rect_outline_eraser_update_subtexture(BrushInfo& binfo)
{
	auto rgns = binfo.interps.rect_outline.lines();
	for (IntRect r : rgns)
		binfo.eraser_preview_image->update_subtexture(r.scaled(BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

void CBImpl::RectOutline::brush_pencil(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_pencil(canvas, x, y, canvas.binfo.interps.rect_outline, &rect_outline_pencil_and_pen_update_subtexture);
}

void CBImpl::RectOutline::brush_pen(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_pen(canvas, x, y, canvas.binfo.interps.rect_outline, &rect_outline_pencil_and_pen_update_subtexture);
}

void CBImpl::RectOutline::brush_eraser(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_eraser(canvas, x, y, canvas.binfo.interps.rect_outline, &rect_outline_eraser_update_subtexture);
}

void CBImpl::RectOutline::brush_select(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
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

static void rect_outline_reset(DiscreteInterpolator& interp, Buffer& buf, void(*buffer_remove)(Buffer& buf, IPosition pos))
{
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		buffer_remove(buf, pos);
	}
}

void CBImpl::RectOutline::reset_pencil(BrushInfo& binfo)
{
	rect_outline_reset(binfo.interps.rect_outline, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	rect_outline_pencil_and_pen_update_subtexture(binfo);
}

void CBImpl::RectOutline::reset_pen(BrushInfo& binfo)
{
	rect_outline_reset(binfo.interps.rect_outline, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	rect_outline_pencil_and_pen_update_subtexture(binfo);
}

void CBImpl::RectOutline::reset_eraser(BrushInfo& binfo)
{
	rect_outline_reset(binfo.interps.rect_outline, binfo.eraser_preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_rect_alpha(buf, BrushInfo::eraser_preview_img_sx * pos.x, BrushInfo::eraser_preview_img_sy * pos.y,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy, 0);
		});
	rect_outline_eraser_update_subtexture(binfo);
}

// <<<==================================<<< RECT FILL >>>==================================>>>

void CBImpl::RectFill::brush_pencil(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_pencil(canvas, x, y, canvas.binfo.interps.rect_outline, &rect_outline_pencil_and_pen_update_subtexture, false);
}

void CBImpl::RectFill::brush_pen(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_pen(canvas, x, y, canvas.binfo.interps.rect_outline, &rect_outline_pencil_and_pen_update_subtexture, false);
}

void CBImpl::RectFill::brush_eraser(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
	standard_outline_brush_eraser(canvas, x, y, canvas.binfo.interps.rect_outline, &rect_outline_eraser_update_subtexture, false);
}

void CBImpl::RectFill::brush_select(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		rect_alternate_apply(canvas.binfo.starting_pos, x, y);
	// LATER
}

void CBImpl::RectFill::start(Canvas& canvas)
{
	CBImpl::RectOutline::start(canvas);
}

void CBImpl::RectFill::submit_pencil(Canvas& canvas)
{
	standard_submit_filled_pencil(canvas, canvas.binfo.interps.rect_fill);
}

void CBImpl::RectFill::submit_pen(Canvas& canvas)
{
	standard_submit_filled_pen(canvas, canvas.binfo.interps.rect_fill);
}

void CBImpl::RectFill::submit_eraser(Canvas& canvas)
{
	standard_submit_filled_eraser(canvas, canvas.binfo.interps.rect_fill);
}

void CBImpl::RectFill::reset_pencil(BrushInfo& binfo)
{
	CBImpl::RectOutline::reset_pencil(binfo);
}

void CBImpl::RectFill::reset_pen(BrushInfo& binfo)
{
	CBImpl::RectOutline::reset_pen(binfo);
}

void CBImpl::RectFill::reset_eraser(BrushInfo& binfo)
{
	CBImpl::RectOutline::reset_eraser(binfo);
}

// <<<==================================<<< ELLIPSE OUTLINE >>>==================================>>>

// LATER for ellipse at the very least, cursor should be able to exit canvas while brushing. This would be in Canvas::brush(), with modifications in start()/submit()

static void ellipse_alternate_apply(IPosition starting_pos, int& x, int& y)
{
	int dx = x - starting_pos.x;
	int dy = y - starting_pos.y;
	int adx = std::abs(dx);
	int ady = std::abs(dy);
	if (adx < ady)
		y = starting_pos.y + glm::sign(dy) * adx;
	else if (adx > ady)
		x = starting_pos.x + glm::sign(dx) * ady;
}

void CBImpl::EllipseOutline::brush_pencil(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	canvas.binfo.brushing_bbox = abs_bounds(canvas.binfo.starting_pos, { x, y });
	standard_outline_brush_pencil(canvas, x, y, canvas.binfo.interps.ellipse_outline, [](BrushInfo& binfo) {
		binfo.preview_image->update_subtexture(abs_rect(binfo.interps.ellipse_outline.start, binfo.interps.ellipse_outline.finish));
		});
}

void CBImpl::EllipseOutline::brush_pen(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	canvas.binfo.brushing_bbox = abs_bounds(canvas.binfo.starting_pos, { x, y });
	standard_outline_brush_pen(canvas, x, y, canvas.binfo.interps.ellipse_outline, [](BrushInfo& binfo) {
		binfo.preview_image->update_subtexture(abs_rect(binfo.interps.ellipse_outline.start, binfo.interps.ellipse_outline.finish));
		});
}

void CBImpl::EllipseOutline::brush_eraser(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	canvas.binfo.brushing_bbox = abs_bounds(canvas.binfo.starting_pos, { x, y });
	standard_outline_brush_eraser(canvas, x, y, canvas.binfo.interps.ellipse_outline, [](BrushInfo& binfo) {
		binfo.eraser_preview_image->update_subtexture(abs_rect(binfo.interps.ellipse_outline.start, binfo.interps.ellipse_outline.finish,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
		});
}

void CBImpl::EllipseOutline::brush_select(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	// LATER
}

void CBImpl::EllipseOutline::start(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.show_preview = true;
	auto& interp = binfo.interps.ellipse_outline;
	interp.start = interp.finish = binfo.starting_pos;
	interp.sync_with_endpoints();
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

static void ellipse_outline_reset(BrushInfo& binfo, Buffer& buf, void(*buffer_remove)(Buffer& buf, IPosition pos))
{
	IntBounds bbox = binfo.brushing_bbox;
	DiscreteEllipseOutlineInterpolator deoi;
	deoi.start = { bbox.x1, bbox.y1 };
	deoi.finish = { bbox.x2, bbox.y2 };
	deoi.sync_with_endpoints();
	IPosition pos{};
	for (unsigned int i = 0; i < deoi.length; ++i)
	{
		deoi.at(i, pos.x, pos.y);
		buffer_remove(buf, pos);
	}
}

void CBImpl::EllipseOutline::reset_pencil(BrushInfo& binfo)
{
	ellipse_outline_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(bounds_to_rect(binfo.brushing_bbox));
}

void CBImpl::EllipseOutline::reset_pen(BrushInfo& binfo)
{
	ellipse_outline_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(bounds_to_rect(binfo.brushing_bbox));
}

void CBImpl::EllipseOutline::reset_eraser(BrushInfo& binfo)
{
	ellipse_outline_reset(binfo, binfo.eraser_preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_rect_alpha(buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
		});
	binfo.eraser_preview_image->update_subtexture(bounds_to_rect(binfo.brushing_bbox, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

// <<<==================================<<< ELLIPSE FILL >>>==================================>>>

void CBImpl::EllipseFill::brush_pencil(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	canvas.binfo.brushing_bbox = abs_bounds(canvas.binfo.starting_pos, { x, y });
	standard_outline_brush_pencil(canvas, x, y, canvas.binfo.interps.ellipse_outline, [](BrushInfo& binfo) {
		binfo.preview_image->update_subtexture(abs_rect(binfo.interps.ellipse_outline.start, binfo.interps.ellipse_outline.finish));
		}, false);
}

void CBImpl::EllipseFill::brush_pen(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	canvas.binfo.brushing_bbox = abs_bounds(canvas.binfo.starting_pos, { x, y });
	standard_outline_brush_pen(canvas, x, y, canvas.binfo.interps.ellipse_outline, [](BrushInfo& binfo) {
		binfo.preview_image->update_subtexture(abs_rect(binfo.interps.ellipse_outline.start, binfo.interps.ellipse_outline.finish));
		}, false);
}

void CBImpl::EllipseFill::brush_eraser(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	canvas.binfo.brushing_bbox = abs_bounds(canvas.binfo.starting_pos, { x, y });
	standard_outline_brush_eraser(canvas, x, y, canvas.binfo.interps.ellipse_outline, [](BrushInfo& binfo) {
		binfo.eraser_preview_image->update_subtexture(abs_rect(binfo.interps.ellipse_outline.start, binfo.interps.ellipse_outline.finish,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
		}, false);
}

void CBImpl::EllipseFill::brush_select(Canvas& canvas, int x, int y)
{
	if (MainWindow->is_shift_pressed())
		ellipse_alternate_apply(canvas.binfo.starting_pos, x, y);
	// LATER
}

void CBImpl::EllipseFill::start(Canvas& canvas)
{
	CBImpl::EllipseOutline::start(canvas);
}

void CBImpl::EllipseFill::submit_pencil(Canvas& canvas)
{
	standard_submit_filled_pencil(canvas, canvas.binfo.interps.ellipse_fill);
}

void CBImpl::EllipseFill::submit_pen(Canvas& canvas)
{
	standard_submit_filled_pen(canvas, canvas.binfo.interps.ellipse_fill);
}

void CBImpl::EllipseFill::submit_eraser(Canvas& canvas)
{
	standard_submit_filled_eraser(canvas, canvas.binfo.interps.ellipse_fill);
}

void CBImpl::EllipseFill::reset_pencil(BrushInfo& binfo)
{
	CBImpl::EllipseOutline::reset_pencil(binfo);
}

void CBImpl::EllipseFill::reset_pen(BrushInfo& binfo)
{
	CBImpl::EllipseOutline::reset_pen(binfo);
}

void CBImpl::EllipseFill::reset_eraser(BrushInfo& binfo)
{
	CBImpl::EllipseOutline::reset_eraser(binfo);
}

// <<<==================================<<< BUCKET FILL >>>==================================>>>

static void bucket_fill_find_noncontiguous(Canvas& canvas, int x, int y, void(*color_pixel)(Canvas& canvas, int x, int y))
{
	const PixelRGBA color = canvas.pixel_color_at(x, y);
	IntBounds& bbox = canvas.binfo.brushing_bbox;
	for (int u = 0; u < canvas.image->buf.width; ++u)
	{
		for (int v = 0; v < canvas.image->buf.height; ++v)
		{
			if (canvas.battr.tolerance.tol(color.to_rgba(), canvas.pixel_color_at(u, v).to_rgba()))
			{
				color_pixel(canvas, u, v);
				update_bbox(bbox, u, v);
			}
		}
	}
}

static void bucket_fill_find_contiguous(Canvas& canvas, int x, int y, void(*color_pixel)(Canvas& canvas, int x, int y))
{
	// LATER use tolerance interval
	// TODO
}

static void bucket_fill_reset_bbox(Canvas& canvas)
{
	IntBounds& bbox = canvas.binfo.brushing_bbox;
	bbox.x1 = INT_MAX;
	bbox.x2 = INT_MIN;
	bbox.y1 = INT_MAX;
	bbox.y2 = INT_MIN;
}

void CBImpl::BucketFill::brush_pencil(Canvas& canvas, int x, int y)
{
	canvas.binfo.storage_2c.clear();
	bucket_fill_reset_bbox(canvas);
	auto color_pixel = [](Canvas& canvas, int x, int y) {
		PixelRGBA old_color = canvas.pixel_color_at(x, y);
		PixelRGBA new_color = canvas.applied_pencil_rgba();
		new_color.blend_over(old_color);
		canvas.binfo.storage_2c[{ x, y }] = { new_color, old_color };
		buffer_set_pixel_color(canvas.image->buf, x, y, new_color);
		};
	if (MainWindow->is_shift_pressed())
		bucket_fill_find_noncontiguous(canvas, x, y, color_pixel);
	else
		bucket_fill_find_contiguous(canvas, x, y, color_pixel);
	if (canvas.binfo.brushing_bbox.valid())
	{
		canvas.image->update_subtexture(bounds_to_rect(canvas.binfo.brushing_bbox));
		standard_push_pencil(canvas, canvas.binfo.brushing_bbox);
	}
	canvas.binfo.storage_2c.clear();
	bucket_fill_reset_bbox(canvas);
	canvas.cursor_enter();
}

void CBImpl::BucketFill::brush_pen(Canvas& canvas, int x, int y)
{
	canvas.binfo.storage_1c.clear();
	bucket_fill_reset_bbox(canvas);
	auto color_pixel = [](Canvas& canvas, int x, int y) {
		canvas.binfo.storage_1c[{ x, y }] = canvas.pixel_color_at(x, y);
		buffer_set_pixel_color(canvas.image->buf, x, y, canvas.applied_pen_rgba());
		};
	if (MainWindow->is_shift_pressed())
		bucket_fill_find_noncontiguous(canvas, x, y, color_pixel);
	else
		bucket_fill_find_contiguous(canvas, x, y, color_pixel);
	if (canvas.binfo.brushing_bbox.valid())
	{
		canvas.image->update_subtexture(bounds_to_rect(canvas.binfo.brushing_bbox));
		standard_push_pen(canvas, canvas.binfo.brushing_bbox);
	}
	canvas.binfo.storage_1c.clear();
	bucket_fill_reset_bbox(canvas);
	canvas.cursor_enter();
}

void CBImpl::BucketFill::brush_eraser(Canvas& canvas, int x, int y)
{
	canvas.binfo.storage_1c.clear();
	auto color_pixel = [](Canvas& canvas, int x, int y) {
		canvas.binfo.storage_1c[{ x, y }] = canvas.pixel_color_at(x, y);
		buffer_set_pixel_alpha(canvas.image->buf, x, y, 0);
		};
	if (MainWindow->is_shift_pressed())
		bucket_fill_find_noncontiguous(canvas, x, y, color_pixel);
	else
		bucket_fill_find_contiguous(canvas, x, y, color_pixel);
	if (canvas.binfo.brushing_bbox.valid())
	{
		canvas.image->update_subtexture(bounds_to_rect(canvas.binfo.brushing_bbox));
		standard_push_eraser(canvas, canvas.binfo.brushing_bbox);
	}
	canvas.binfo.storage_1c.clear();
	bucket_fill_reset_bbox(canvas);
	canvas.cursor_enter();
}

void CBImpl::BucketFill::brush_select(Canvas& canvas, int x, int y)
{
	// LATER
}
