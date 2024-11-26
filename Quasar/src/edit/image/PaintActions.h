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

struct PaintToolAction : public ActionBase
{
	std::weak_ptr<Image> image;
	IntBounds bbox;
	std::unordered_map<CanvasPixel, PixelRGBA> painted_colors;
	PaintToolAction(const std::shared_ptr<Image>& image, IntBounds bbox, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct LineToolAction : public ActionBase
{
	std::weak_ptr<Image> image;
	PixelRGBA color;
	IntBounds bbox;
	std::unordered_map<IPosition, PixelRGBA> painted_colors;
	LineToolAction(const std::shared_ptr<Image>& image, PixelRGBA color, IPosition start, IPosition finish, std::unordered_map<IPosition, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct LineBlendToolAction : public ActionBase
{
	std::weak_ptr<Image> image;
	IntBounds bbox;
	std::unordered_map<CanvasPixel, PixelRGBA> painted_colors;
	LineBlendToolAction(const std::shared_ptr<Image>& image, IPosition start, IPosition finish, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};
