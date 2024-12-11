#pragma once

#include "../../panels/BrushHeader.h"
#include "PaintActions.h"

struct BrushInfo
{
	struct Canvas* canvas = nullptr;

	bool brushing = false;
	bool cancelling = false;
	BrushTip tip = BrushTip::PENCIL;
	BrushTool tool = BrushTool::CAMERA;
	IPosition starting_pos = { -1, -1 };
	IPosition last_brush_pos = { -1, -1 };
	IPosition image_pos = { -1, -1 };
	IntBounds brushing_bbox = IntBounds::NADIR;
	bool show_brush_preview = false;
	std::shared_ptr<Image> preview_image, eraser_preview_image;
	static const int eraser_preview_img_sx = 2, eraser_preview_img_sy = 2;
	std::unordered_map<IPosition, PixelRGBA> storage_1c;
	std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>> storage_2c;

	bool show_selection_preview = false;
	class SelectionMants* smants = nullptr;
	class SelectionMants* smants_preview = nullptr;
	std::unordered_set<IPosition> storage_select_remove;
	std::unordered_set<IPosition> storage_select_add;
	std::unordered_set<IPosition> cleared_selection;
	std::unordered_map<IPosition, PixelRGBA> storage_selection_1c;
	std::unordered_map<IPosition, PixelRGBA> raw_selection_pixels;

	bool show_selection_subimage = false;
	struct FlatSprite* sel_subimg_sprite = nullptr;
	std::shared_ptr<Image> selection_subimage;
	IPosition move_selpxs_offset = {};

	struct
	{
		DiscreteLineInterpolator line = {};
		DiscreteRectOutlineInterpolator rect_outline = {};
		DiscreteRectFillInterpolator rect_fill = {};
		DiscreteRectFillInterpolator temp_rect_fill = {};
		DiscreteEllipseOutlineInterpolator ellipse_outline = {};
		DiscreteEllipseFillInterpolator ellipse_fill = {};
	} interps;

	void reset();
	bool add_to_selection(IPosition pos);
	bool remove_from_selection(IPosition pos);
	IntBounds clear_selection();
	void push_selection_to_history();
	bool point_valid_in_selection(int x, int y) const;
};
