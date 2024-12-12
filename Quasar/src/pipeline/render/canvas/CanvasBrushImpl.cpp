#include "CanvasBrushImpl.h"

#include <stack>

#include "Canvas.h"
#include "SelectionMants.h"
#include "../../panels/Easel.h"
#include "user/Machine.h"
#include "../FlatSprite.h"

static void _standard_outline_brush_pencil_looperand_update_storage(Canvas& canvas, IPosition pos)
{
	PixelRGBA old_color = canvas.image->buf.pixel_color_at(pos.x, pos.y);
	PixelRGBA new_color = canvas.applied_color().get_pixel_rgba();
	buffer_set_pixel_color(canvas.binfo.preview_image->buf, pos.x, pos.y, new_color);
	new_color.blend_over(old_color);
	canvas.binfo.storage_2c[pos] = { new_color, old_color };
}

static void _standard_outline_brush_pencil_looperand_dont_update_storage(Canvas& canvas, IPosition pos)
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
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	update_subtexture(binfo);

	interp.finish = binfo.last_brush_pos;
	interp.sync_with_endpoints();
	auto looperand = update_storage ? &_standard_outline_brush_pencil_looperand_update_storage : &_standard_outline_brush_pencil_looperand_dont_update_storage;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			looperand(canvas, pos);
	}
	update_subtexture(binfo);
}

static void _standard_outline_brush_pen_looperand_dont_update_storage(Canvas& canvas, IPosition pos)
{
	buffer_set_pixel_color(canvas.binfo.preview_image->buf, pos.x, pos.y, canvas.applied_color().no_alpha_equivalent().get_pixel_rgba());
}

static void _standard_outline_brush_pen_looperand_update_storage(Canvas& canvas, IPosition pos)
{
	_standard_outline_brush_pen_looperand_dont_update_storage(canvas, pos);
	canvas.binfo.storage_1c[pos] = canvas.image->buf.pixel_color_at(pos.x, pos.y);
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
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			buffer_set_pixel_alpha(binfo.preview_image->buf, pos.x, pos.y, 0);
	}
	update_subtexture(binfo);

	interp.finish = binfo.last_brush_pos;
	interp.sync_with_endpoints();
	auto looperand = update_storage ? &_standard_outline_brush_pen_looperand_update_storage : &_standard_outline_brush_pen_looperand_dont_update_storage;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			looperand(canvas, pos);
	}
	update_subtexture(binfo);
}

static void _standard_outline_brush_eraser_looperand_dont_update_storage(Canvas& canvas, IPosition pos)
{
	buffer_set_rect_alpha(canvas.binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 255, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
}

static void _standard_outline_brush_eraser_looperand_update_storage(Canvas& canvas, IPosition pos)
{
	_standard_outline_brush_eraser_looperand_dont_update_storage(canvas, pos);
	canvas.binfo.storage_1c[pos] = canvas.image->buf.pixel_color_at(pos.x, pos.y);
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
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			buffer_set_rect_alpha(binfo.eraser_preview_image->buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
	}
	update_subtexture(binfo);

	interp.finish = binfo.last_brush_pos;
	interp.sync_with_endpoints();
	auto looperand = update_storage ? &_standard_outline_brush_eraser_looperand_update_storage : &_standard_outline_brush_eraser_looperand_dont_update_storage;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			looperand(canvas, pos);
	}
	update_subtexture(binfo);
}

static void standard_outline_brush_select(Canvas& canvas, int x, int y, DiscreteInterpolator& interp)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.last_brush_pos = { x, y };
	binfo.smants_preview->send_buffer(binfo.smants_preview->clear());

	interp.finish = binfo.last_brush_pos;
	interp.sync_with_endpoints();
	IntBounds bbox = IntBounds::NADIR;
	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		if (binfo.smants_preview->add(pos))
			update_bbox(bbox, pos.x, pos.y);
	}
	binfo.smants_preview->send_buffer(bbox);
}

static void standard_start(Canvas& canvas, DiscreteInterpolator& interp)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.show_brush_preview = true;
	interp.start = interp.finish = binfo.starting_pos;
	interp.sync_with_endpoints();
}

static void standard_start_select_check_replace(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY && !MainWindow->is_ctrl_pressed() && !binfo.smants->get_points().empty())
	{
		IntBounds bbox = binfo.clear_selection();
		if (bbox != IntBounds::NADIR)
		{
			update_bbox(binfo.brushing_bbox, bbox);
			binfo.smants->send_buffer(bbox);
		}
	}
}

static void standard_start_select(Canvas& canvas, DiscreteInterpolator& interp)
{
	canvas.binfo.show_selection_preview = true;
	interp.start = interp.finish = canvas.binfo.starting_pos;
	interp.sync_with_endpoints();
	standard_start_select_check_replace(canvas);
}

static void standard_submit_pencil(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_2c.empty())
		Machine.history.execute(std::make_shared<TwoColorAction>(canvas.image, abs_bounds(binfo.starting_pos, binfo.last_brush_pos), std::move(binfo.storage_2c)));
}

static void standard_push_pencil(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_2c.empty())
		Machine.history.push(std::make_shared<TwoColorAction>(canvas.image, binfo.brushing_bbox, std::move(binfo.storage_2c)));
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
			if (binfo.point_valid_in_selection(pos.x, pos.y))
			{
				PixelRGBA old_color = canvas.image->buf.pixel_color_at(pos.x, pos.y);
				PixelRGBA new_color = blend_over(canvas.applied_color().get_pixel_rgba(), old_color);
				canvas.binfo.storage_2c[pos] = { new_color, old_color };
			}
		}
		standard_submit_pencil(canvas);
	}
}

