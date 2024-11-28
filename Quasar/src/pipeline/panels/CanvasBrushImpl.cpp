#include "CanvasBrushImpl.h"

#include "Easel.h"
#include "user/Machine.h"

void CBImpl::Camera::brush(Canvas& canvas, int x, int y)
{
	// do nothing
}

void CBImpl::Paint::brush(Canvas& canvas, int x, int y)
{
	Buffer image_shifted_buf = canvas.image->buf;
	Canvas::Brush& binfo = canvas.binfo;
	image_shifted_buf.pixels += image_shifted_buf.byte_offset(x, y);
	if (x < binfo.brushing_bbox.x1)
		binfo.brushing_bbox.x1 = x;
	if (x > binfo.brushing_bbox.x2)
		binfo.brushing_bbox.x2 = x;
	if (y < binfo.brushing_bbox.y1)
		binfo.brushing_bbox.y1 = y;
	if (y > binfo.brushing_bbox.y2)
		binfo.brushing_bbox.y2 = y;
	switch (binfo.tip)
	{
	case BrushTip::PENCIL:
	{
		binfo.storage_mode = Canvas::Brush::PAINTED_COLORS;
		IPosition pos{ x, y };
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
		canvas.image->update_subtexture(x, y, 1, 1);
		auto iter = binfo.painted_colors.find(pos);
		if (iter == binfo.painted_colors.end())
			binfo.painted_colors[pos] = { initial_c, final_c };
		else
			iter->second.second = final_c;
		break;
	}
	case BrushTip::PEN:
	{
		binfo.storage_mode = Canvas::Brush::PAINTED_COLORS;
		IPosition pos{ x, y };
		PixelRGBA initial_c{ 0, 0, 0, 0 };
		PixelRGBA color = canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY ? canvas.pric_pen_pxs : canvas.altc_pen_pxs;
		for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
		{
			initial_c.at(i) = image_shifted_buf.pixels[i];
			image_shifted_buf.pixels[i] = color[i];
		}
		canvas.image->update_subtexture(x, y, 1, 1);
		auto iter = binfo.painted_colors.find(pos);
		if (iter == binfo.painted_colors.end())
			binfo.painted_colors[pos] = { initial_c, color };
		else
			iter->second.second = color;
		break;
	}
	case BrushTip::ERASER:
	{
		binfo.storage_mode = Canvas::Brush::PAINTED_COLORS;
		Position pos{ x, y };
		PixelRGBA initial_c{ 0, 0, 0, 0 };
		PixelRGBA final_c{ 0, 0, 0, 0 };
		for (CHPP i = 0; i < image_shifted_buf.chpp; ++i)
		{
			initial_c.at(i) = image_shifted_buf.pixels[i];
			image_shifted_buf.pixels[i] = 0;
		}
		canvas.image->update_subtexture(x, y, 1, 1);
		auto iter = binfo.painted_colors.find(pos);
		if (iter == binfo.painted_colors.end())
			binfo.painted_colors[pos] = { initial_c, final_c };
		else
			iter->second.second = final_c;
		break;
	}
	case BrushTip::SELECT:
		// LATER
		break;
	}
}

void CBImpl::Line::brush(Canvas& canvas, int x, int y)
{
	Buffer image_shifted_buf = canvas.image->buf;
	Canvas::Brush& binfo = canvas.binfo;
	switch (binfo.tip)
	{
	case BrushTip::PENCIL:
	{
		binfo.storage_mode = Canvas::Brush::PAINTED_COLORS;
		binfo.painted_colors.clear();
		auto& interp = binfo.interps.line;
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos.x, pos.y);
			buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
		}
		binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));

		interp.start = binfo.starting_pos;
		interp.finish = { x, y };
		interp.sync_with_endpoints();
		PixelRGBA new_color;
		PixelRGBA old_color;
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos);
			old_color = canvas.pixel_color_at(pos);
			new_color = canvas.applied_color().get_pixel_rgba();
			buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, new_color);
			new_color.blend_over(old_color);
			binfo.painted_colors[pos] = { new_color, old_color };
		}
		binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
		break;
	}
	case BrushTip::PEN:
	{
		binfo.storage_mode = Canvas::Brush::PREVIEW_POSITIONS;
		binfo.preview_positions.clear();
		auto& interp = binfo.interps.line;
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos.x, pos.y);
			buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
		}
		binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));

		interp.start = binfo.starting_pos;
		interp.finish = { x, y };
		interp.sync_with_endpoints();
		RGBA preview_color;
		PixelRGBA new_color;
		PixelRGBA old_color;
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos);
			old_color = canvas.pixel_color_at(pos);
			preview_color = canvas.applied_color();
			new_color = preview_color.get_pixel_rgba();
			buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y,
				RGBA(preview_color.rgb.r * preview_color.alpha, preview_color.rgb.g * preview_color.alpha, preview_color.rgb.b * preview_color.alpha, 1.0f).get_pixel_rgba());
			binfo.preview_positions[pos] = old_color;
		}
		binfo.preview_image->update_subtexture(abs_rect(interp.start, interp.finish));
		break;
	}
	case BrushTip::ERASER:
	{
		binfo.storage_mode = Canvas::Brush::PREVIEW_POSITIONS;
		binfo.preview_positions.clear();
		auto& interp = binfo.interps.line;
		IPosition pos{};
		for (unsigned int i = 0; i < interp.length; ++i)
		{
			interp.at(i, pos);
			// TODO here and for rect fill, use eraser_arr_w/h instead of 2
			buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, 2, 2);
		}
		binfo.eraser_preview_image->update_subtexture(abs_rect(interp.start, interp.finish, 2, 2));

		interp.start = binfo.starting_pos;
		interp.finish = { x, y };
		interp.sync_with_endpoints();
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
			binfo.preview_positions[pos] = old_color;
		}
		binfo.eraser_preview_image->update_subtexture(abs_rect(interp.start, interp.finish, 2, 2));
		break;
	}
	case BrushTip::SELECT:
		// LATER
		break;
	}
}

