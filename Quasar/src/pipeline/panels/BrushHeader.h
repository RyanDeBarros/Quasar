#pragma once

enum class BrushTip
{
	PENCIL = 0b1,
	PEN = 0b10,
	ERASER = 0b100,
	SELECT = 0b1000
};

inline int operator&(BrushTip a, BrushTip b) { return int(a) & int(b); }
inline int operator&(int a, BrushTip b) { return a & int(b); }
inline int operator&(BrushTip a, int b) { return int(a) & b; }
inline int operator|(BrushTip a, BrushTip b) { return int(a) | int(b); }
inline int operator|(int a, BrushTip b) { return a | int(b); }
inline int operator|(BrushTip a, int b) { return int(a) | b; }
inline int operator~(BrushTip a) { return ~int(a); }

enum class BrushTool
{
	CAMERA,
	PAINT,
	LINE,
	FILL,
	RECT_OUTLINE,
	RECT_FILL,
	ELLIPSE_OUTLINE,
	ELLIPSE_FILL,
};