static void standard_submit_pen(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorAction>(canvas.image, canvas.applied_color().get_pixel_rgba(),
			abs_bounds(binfo.starting_pos, binfo.last_brush_pos), std::move(binfo.storage_1c)));
}

static void standard_push_pen(Canvas& canvas, PixelRGBA color)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.push(std::make_shared<OneColorAction>(canvas.image, color, binfo.brushing_bbox, std::move(binfo.storage_1c)));
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
			if (binfo.point_valid_in_selection(pos.x, pos.y))
				canvas.binfo.storage_1c[pos] = canvas.image->buf.pixel_color_at(pos.x, pos.y);
		}
		standard_submit_pen(canvas);
	}
}

static void standard_submit_eraser(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.execute(std::make_shared<OneColorAction>(canvas.image, PixelRGBA{},
			abs_bounds(binfo.starting_pos, binfo.last_brush_pos), std::move(binfo.storage_1c)));
}

static void standard_push_eraser(Canvas& canvas)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.image && !binfo.storage_1c.empty())
		Machine.history.push(std::make_shared<OneColorAction>(canvas.image, PixelRGBA{}, binfo.brushing_bbox, std::move(binfo.storage_1c)));
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
			if (binfo.point_valid_in_selection(pos.x, pos.y))
				canvas.binfo.storage_1c[pos] = canvas.image->buf.pixel_color_at(pos.x, pos.y);
		}
		standard_submit_eraser(canvas);
	}
}

static void _standard_submit_select_body(BrushInfo& binfo, DiscreteInterpolator& interp, bool(*add_remove_func)(BrushInfo&, IPosition))
{
	IPosition pos{};
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		if (add_remove_func(binfo, pos))
			update_bbox(binfo.brushing_bbox, pos.x, pos.y);
	}
	binfo.smants->send_buffer(binfo.brushing_bbox);
}

static void standard_submit_select(Canvas& canvas, DiscreteInterpolator& interp)
{
	static auto add = [](BrushInfo& binfo, IPosition pos) { return binfo.add_to_selection(pos); };
	static auto remove = [](BrushInfo& binfo, IPosition pos) { return binfo.remove_from_selection(pos); };
	if (canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY)
		_standard_submit_select_body(canvas.binfo, interp, add);
	else if (canvas.cursor_state == Canvas::CursorState::DOWN_ALTERNATE)
		_standard_submit_select_body(canvas.binfo, interp, remove);
	canvas.binfo.push_selection_to_history();
}