void CBImpl::Line::start(Canvas& canvas)
{
	canvas.binfo.show_preview = true;
}

void CBImpl::Line::submit(Canvas& canvas)
{
	// TODO
	//if (binfo.brush_pos != IPosition{ -1, -1 })
		//brush(binfo.brush_pos.x, binfo.brush_pos.y);
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image)
	{
		switch (binfo.tip)
		{
		case BrushTip::PENCIL:
			if (!binfo.painted_colors.empty())
				Machine.history.execute(std::make_shared<OneColorPencilAction>(canvas.image, binfo.starting_pos, binfo.brush_pos, std::move(binfo.painted_colors)));
			break;
		case BrushTip::PEN:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
					binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::ERASER:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 },
					binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::SELECT:
			// LATER
			break;
		}
	}
}

void CBImpl::RectFill::brush(Canvas& canvas, int x, int y)
{
	Canvas::Brush& binfo = canvas.binfo;
	if (binfo.tip != BrushTip::SELECT)
	{
		DiscreteRectFillInterpolator new_interp{ binfo.starting_pos, { x, y } };
		new_interp.sync_with_endpoints();
		auto& diff = binfo.interp_diffs.rect_fill;
		diff.first = binfo.interps.rect_fill;
		diff.second = new_interp;
		diff.sync_with_interpolators();
		IPosition pos{};
		// TODO rename painted_colors to 2-color storage, and preview_positions to 1-color storage.
		auto painted_colors_storage_remove = [&pc = binfo.painted_colors](IPosition pos) { pc.erase(pos); };
		auto preview_positions_storage_remove = [&pp = binfo.preview_positions](IPosition pos) { pp.erase(pos); };
		std::function<void(IPosition)> storage_remove;
		if (binfo.storage_mode == Canvas::Brush::PAINTED_COLORS)
			storage_remove = painted_colors_storage_remove;
		else if (binfo.storage_mode == Canvas::Brush::PREVIEW_POSITIONS)
			storage_remove = preview_positions_storage_remove;

		for (unsigned int i = 0; i < diff.remove_length; ++i)
		{
			diff.remove_at(i, pos.x, pos.y);
			if (binfo.tip & BrushTip::ERASER)
				buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, 2, 2);
			else
				buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
			storage_remove(pos);
		}

		auto painted_colors_storage_insert = [&pc = binfo.painted_colors](IPosition pos, PixelRGBA new_color, PixelRGBA old_color) {
			new_color.blend_over(old_color);
			pc[pos] = { new_color, old_color };
			};
		auto preview_positions_storage_insert = [&pp = binfo.preview_positions](IPosition pos, PixelRGBA new_color, PixelRGBA old_color) {
			pp[pos] = old_color;
			};
		std::function<void(IPosition, PixelRGBA, PixelRGBA)> storage_insert;
		if (binfo.storage_mode == Canvas::Brush::PAINTED_COLORS)
			storage_insert = painted_colors_storage_insert;
		else if (binfo.storage_mode == Canvas::Brush::PREVIEW_POSITIONS)
			storage_insert = preview_positions_storage_insert;

		RGBA preview_color;
		PixelRGBA old_color;
		PixelRGBA new_color;
		for (unsigned int i = 0; i < diff.insert_length; ++i)
		{
			diff.insert_at(i, pos.x, pos.y);
			old_color = canvas.pixel_color_at(pos.x, pos.y);
			preview_color = canvas.applied_color();
			new_color = preview_color.get_pixel_rgba();
			if (binfo.tip & BrushTip::ERASER)
				buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, 2, 2);
			else if (binfo.tip & BrushTip::PENCIL)
				buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y, preview_color.get_pixel_rgba());
			else if (binfo.tip & BrushTip::PEN)
				buffer_set_pixel_color(binfo.preview_image->buf, pos.x, pos.y,
					RGBA(preview_color.rgb.r * preview_color.alpha, preview_color.rgb.g * preview_color.alpha, preview_color.rgb.b * preview_color.alpha, 1.0f).get_pixel_rgba());

			storage_insert(pos, new_color, old_color);
		}
		if (binfo.tip & BrushTip::ERASER)
		{
			binfo.eraser_preview_image->update_subtexture(diff.first_bbox.x1 * 2, diff.first_bbox.y1 * 2, diff.first_bbox.width_no_abs() * 2, diff.first_bbox.height_no_abs() * 2);
			binfo.eraser_preview_image->update_subtexture(diff.second_bbox.x1 * 2, diff.second_bbox.y1 * 2, diff.second_bbox.width_no_abs() * 2, diff.second_bbox.height_no_abs() * 2);
		}
		else
		{
			binfo.preview_image->update_subtexture(diff.first_bbox.x1, diff.first_bbox.y1, diff.first_bbox.width_no_abs(), diff.first_bbox.height_no_abs());
			binfo.preview_image->update_subtexture(diff.second_bbox.x1, diff.second_bbox.y1, diff.second_bbox.width_no_abs(), diff.second_bbox.height_no_abs());
		}
		binfo.interps.rect_fill = new_interp;
		binfo.interps.rect_fill.sync_with_endpoints();
	}
	else
	{
		// LATER
	}
}

