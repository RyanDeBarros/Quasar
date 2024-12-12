#pragma once

#include "../../panels/BrushHeader.h"
#include "PaintActions.h"

struct BrushInfo
{
	struct Canvas* canvas = nullptr;

	enum class State
	{
		NEUTRAL = 0b1,
		BRUSHING = 0b10,
		PIPETTE = 0b100,
		MOVING_SUBIMG = 0b1000
	} state = State::NEUTRAL;

	bool cancelling = false;
	BrushTip tip = BrushTip::PENCIL;
	BrushTool tool = BrushTool::CAMERA;
	IPosition starting_pos = { -1, -1 };
	IPosition last_brush_pos = { -1, -1 };
	IPosition image_pos = { -1, -1 };
	IntBounds brushing_bbox = IntBounds::NADIR;
	PixelRGBA brushing_color = {};
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
	std::unordered_set<IPosition> raw_selection_positions;

	struct FlatSprite* sel_subimg_sprite = nullptr;
	std::shared_ptr<Image> selection_subimage;
	IPosition move_selpxs_offset = {};
	bool apply_selection_with_pencil = true;

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
	bool push_selection_to_history();
	bool point_valid_in_selection(int x, int y) const;
};

inline int operator|(BrushInfo::State a, BrushInfo::State b) { return int(a) | int(b); }
inline int operator|(BrushInfo::State a, int b) { return int(a) | b; }
inline int operator|(int a, BrushInfo::State b) { return a | int(b); }
inline int operator&(BrushInfo::State a, BrushInfo::State b) { return int(a) & int(b); }
inline int operator&(BrushInfo::State a, int b) { return int(a) & b; }
inline int operator&(int a, BrushInfo::State b) { return a & int(b); }
inline int operator~(BrushInfo::State a) { return ~int(a); }