static void standard_reset_outline_select(BrushInfo& binfo)
{
	if (binfo.cancelling)
	{
		IntBounds bbox = IntBounds::NADIR;
		for (IPosition pos : binfo.cleared_selection)
			if (binfo.add_to_selection(pos))
				update_bbox(bbox, pos.x, pos.y);
		binfo.smants->send_buffer(bbox);
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

void CBImpl::fill_selection_pencil(Canvas& canvas, PixelRGBA color)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.brushing_bbox = IntBounds::NADIR;
	for (IPosition pos : canvas.binfo.smants->get_points())
	{
		PixelRGBA old_color = canvas.image->buf.pixel_color_at(pos.x, pos.y);
		PixelRGBA new_color = blend_over(color, old_color);
		binfo.storage_2c[pos] = { new_color, old_color };
		buffer_set_pixel_color(canvas.image->buf, pos.x, pos.y, new_color);
		update_bbox(binfo.brushing_bbox, pos.x, pos.y);
	}
	canvas.image->update_subtexture(binfo.brushing_bbox);
	standard_push_pencil(canvas);
}

void CBImpl::fill_selection_pen(Canvas& canvas, PixelRGBA color)
{
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	for (IPosition pos : canvas.binfo.smants->get_points())
	{
		canvas.binfo.storage_1c[pos] = canvas.image->buf.pixel_color_at(pos.x, pos.y);
		buffer_set_pixel_color(canvas.image->buf, pos.x, pos.y, color);
		update_bbox(canvas.binfo.brushing_bbox, pos.x, pos.y);
	}
	canvas.image->update_subtexture(canvas.binfo.brushing_bbox);
	standard_push_pen(canvas, color);
}

void CBImpl::fill_selection_eraser(Canvas& canvas)
{
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	for (IPosition pos : canvas.binfo.smants->get_points())
	{
		canvas.binfo.storage_1c[pos] = canvas.image->buf.pixel_color_at(pos.x, pos.y);
		buffer_set_pixel_alpha(canvas.image->buf, pos.x, pos.y, 0);
		update_bbox(canvas.binfo.brushing_bbox, pos.x, pos.y);
	}
	canvas.image->update_subtexture(canvas.binfo.brushing_bbox);
	standard_push_eraser(canvas);
}

struct UpdateSelectionMoveAction : public ActionBase
{
	IPosition delta;
	UpdateSelectionMoveAction(IPosition delta) : delta(delta) { weight = sizeof(UpdateSelectionMoveAction); }
	virtual void forward() override
	{
		BrushInfo& binfo = MEasel->canvas().binfo;
		binfo.move_selpxs_offset += delta;
	}
	virtual void backward() override
	{
		BrushInfo& binfo = MEasel->canvas().binfo;
		binfo.move_selpxs_offset -= delta;
	}
	QUASAR_ACTION_EQUALS_OVERRIDE(UpdateSelectionMoveAction)
};

static void move_selection_with_pixels_initial_common_pencil_pen(Canvas& canvas, int dx, int dy, bool pencil)
{
	BrushInfo& binfo = canvas.binfo;
	binfo.state = BrushInfo::State::MOVING_SUBIMG;
	auto& selbuf = binfo.selection_subimage->buf;
	auto& points = binfo.smants->get_points();
	std::unordered_set<IPosition> premove_points;
	IntBounds remove_bbox = IntBounds::NADIR;

	static auto pencil_color_conv = [](PixelRGBA color) { return color; };
	static auto pen_color_conv = [](PixelRGBA color) { return color.no_alpha_equivalent(); };
	auto color_conv = pencil ? pencil_color_conv : pen_color_conv;

	while (!points.empty())
	{
		IPosition from = *points.begin();
		if (binfo.remove_from_selection(from))
			update_bbox(remove_bbox, from.x, from.y);
		PixelRGBA color = canvas.image->buf.pixel_color_at(from.x, from.y);
		binfo.raw_selection_pixels[from] = color;
		PixelRGBA sel_color = color_conv(color);
		binfo.storage_selection_1c[from] = sel_color;
		buffer_set_pixel_color(selbuf, from.x, from.y, sel_color);
		binfo.storage_1c[from] = color;
		buffer_set_pixel_alpha(canvas.image->buf, from.x, from.y, 0);
		premove_points.insert(from);
	}
	binfo.selection_subimage->update_subtexture(remove_bbox);
	canvas.image->update_subtexture(remove_bbox);
	IntBounds add_bbox = IntBounds::NADIR;
	for (auto iter = premove_points.begin(); iter != premove_points.end(); ++iter)
	{
		if (binfo.add_to_selection(*iter + IPosition{ dx, dy }))
			update_bbox(add_bbox, iter->x + dx, iter->y + dy);
	}
	binfo.sel_subimg_sprite->self.transform.position += Position{ dx, dy };
	binfo.sel_subimg_sprite->update_transform().ur->send_buffer();
	// LATER if remove_bbox and add_bbox overlap, don't send that overlapped section. send only their union
	if (!intersection(remove_bbox, canvas.image->buf.bbox(), remove_bbox))
		remove_bbox = IntBounds::NADIR;
	if (!intersection(add_bbox, canvas.image->buf.bbox(), add_bbox))
		add_bbox = IntBounds::NADIR;
	binfo.smants->send_buffer(remove_bbox);
	binfo.smants->send_buffer(add_bbox);

	binfo.move_selpxs_offset = { dx, dy };
	Machine.history.push(std::make_shared<CompositeAction>(std::vector<std::shared_ptr<ActionBase>>{
		std::make_shared<SmantsMoveAction>(IPosition{ dx, dy }, std::move(premove_points)),
		std::make_shared<InverseAction>(std::make_shared<OneColorAction>(binfo.selection_subimage, PixelRGBA{ 0, 0, 0, 0 }, remove_bbox, std::move(binfo.storage_selection_1c))),
		std::make_shared<OneColorAction>(canvas.image, PixelRGBA{ 0, 0, 0, 0 }, remove_bbox, std::move(binfo.storage_1c)),
		std::make_shared<SpriteMoveAction>(binfo.sel_subimg_sprite, IPosition{ dx, dy }),
		std::make_shared<UpdateSelectionMoveAction>(IPosition{ dx, dy }),
	}));
}

static void move_selection_with_pixels(Canvas& canvas, int dx, int dy)
{
	BrushInfo& binfo = canvas.binfo;
	auto& points = binfo.smants->get_points();
	std::unordered_set<IPosition> premove_points;
	IntBounds remove_bbox = IntBounds::NADIR;
	while (!points.empty())
	{
		IPosition from = *points.begin();
		if (binfo.remove_from_selection(from))
			update_bbox(remove_bbox, from.x, from.y);
		premove_points.insert(from);
	}
	IntBounds add_bbox = IntBounds::NADIR;
	for (auto iter = premove_points.begin(); iter != premove_points.end(); ++iter)
	{
		if (binfo.add_to_selection(*iter + IPosition{ dx, dy }))
			update_bbox(add_bbox, iter->x + dx, iter->y + dy);
	}
	binfo.sel_subimg_sprite->self.transform.position += Position{ dx, dy };
	binfo.sel_subimg_sprite->update_transform().ur->send_buffer();
	// LATER if remove_bbox and add_bbox overlap, don't send that overlapped section. send only their union
	if (!intersection(remove_bbox, canvas.image->buf.bbox(), remove_bbox))
		remove_bbox = IntBounds::NADIR;
	if (!intersection(add_bbox, canvas.image->buf.bbox(), add_bbox))
		add_bbox = IntBounds::NADIR;
	binfo.smants->send_buffer(remove_bbox);
	binfo.smants->send_buffer(add_bbox);
	binfo.move_selpxs_offset += IPosition{ dx, dy };
	Machine.history.push(std::make_shared<CompositeAction>(std::vector<std::shared_ptr<ActionBase>>{
		std::make_shared<SmantsMoveAction>(IPosition{ dx, dy }, std::move(premove_points)),
		std::make_shared<SpriteMoveAction>(binfo.sel_subimg_sprite, IPosition{ dx, dy }),
		std::make_shared<UpdateSelectionMoveAction>(IPosition{ dx, dy }),
	}));
}

void CBImpl::move_selection_with_pixels_pencil(Canvas& canvas, int dx, int dy)
{
	if (canvas.binfo.state == BrushInfo::State::MOVING_SUBIMG)
		move_selection_with_pixels(canvas, dx, dy);
	else
		move_selection_with_pixels_initial_common_pencil_pen(canvas, dx, dy, true);
}

void CBImpl::move_selection_with_pixels_pen(Canvas& canvas, int dx, int dy)
{
	if (canvas.binfo.state == BrushInfo::State::MOVING_SUBIMG)
		move_selection_with_pixels(canvas, dx, dy);
	else
		move_selection_with_pixels_initial_common_pencil_pen(canvas, dx, dy, false);
}

void CBImpl::move_selection_without_pixels(Canvas& canvas, int dx, int dy)
{
	BrushInfo& binfo = canvas.binfo;
	auto& points = binfo.smants->get_points();
	std::unordered_set<IPosition> premove_points;
	IntBounds remove_bbox = IntBounds::NADIR;
	while (!points.empty())
	{
		IPosition from = *points.begin();
		if (binfo.remove_from_selection(from))
			update_bbox(remove_bbox, from.x, from.y);
		premove_points.insert(from);
	}
	IntBounds add_bbox = IntBounds::NADIR;
	for (auto iter = premove_points.begin(); iter != premove_points.end(); ++iter)
	{
		if (binfo.add_to_selection(*iter + IPosition{ dx, dy }))
			update_bbox(add_bbox, iter->x + dx, iter->y + dy);
	}
	// LATER if remove_bbox and add_bbox overlap, don't send that overlapped section. send only their union
	if (!intersection(remove_bbox, canvas.image->buf.bbox(), remove_bbox))
		remove_bbox = IntBounds::NADIR;
	if (!intersection(add_bbox, canvas.image->buf.bbox(), add_bbox))
		add_bbox = IntBounds::NADIR;
	binfo.smants->send_buffer(remove_bbox);
	binfo.smants->send_buffer(add_bbox);
	Machine.history.push(std::make_shared<SmantsMoveAction>(IPosition{ dx, dy }, std::move(premove_points)));
}

void CBImpl::transition_selection_tip(Canvas& canvas, BrushTip from, BrushTip to)
{
	if (from & (BrushTip::PENCIL | BrushTip::PEN))
	{
		if (to & (BrushTip::ERASER | BrushTip::SELECT))
			canvas.apply_selection();
		else if (from & BrushTip::PENCIL && to & BrushTip::PEN)
		{
			IntBounds bbox = IntBounds::NADIR;
			auto& selbuf = canvas.binfo.selection_subimage->buf;
			for (auto iter = canvas.binfo.raw_selection_pixels.begin(); iter != canvas.binfo.raw_selection_pixels.end(); ++iter)
			{
				buffer_set_pixel_color(selbuf, iter->first.x, iter->first.y, iter->second.no_alpha_equivalent());
				update_bbox(bbox, iter->first.x, iter->first.y);
			}
			canvas.binfo.selection_subimage->update_subtexture(bbox);
		}
		else if (from & BrushTip::PEN && to & BrushTip::PENCIL)
		{
			IntBounds bbox = IntBounds::NADIR;
			auto& selbuf = canvas.binfo.selection_subimage->buf;
			for (auto iter = canvas.binfo.raw_selection_pixels.begin(); iter != canvas.binfo.raw_selection_pixels.end(); ++iter)
			{
				buffer_set_pixel_color(selbuf, iter->first.x, iter->first.y, iter->second);
				update_bbox(bbox, iter->first.x, iter->first.y);
			}
			canvas.binfo.selection_subimage->update_subtexture(bbox);
		}
	}
}

struct ApplySelectionAction : public ActionBase
{
	bool initially_pen;
	IPosition sprite_pos = {};
	IPosition move_offset = {};
	struct PixelRecord
	{
		PixelRGBA applied_on, subimg, result;
	};

	std::unordered_map<IPosition, PixelRecord> applied_pixels;

	ApplySelectionAction(bool pen)
		: initially_pen(pen)
	{
		weight = sizeof(ApplySelectionAction);
		Canvas& canvas = MEasel->canvas();
		BrushInfo& binfo = canvas.binfo;
		sprite_pos = binfo.sel_subimg_sprite->self.transform.position;
		move_offset = binfo.move_selpxs_offset;

		static const auto set_pixel_record_pen = [](PixelRGBA image_color, PixelRGBA sel_color) { return PixelRecord{ image_color, sel_color.no_alpha_equivalent(), sel_color }; };
		static const auto set_pixel_record_pencil = [](PixelRGBA image_color, PixelRGBA sel_color) { return PixelRecord{ image_color, sel_color, blend_over(sel_color, image_color) }; };
		auto set_pixel_record = pen ? set_pixel_record_pen : set_pixel_record_pencil;

		while (!binfo.raw_selection_pixels.empty())
		{
			auto iter = binfo.raw_selection_pixels.begin();
			IPosition pos = iter->first + move_offset;
			applied_pixels[pos] = set_pixel_record(canvas.image->buf.pixel_color_at(pos.x, pos.y), iter->second);
			binfo.raw_selection_pixels.erase(iter);
		}
	}
	virtual void forward() override
	{
		Canvas& canvas = MEasel->canvas();
		BrushInfo& binfo = canvas.binfo;

		binfo.state = BrushInfo::State::NEUTRAL;
		binfo.move_selpxs_offset = {};

		Buffer& img_buf = canvas.image->buf;
		IntBounds img_bbox = IntBounds::NADIR;
		Buffer& subimg_buf = binfo.selection_subimage->buf;
		IntBounds subimg_bbox = IntBounds::NADIR;
		for (auto iter = applied_pixels.begin(); iter != applied_pixels.end(); ++iter)
		{
			if (in_diagonal_rect(iter->first, {}, { img_buf.width, img_buf.height }))
			{
				buffer_set_pixel_color(img_buf, iter->first.x, iter->first.y, iter->second.result);
				update_bbox(img_bbox, iter->first.x, iter->first.y);
			}
			IPosition subimg_pos = iter->first - move_offset;
			if (in_diagonal_rect(subimg_pos, {}, { img_buf.width, img_buf.height }))
			{
				buffer_set_pixel_alpha(subimg_buf, subimg_pos.x, subimg_pos.y, 0);
				update_bbox(subimg_bbox, subimg_pos.x, subimg_pos.y);
			}
		}
		canvas.image->update_subtexture(img_bbox);
		binfo.selection_subimage->update_subtexture(subimg_bbox);
		binfo.sel_subimg_sprite->self.transform.position = {};
		binfo.sel_subimg_sprite->update_transform().ur->send_buffer();
		binfo.smants->send_buffer(binfo.clear_selection());
		binfo.raw_selection_pixels.clear();
	}
	virtual void backward() override
	{
		Canvas& canvas = MEasel->canvas();
		BrushInfo& binfo = canvas.binfo;

		binfo.state = BrushInfo::State::MOVING_SUBIMG;
		binfo.move_selpxs_offset = move_offset;

		IntBounds sel_bbox = IntBounds::NADIR;
		Buffer& img_buf = canvas.image->buf;
		IntBounds img_bbox = IntBounds::NADIR;
		Buffer& subimg_buf = binfo.selection_subimage->buf;
		IntBounds subimg_bbox = IntBounds::NADIR;

		static const auto raw_sel_pixel_pen = [](PixelRecord pxr) { return pxr.result; };
		static const auto raw_sel_pixel_pencil = [](PixelRecord pxr) { return pxr.subimg; };
		auto raw_sel_pixel = initially_pen ? raw_sel_pixel_pen : raw_sel_pixel_pencil;

		for (auto iter = applied_pixels.begin(); iter != applied_pixels.end(); ++iter)
		{
			if (in_diagonal_rect(iter->first, {}, { img_buf.width, img_buf.height }))
			{
				buffer_set_pixel_color(img_buf, iter->first.x, iter->first.y, iter->second.applied_on);
				update_bbox(img_bbox, iter->first.x, iter->first.y);
			}
			IPosition subimg_pos = iter->first - move_offset;
			if (in_diagonal_rect(subimg_pos, {}, { img_buf.width, img_buf.height }))
			{
				buffer_set_pixel_alpha(subimg_buf, subimg_pos.x, subimg_pos.y, iter->second.subimg.a);
				update_bbox(subimg_bbox, subimg_pos.x, subimg_pos.y);
			}
			if (binfo.add_to_selection(iter->first))
				update_bbox(sel_bbox, iter->first.x, iter->first.y);
			binfo.raw_selection_pixels[iter->first - move_offset] = raw_sel_pixel(iter->second);
		}
		canvas.image->update_subtexture(img_bbox);
		binfo.selection_subimage->update_subtexture(subimg_bbox);
		binfo.sel_subimg_sprite->self.transform.position = sprite_pos;
		binfo.sel_subimg_sprite->update_transform().ur->send_buffer();
		binfo.smants->send_buffer(sel_bbox);
	}
};

void CBImpl::apply_selection_pencil(Canvas& canvas)
{
	Machine.history.execute(std::make_shared<ApplySelectionAction>(false));
}

void CBImpl::apply_selection_pen(Canvas& canvas)
{
	Machine.history.execute(std::make_shared<ApplySelectionAction>(true));
}

// <<<==================================<<< CAMERA >>>==================================>>>

void CBImpl::Camera::brush(Canvas& canvas, int x, int y)
{
	// do nothing
}

// <<<==================================<<< PAINT >>>==================================>>>

static void paint_brush_suffix(Canvas& canvas, int x, int y, PixelRGBA initial_c, PixelRGBA final_c)
{
	canvas.image->update_subtexture(x, y, 1, 1);
	auto iter = canvas.binfo.storage_2c.find({ x, y });
	if (iter == canvas.binfo.storage_2c.end())
		canvas.binfo.storage_2c[{ x, y }] = { final_c, initial_c };
	else
		iter->second.first = final_c;
}

void CBImpl::Paint::brush_pencil(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	if (!binfo.point_valid_in_selection(x, y))
		return;
	update_bbox(binfo.brushing_bbox, x, y);
	PixelRGBA initial_c = canvas.image->buf.pixel_color_at(x, y);
	PixelRGBA final_c = blend_over(canvas.applied_pencil_rgba(), initial_c);
	buffer_set_pixel_color(canvas.image->buf, x, y, final_c);
	paint_brush_suffix(canvas, x, y, initial_c, final_c);
}

void CBImpl::Paint::brush_pen(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	if (!binfo.point_valid_in_selection(x, y))
		return;
	update_bbox(binfo.brushing_bbox, x, y);
	PixelRGBA initial_c = canvas.image->buf.pixel_color_at(x, y);
	PixelRGBA final_c = canvas.applied_pen_rgba();
	buffer_set_pixel_color(canvas.image->buf, x, y, final_c);
	paint_brush_suffix(canvas, x, y, initial_c, final_c);
}

void CBImpl::Paint::brush_eraser(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	if (!binfo.point_valid_in_selection(x, y))
		return;
	update_bbox(binfo.brushing_bbox, x, y);
	PixelRGBA initial_c = canvas.image->buf.pixel_color_at(x, y);
	buffer_set_pixel_alpha(canvas.image->buf, x, y, 0);
	canvas.image->update_subtexture(x, y, 1, 1);
	if (canvas.binfo.storage_1c.find({ x, y }) == canvas.binfo.storage_1c.end())
		canvas.binfo.storage_1c[{ x, y }] = initial_c;
}

void CBImpl::Paint::brush_select(Canvas& canvas, int x, int y)
{
	BrushInfo& binfo = canvas.binfo;
	if (canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY)
	{
		if (binfo.add_to_selection({ x, y }))
		{
			update_bbox(binfo.brushing_bbox, x, y);
			binfo.smants->send_buffer(binfo.brushing_bbox);
		}
	}
	else if (canvas.cursor_state == Canvas::CursorState::DOWN_ALTERNATE)
	{
		if (binfo.remove_from_selection({ x, y }))
		{
			update_bbox(binfo.brushing_bbox, x, y);
			binfo.smants->send_buffer(binfo.brushing_bbox);
		}
	}
}

void CBImpl::Paint::start_select(Canvas& canvas)
{
	standard_start_select_check_replace(canvas);
}

void CBImpl::Paint::submit_pencil(Canvas& canvas)
{
	if (canvas.image && !canvas.binfo.storage_2c.empty())
		Machine.history.push(std::make_shared<TwoColorAction>(canvas.image, canvas.binfo.brushing_bbox, std::move(canvas.binfo.storage_2c)));
}

void CBImpl::Paint::submit_pen(Canvas& canvas)
{
	if (canvas.image && !canvas.binfo.storage_2c.empty())
		Machine.history.push(std::make_shared<TwoColorAction>(canvas.image, canvas.binfo.brushing_bbox, std::move(canvas.binfo.storage_2c)));
}

void CBImpl::Paint::submit_eraser(Canvas& canvas)
{
	if (canvas.image && !canvas.binfo.storage_1c.empty())
		Machine.history.push(std::make_shared<OneColorAction>(canvas.image, PixelRGBA{}, canvas.binfo.brushing_bbox, std::move(canvas.binfo.storage_1c)));
}

void CBImpl::Paint::submit_select(Canvas& canvas)
{
	canvas.binfo.push_selection_to_history();
}

void CBImpl::Paint::reset_pencil(BrushInfo& binfo)
{
	TwoColorAction(binfo.canvas->image, binfo.brushing_bbox, std::move(binfo.storage_2c)).backward();
}

void CBImpl::Paint::reset_pen(BrushInfo& binfo)
{
	TwoColorAction(binfo.canvas->image, binfo.brushing_bbox, std::move(binfo.storage_2c)).backward();
}

void CBImpl::Paint::reset_eraser(BrushInfo& binfo)
{
	OneColorAction(binfo.canvas->image, PixelRGBA{}, binfo.brushing_bbox, std::move(binfo.storage_1c)).backward();
}

void CBImpl::Paint::reset_select(BrushInfo& binfo)
{
	SmantsModifyAction(binfo.smants, binfo.brushing_bbox, std::move(binfo.storage_select_remove), std::move(binfo.storage_select_add)).backward();
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
	standard_outline_brush_select(canvas, x, y, canvas.binfo.interps.line);
}

void CBImpl::Line::start(Canvas& canvas)
{
	standard_start(canvas, canvas.binfo.interps.line);
}

void CBImpl::Line::start_select(Canvas& canvas)
{
	standard_start_select(canvas, canvas.binfo.interps.line);
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

void CBImpl::Line::submit_select(Canvas& canvas)
{
	standard_submit_select(canvas, canvas.binfo.interps.line);
}

static void line_reset(BrushInfo& binfo, Buffer& buf, void(*buffer_remove)(Buffer& buf, IPosition pos))
{
	DiscreteLineInterpolator& interp = binfo.interps.line;
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			buffer_remove(buf, pos);
	}
}

void CBImpl::Line::reset_pencil(BrushInfo& binfo)
{
	line_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish));
}

