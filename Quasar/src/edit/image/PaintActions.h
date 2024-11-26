#pragma once

#include "variety/History.h"
#include "Image.h"
#include "../color/Color.h"

extern void buffer_set_pixel_color(const Buffer& buf, int x, int y, PixelRGBA c);

struct CanvasPixel
{
	int x, y;
	PixelRGBA c;
	bool operator==(const CanvasPixel& cpx) const { return x == cpx.x && y == cpx.y; }
};

template<>
struct std::hash<CanvasPixel>
{
	size_t operator()(const CanvasPixel& cpx) const { return std::hash<int>{}(cpx.x) ^ std::hash<int>{}(cpx.y); }
};

struct DiscreteLineInterpolator
{
	IPosition start;
	IPosition finish;
	IPosition delta;
	unsigned int length;

	DiscreteLineInterpolator(IPosition start, IPosition finish);

	IPosition at(int i) const { IPosition pos; at(i, pos); return pos; }
	void at(int i, IPosition& pos) const { at(i, pos.x, pos.y); }
	void at(int i, int& x, int& y) const;
};

struct DiscreteRectFillInterpolator
{
	IPosition start;
	IPosition finish;
	IPosition delta;
	unsigned int length;

	DiscreteRectFillInterpolator(IPosition start, IPosition finish);

	IPosition at(int i) const { IPosition pos; at(i, pos); return pos; }
	void at(int i, IPosition& pos) const { at(i, pos.x, pos.y); }
	void at(int i, int& x, int& y) const;
};

struct PaintToolAction : public ActionBase
{
	std::weak_ptr<Image> image;
	IntBounds bbox;
	std::unordered_map<CanvasPixel, PixelRGBA> painted_colors;
	PaintToolAction(const std::shared_ptr<Image>& image, IntBounds bbox, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct OneColorPenAction : public ActionBase
{
	std::weak_ptr<Image> image;
	PixelRGBA color;
	IntBounds bbox;
	std::unordered_map<IPosition, PixelRGBA> painted_colors;
	OneColorPenAction(const std::shared_ptr<Image>& image, PixelRGBA color, IPosition start, IPosition finish, std::unordered_map<IPosition, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct OneColorPencilAction : public ActionBase
{
	std::weak_ptr<Image> image;
	IntBounds bbox;
	std::unordered_map<CanvasPixel, PixelRGBA> painted_colors;
	OneColorPencilAction(const std::shared_ptr<Image>& image, IPosition start, IPosition finish, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};