void CBImpl::RectFill::start(Canvas& canvas)
{
	Canvas::Brush& binfo = canvas.binfo;
	binfo.show_preview = true;
	binfo.interps.rect_fill.start = binfo.interps.rect_fill.finish = binfo.starting_pos;
	binfo.interps.rect_fill.sync_with_endpoints();
	if (binfo.starting_pos != IPosition{ -1, -1 }) // TODO if starting brush, would this really ever fail?
	{
		PixelRGBA old_color = canvas.pixel_color_at(binfo.starting_pos.x, binfo.starting_pos.y);
		PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
		if (binfo.tip & BrushTip::PENCIL)
		{
			buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, new_color);
			binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
		}
		else if (binfo.tip & BrushTip::PEN)
		{
			RGBA preview_color = new_color.to_rgba();
			buffer_set_pixel_color(binfo.preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y,
				RGBA(preview_color.rgb.r * preview_color.alpha, preview_color.rgb.g * preview_color.alpha, preview_color.rgb.b * preview_color.alpha, 1.0f).get_pixel_rgba());
			binfo.preview_image->update_subtexture(binfo.starting_pos.x, binfo.starting_pos.y, 1, 1);
		}
		else if (binfo.tip & BrushTip::ERASER)
		{
			buffer_set_rect_alpha(binfo.eraser_preview_image->buf, binfo.starting_pos.x, binfo.starting_pos.y, 1, 1, 255, 2, 2);
			binfo.eraser_preview_image->update_subtexture(binfo.starting_pos.x * 2, binfo.starting_pos.y * 2, 2, 2);
		}
		if (binfo.tip & BrushTip::PENCIL)
		{
			new_color.blend_over(old_color);
			binfo.storage_mode = Canvas::Brush::PAINTED_COLORS;
		}
		else if (binfo.tip & (BrushTip::PEN | BrushTip::ERASER))
			binfo.storage_mode = Canvas::Brush::PREVIEW_POSITIONS;
		if (binfo.storage_mode == Canvas::Brush::PAINTED_COLORS)
			binfo.painted_colors[binfo.starting_pos] = { new_color, old_color };
		else if (binfo.storage_mode == Canvas::Brush::PREVIEW_POSITIONS)
			binfo.preview_positions[binfo.starting_pos] = new_color;
	}
}

void CBImpl::RectFill::submit(Canvas& canvas)
{
	// TODO this comment lags:
	//if (binfo.brush_pos != IPosition{ -1, -1 })
		//hover_pixel_under_cursor(Machine.easel_cursor_world_pos());
	Canvas::Brush& binfo = canvas.binfo;
	if (canvas.image)
	{
		switch (binfo.tip)
		{
		case BrushTip::PENCIL:
			if (!binfo.painted_colors.empty())
				Machine.history.execute(std::make_shared<OneColorPencilAction>(canvas.image, binfo.starting_pos, binfo.brush_pos, std::move(binfo.painted_colors)));
			break;
		case BrushTip::PEN:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
					binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::ERASER:
			if (!binfo.preview_positions.empty())
				Machine.history.execute(std::make_shared<OneColorPenAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 },
					binfo.starting_pos, binfo.brush_pos, std::move(binfo.preview_positions)));
			break;
		case BrushTip::SELECT:
			// LATER
			break;
		}
	}
}
