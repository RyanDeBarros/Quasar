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
	MOVE = 0b1,
	PAINT = 0b10,
	LINE = 0b100,
	FILL = 0b1000,
	RECT_OUTLINE = 0b1'0000,
	RECT_FILL = 0b10'0000,
	ELLIPSE_OUTLINE = 0b100'0000,
	ELLIPSE_FILL = 0b1000'0000,
};

inline int operator&(BrushTool a, BrushTool b) { return int(a) & int(b); }
inline int operator&(int a, BrushTool b) { return a & int(b); }
inline int operator&(BrushTool a, int b) { return int(a) & b; }
inline int operator|(BrushTool a, BrushTool b) { return int(a) | int(b); }
inline int operator|(int a, BrushTool b) { return a | int(b); }
inline int operator|(BrushTool a, int b) { return int(a) | b; }
inline int operator~(BrushTool a) { return ~int(a); }
