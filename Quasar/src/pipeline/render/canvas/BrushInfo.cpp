#include "BrushInfo.h"

#include "CanvasBrushImpl.h"
#include "SelectionMants.h"
#include "user/Machine.h"

void BrushInfo::reset()
{
	if (tool & BrushTool::PAINT)
	{
		if (tip & BrushTip::PENCIL)
			CBImpl::Paint::reset_pencil(*this);
		else if (tip & BrushTip::PEN)
			CBImpl::Paint::reset_pen(*this);
		else if (tip & BrushTip::ERASER)
			CBImpl::Paint::reset_eraser(*this);
		else if (tip & BrushTip::SELECT)
			CBImpl::Paint::reset_select(*this);
	}
	else if (tool & BrushTool::LINE)
	{
		if (tip & BrushTip::PENCIL)
			CBImpl::Line::reset_pencil(*this);
		else if (tip & BrushTip::PEN)
			CBImpl::Line::reset_pen(*this);
		else if (tip & BrushTip::ERASER)
			CBImpl::Line::reset_eraser(*this);
		else if (tip & BrushTip::SELECT)
			CBImpl::Line::reset_select(*this);
	}
	else if (tool & BrushTool::RECT_OUTLINE)
	{
		if (tip & BrushTip::PENCIL)
			CBImpl::RectOutline::reset_pencil(*this);
		else if (tip & BrushTip::PEN)
			CBImpl::RectOutline::reset_pen(*this);
		else if (tip & BrushTip::ERASER)
			CBImpl::RectOutline::reset_eraser(*this);
		else if (tip & BrushTip::SELECT)
			CBImpl::RectOutline::reset_select(*this);
	}
	else if (tool & BrushTool::RECT_FILL)
	{
		if (tip & BrushTip::PENCIL)
			CBImpl::RectFill::reset_pencil(*this);
		else if (tip & BrushTip::PEN)
			CBImpl::RectFill::reset_pen(*this);
		else if (tip & BrushTip::ERASER)
			CBImpl::RectFill::reset_eraser(*this);
		else if (tip & BrushTip::SELECT)
			CBImpl::RectFill::reset_select(*this);
	}
	else if (tool & BrushTool::ELLIPSE_OUTLINE)
	{
		if (tip & BrushTip::PENCIL)
			CBImpl::EllipseOutline::reset_pencil(*this);
		else if (tip & BrushTip::PEN)
			CBImpl::EllipseOutline::reset_pen(*this);
		else if (tip & BrushTip::ERASER)
			CBImpl::EllipseOutline::reset_eraser(*this);
		else if (tip & BrushTip::SELECT)
			CBImpl::EllipseOutline::reset_select(*this);
	}
	else if (tool & BrushTool::ELLIPSE_FILL)
	{
		if (tip & BrushTip::PENCIL)
			CBImpl::EllipseFill::reset_pencil(*this);
		else if (tip & BrushTip::PEN)
			CBImpl::EllipseFill::reset_pen(*this);
		else if (tip & BrushTip::ERASER)
			CBImpl::EllipseFill::reset_eraser(*this);
		else if (tip & BrushTip::SELECT)
			CBImpl::EllipseFill::reset_select(*this);
	}
	cancelling = false;
	brushing_bbox = IntBounds::NADIR;
	last_brush_pos = { -1, -1 };
	starting_pos = { -1, -1 };
	show_brush_preview = false;
	storage_1c.clear();
	storage_2c.clear();
	show_selection_preview = false;
	smants->send_uniforms();
	smants_preview->send_buffer(smants_preview->clear());
	storage_select_add.clear();
	storage_select_remove.clear();
	cleared_selection.clear();
}

bool BrushInfo::add_to_selection(IPosition pos)
{
	if (smants->add(pos))
	{
		if (storage_select_remove.contains(pos))
			storage_select_remove.erase(pos);
		else
			storage_select_add.insert(pos);
		return true;
	}
	return false;
}

bool BrushInfo::remove_from_selection(IPosition pos)
{
	if (smants->remove(pos))
	{
		if (storage_select_add.contains(pos))
			storage_select_add.erase(pos);
		else
			storage_select_remove.insert(pos);
		return true;
	}
	return false;
}

IntBounds BrushInfo::clear_selection()
{
	IntBounds bbox = IntBounds::NADIR;
	IPosition pos{};
	while (!smants->get_points().empty())
	{
		pos = *smants->get_points().begin();
		if (remove_from_selection(pos))
		{
			update_bbox(bbox, pos.x, pos.y);
			cleared_selection.insert(pos);
		}
	}
	return bbox;
}

bool BrushInfo::push_selection_to_history()
{
	if (brushing_bbox == IntBounds::NADIR)
		return false;
	Machine.history.push(std::make_shared<SmantsModifyAction>(smants, brushing_bbox, std::move(storage_select_remove), std::move(storage_select_add)));
	return true;
}

bool BrushInfo::point_valid_in_selection(int x, int y) const
{
	return smants->get_points().empty() || smants->get_points().contains({ x, y });
}