void CBImpl::Line::reset_pen(BrushInfo& binfo)
{
	line_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish));
}

void CBImpl::Line::reset_eraser(BrushInfo& binfo)
{
	line_reset(binfo, binfo.eraser_preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_rect_alpha(buf, BrushInfo::eraser_preview_img_sx * pos.x, BrushInfo::eraser_preview_img_sy * pos.y,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy, 0);
		});
	binfo.eraser_preview_image->update_subtexture(abs_rect(binfo.interps.line.start, binfo.interps.line.finish,
		BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

void CBImpl::Line::reset_select(BrushInfo& binfo)
{
	standard_reset_outline_select(binfo);
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
	standard_outline_brush_select(canvas, x, y, canvas.binfo.interps.rect_outline);
}

void CBImpl::RectOutline::start(Canvas& canvas)
{
	standard_start(canvas, canvas.binfo.interps.rect_outline);
}

void CBImpl::RectOutline::start_select(Canvas& canvas)
{
	standard_start_select(canvas, canvas.binfo.interps.rect_outline);
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

void CBImpl::RectOutline::submit_select(Canvas& canvas)
{
	standard_submit_select(canvas, canvas.binfo.interps.rect_outline);
}

static void rect_outline_reset(BrushInfo& binfo, Buffer& buf, void(*buffer_remove)(Buffer& buf, IPosition pos))
{
	auto& interp = binfo.interps.rect_outline;
	IPosition pos;
	for (unsigned int i = 0; i < interp.length; ++i)
	{
		interp.at(i, pos.x, pos.y);
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			buffer_remove(buf, pos);
	}
}

void CBImpl::RectOutline::reset_pencil(BrushInfo& binfo)
{
	rect_outline_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	rect_outline_pencil_and_pen_update_subtexture(binfo);
}

void CBImpl::RectOutline::reset_pen(BrushInfo& binfo)
{
	rect_outline_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	rect_outline_pencil_and_pen_update_subtexture(binfo);
}

void CBImpl::RectOutline::reset_eraser(BrushInfo& binfo)
{
	rect_outline_reset(binfo, binfo.eraser_preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_rect_alpha(buf, BrushInfo::eraser_preview_img_sx * pos.x, BrushInfo::eraser_preview_img_sy * pos.y,
			BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy, 0);
		});
	rect_outline_eraser_update_subtexture(binfo);
}

void CBImpl::RectOutline::reset_select(BrushInfo& binfo)
{
	standard_reset_outline_select(binfo);
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
	standard_outline_brush_select(canvas, x, y, canvas.binfo.interps.rect_outline);
}

void CBImpl::RectFill::start(Canvas& canvas)
{
	CBImpl::RectOutline::start(canvas);
}

void CBImpl::RectFill::start_select(Canvas& canvas)
{
	CBImpl::RectOutline::start_select(canvas);
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

void CBImpl::RectFill::submit_select(Canvas& canvas)
{
	auto& interps = canvas.binfo.interps;
	interps.rect_fill.start = interps.rect_outline.start;
	interps.rect_fill.finish = interps.rect_outline.finish;
	interps.rect_fill.sync_with_endpoints();
	standard_submit_select(canvas, interps.rect_fill);
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

void CBImpl::RectFill::reset_select(BrushInfo& binfo)
{
	standard_reset_outline_select(binfo);
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
	standard_outline_brush_select(canvas, x, y, canvas.binfo.interps.ellipse_outline);
}

void CBImpl::EllipseOutline::start(Canvas& canvas)
{
	standard_start(canvas, canvas.binfo.interps.ellipse_outline);
}

void CBImpl::EllipseOutline::start_select(Canvas& canvas)
{
	standard_start_select(canvas, canvas.binfo.interps.ellipse_outline);
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

void CBImpl::EllipseOutline::submit_select(Canvas& canvas)
{
	standard_submit_select(canvas, canvas.binfo.interps.ellipse_outline);
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
		if (binfo.point_valid_in_selection(pos.x, pos.y))
			buffer_remove(buf, pos);
	}
}

void CBImpl::EllipseOutline::reset_pencil(BrushInfo& binfo)
{
	ellipse_outline_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(binfo.brushing_bbox);
}

void CBImpl::EllipseOutline::reset_pen(BrushInfo& binfo)
{
	ellipse_outline_reset(binfo, binfo.preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_pixel_alpha(buf, pos.x, pos.y, 0);
		});
	binfo.preview_image->update_subtexture(binfo.brushing_bbox);
}

void CBImpl::EllipseOutline::reset_eraser(BrushInfo& binfo)
{
	ellipse_outline_reset(binfo, binfo.eraser_preview_image->buf, [](Buffer& buf, IPosition pos) {
		buffer_set_rect_alpha(buf, pos.x, pos.y, 1, 1, 0, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy);
		});
	binfo.eraser_preview_image->update_subtexture(bounds_to_rect(binfo.brushing_bbox, BrushInfo::eraser_preview_img_sx, BrushInfo::eraser_preview_img_sy));
}

void CBImpl::EllipseOutline::reset_select(BrushInfo& binfo)
{
	standard_reset_outline_select(binfo);
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
	standard_outline_brush_select(canvas, x, y, canvas.binfo.interps.ellipse_outline);
}

void CBImpl::EllipseFill::start(Canvas& canvas)
{
	CBImpl::EllipseOutline::start(canvas);
}

void CBImpl::EllipseFill::start_select(Canvas& canvas)
{
	CBImpl::EllipseOutline::start_select(canvas);
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

void CBImpl::EllipseFill::submit_select(Canvas& canvas)
{
	auto& interps = canvas.binfo.interps;
	interps.ellipse_fill.start = interps.ellipse_outline.start;
	interps.ellipse_fill.finish = interps.ellipse_outline.finish;
	interps.ellipse_fill.sync_with_endpoints();
	standard_submit_select(canvas, interps.ellipse_fill);
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

void CBImpl::EllipseFill::reset_select(BrushInfo& binfo)
{
	standard_reset_outline_select(binfo);
}

// <<<==================================<<< BUCKET FILL >>>==================================>>>

static void bucket_fill_find_noncontiguous(Canvas& canvas, int x, int y, bool(*color_pixel)(Canvas& canvas, int x, int y))
{
	const PixelRGBA base_color = canvas.image->buf.pixel_color_at(x, y);
	IntBounds& bbox = canvas.binfo.brushing_bbox;
	Dim w = canvas.image->buf.width, h = canvas.image->buf.height;
	for (int u = 0; u < w; ++u)
	{
		for (int v = 0; v < h; ++v)
		{
			if (canvas.battr.tolerance.tol(base_color.to_rgba(), canvas.image->buf.pixel_color_at(u, v).to_rgba()))
			{
				if (color_pixel(canvas, u, v))
					update_bbox(bbox, u, v);
			}
		}
	}
}

static void bucket_fill_find_contiguous(Canvas& canvas, int x, int y, bool(*color_pixel)(Canvas& canvas, int x, int y), bool(*already_stored)(Canvas& canvas, int x, int y))
{
	static auto can_color = [](Canvas& canvas, int x, int y, RGBA base_color, bool(*already_stored)(Canvas& canvas, int x, int y)) {
		return !already_stored(canvas, x, y) && canvas.battr.tolerance.tol(base_color, canvas.image->buf.pixel_color_at(x, y).to_rgba());
		};

	static auto process = [](Canvas& canvas, int x, int y, int came_from, bool(*color_pixel)(Canvas& canvas, int x, int y), IntBounds& bbox, std::stack<std::tuple<int, int, Cardinal>>& stack) {
		if (color_pixel(canvas, x, y))
			update_bbox(bbox, x, y);
		if (came_from != Cardinal::LEFT && x > 0)
			stack.push({ x - 1, y, Cardinal::RIGHT });
		if (came_from != Cardinal::RIGHT && x < canvas.image->buf.width - 1)
			stack.push({ x + 1, y, Cardinal::LEFT });
		if (came_from != Cardinal::DOWN && y > 0)
			stack.push({ x, y - 1, Cardinal::UP });
		if (came_from != Cardinal::UP && y < canvas.image->buf.height - 1)
			stack.push({ x, y + 1, Cardinal::DOWN });
		};

	RGBA base_color = canvas.image->buf.pixel_color_at(x, y).to_rgba();
	IntBounds& bbox = canvas.binfo.brushing_bbox;
	std::stack<std::tuple<int, int, Cardinal>> stack;
	process(canvas, x, y, -1, color_pixel, bbox, stack);
	while (!stack.empty())
	{
		const auto& [u, v, came_from] = stack.top();
		stack.pop();
		if (can_color(canvas, u, v, base_color, already_stored))
			process(canvas, u, v, came_from, color_pixel, bbox, stack);
	}
}

void CBImpl::BucketFill::brush_pencil(Canvas& canvas, int x, int y)
{
	canvas.binfo.storage_2c.clear();
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	static auto color_pixel = [](Canvas& canvas, int x, int y) {
		if (!canvas.binfo.point_valid_in_selection(x, y))
			return false;
		PixelRGBA old_color = canvas.image->buf.pixel_color_at(x, y);
		PixelRGBA new_color = blend_over(canvas.applied_pencil_rgba(), old_color);
		canvas.binfo.storage_2c[{ x, y }] = { new_color, old_color };
		buffer_set_pixel_color(canvas.image->buf, x, y, new_color);
		return true;
		};
	static auto already_stored = [](Canvas& canvas, int x, int y) {
		return !canvas.binfo.point_valid_in_selection(x, y) || canvas.binfo.storage_2c.find({ x, y }) != canvas.binfo.storage_2c.end();
		};
	if (MainWindow->is_shift_pressed())
		bucket_fill_find_noncontiguous(canvas, x, y, color_pixel);
	else
		bucket_fill_find_contiguous(canvas, x, y, color_pixel, already_stored);
	if (canvas.binfo.brushing_bbox.valid())
	{
		canvas.image->update_subtexture(canvas.binfo.brushing_bbox);
		standard_push_pencil(canvas);
	}
	canvas.binfo.storage_2c.clear();
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	canvas.cursor_enter();
}

void CBImpl::BucketFill::brush_pen(Canvas& canvas, int x, int y)
{
	canvas.binfo.storage_1c.clear();
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	static auto color_pixel = [](Canvas& canvas, int x, int y) {
		if (!canvas.binfo.point_valid_in_selection(x, y))
			return false;
		canvas.binfo.storage_1c[{ x, y }] = canvas.image->buf.pixel_color_at(x, y);
		buffer_set_pixel_color(canvas.image->buf, x, y, canvas.applied_pen_rgba());
		return true;
		};
	static auto already_stored = [](Canvas& canvas, int x, int y) {
		return !canvas.binfo.point_valid_in_selection(x, y) || canvas.binfo.storage_1c.find({ x, y }) != canvas.binfo.storage_1c.end();
		};
	if (MainWindow->is_shift_pressed())
		bucket_fill_find_noncontiguous(canvas, x, y, color_pixel);
	else
		bucket_fill_find_contiguous(canvas, x, y, color_pixel, already_stored);
	if (canvas.binfo.brushing_bbox.valid())
	{
		canvas.image->update_subtexture(canvas.binfo.brushing_bbox);
		standard_push_pen(canvas, canvas.applied_color().get_pixel_rgba());
	}
	canvas.binfo.storage_1c.clear();
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	canvas.cursor_enter();
}

void CBImpl::BucketFill::brush_eraser(Canvas& canvas, int x, int y)
{
	canvas.binfo.storage_1c.clear();
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	static auto color_pixel = [](Canvas& canvas, int x, int y) {
		if (!canvas.binfo.point_valid_in_selection(x, y))
			return false;
		canvas.binfo.storage_1c[{ x, y }] = canvas.image->buf.pixel_color_at(x, y);
		buffer_set_pixel_alpha(canvas.image->buf, x, y, 0);
		return true;
		};
	static auto already_stored = [](Canvas& canvas, int x, int y) {
		return !canvas.binfo.point_valid_in_selection(x, y) || canvas.binfo.storage_1c.find({ x, y }) != canvas.binfo.storage_1c.end();
		};
	if (MainWindow->is_shift_pressed())
		bucket_fill_find_noncontiguous(canvas, x, y, color_pixel);
	else
		bucket_fill_find_contiguous(canvas, x, y, color_pixel, already_stored);
	if (canvas.binfo.brushing_bbox.valid())
	{
		canvas.image->update_subtexture(canvas.binfo.brushing_bbox);
		standard_push_eraser(canvas);
	}
	canvas.binfo.storage_1c.clear();
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	canvas.cursor_enter();
}

void CBImpl::BucketFill::brush_select(Canvas& canvas, int x, int y)
{
	standard_start_select_check_replace(canvas);
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	static auto select_pixel = [](Canvas& canvas, int x, int y) {
		if (canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY)
			return canvas.binfo.add_to_selection({ x, y });
		else if (canvas.cursor_state == Canvas::CursorState::DOWN_ALTERNATE)
			return canvas.binfo.remove_from_selection({ x, y });
		else
			return false;
		};
	static auto alerady_stored = [](Canvas& canvas, int x, int y) {
		if (canvas.cursor_state == Canvas::CursorState::DOWN_PRIMARY)
			return canvas.binfo.smants->get_points().contains({ x, y });
		else if (canvas.cursor_state == Canvas::CursorState::DOWN_ALTERNATE)
			return !canvas.binfo.smants->get_points().contains({ x, y });
		else
			return true;
		};
	if (MainWindow->is_shift_pressed())
		bucket_fill_find_noncontiguous(canvas, x, y, select_pixel);
	else
		bucket_fill_find_contiguous(canvas, x, y, select_pixel, alerady_stored);
	canvas.binfo.smants->send_buffer(canvas.binfo.brushing_bbox);
	canvas.binfo.push_selection_to_history();
	canvas.binfo.brushing_bbox = IntBounds::NADIR;
	canvas.cursor_enter();
}

void CBImpl::BucketFill::reset_select(BrushInfo& binfo)
{
	standard_reset_outline_select(binfo);
}
