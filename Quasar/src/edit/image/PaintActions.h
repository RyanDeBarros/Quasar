#pragma once

#include "variety/History.h"
#include "Image.h"
#include "../color/Color.h"

extern void buffer_set_pixel_color(const Buffer& buf, int x, int y, PixelRGBA c);

struct DiscreteLineInterpolator
{
	IPosition start = {};
	IPosition finish = {};
	IPosition delta = {};
	unsigned int length = 1;

	void sync_with_endpoints();
	IPosition at(int i) const { IPosition pos; at(i, pos); return pos; }
	void at(int i, IPosition& pos) const { at(i, pos.x, pos.y); }
	void at(int i, int& x, int& y) const;
};

struct DiscreteRectFillInterpolator
{
	IPosition start = {};
	IPosition finish = {};
	IPosition delta = {};
	unsigned int length = 1;

	void sync_with_endpoints();
	IPosition at(int i) const { IPosition pos; at(i, pos); return pos; }
	void at(int i, IPosition& pos) const { at(i, pos.x, pos.y); }
	void at(int i, int& x, int& y) const;
};

struct DiscreteRectFillDifference
{
	DiscreteRectFillInterpolator first;
	DiscreteRectFillInterpolator second;
	IntBounds first_bbox;
	IntBounds second_bbox;
	IntBounds middle_bbox;
	unsigned int common_length;
	unsigned int remove_length;
	unsigned int insert_length;

	void sync_with_interpolators();
	IPosition remove_at(int i) const { IPosition pos; remove_at(i, pos); return pos; }
	void remove_at(int i, IPosition& pos) const { remove_at(i, pos.x, pos.y); }
	void remove_at(int i, int& x, int& y) const;
	IPosition insert_at(int i) const { IPosition pos; insert_at(i, pos); return pos; }
	void insert_at(int i, IPosition& pos) const { insert_at(i, pos.x, pos.y); }
	void insert_at(int i, int& x, int& y) const;
};

struct PaintToolAction : public ActionBase
{
	std::weak_ptr<Image> image;
	IntBounds bbox;
	std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>> painted_colors;
	PaintToolAction(const std::shared_ptr<Image>& image, IntBounds bbox, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors);
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
	std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>> painted_colors;
	OneColorPencilAction(const std::shared_ptr<Image>& image, IPosition start, IPosition finish, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};
