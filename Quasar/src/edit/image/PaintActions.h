#pragma once

#include "variety/History.h"
#include "Image.h"
#include "../color/Color.h"

// TODO utility to get color from buffer pos

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
	std::unordered_map<CanvasPixel, PixelRGBA> painted_colors;
	PaintToolAction(const std::shared_ptr<Image>& image, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct LineToolAction : public ActionBase
{
	std::weak_ptr<Image> image;
	PixelRGBA color;
	int x1, x2, y1, y2;
	std::unordered_map<IPosition, PixelRGBA> painted_colors;
	LineToolAction(const std::shared_ptr<Image>& image, PixelRGBA color, IPosition start, IPosition finish, std::unordered_map<IPosition, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct LineBlendToolAction : public ActionBase
{
	std::weak_ptr<Image> image;
	int x1, x2, y1, y2;
	std::unordered_map<CanvasPixel, PixelRGBA> painted_colors;
	LineBlendToolAction(const std::shared_ptr<Image>& image, IPosition start, IPosition finish, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};
