#pragma once

#include <array>

#include "variety/History.h"
#include "Image.h"
#include "../color/Color.h"

extern void buffer_set_pixel_color(const Buffer& buf, int x, int y, PixelRGBA c);
extern void buffer_set_pixel_alpha(const Buffer& buf, int x, int y, int alpha);
extern void buffer_set_rect_alpha(const Buffer& buf, int x, int y, int w, int h, int alpha, int sx = 1, int sy = 1);

struct DiscreteInterpolator
{
	IPosition start = {};
	IPosition finish = {};
	unsigned int length = 0;

	virtual void sync_with_endpoints() = 0;
	virtual void at(int i, int& x, int& y) const = 0;
};

struct DiscreteLineInterpolator : public DiscreteInterpolator
{
	IPosition delta = {};

	virtual void sync_with_endpoints() override;
	virtual void at(int i, int& x, int& y) const override;
};

struct DiscreteRectOutlineInterpolator : public DiscreteInterpolator
{
	IPosition delta = {};

	virtual void sync_with_endpoints() override;
	virtual void at(int i, int& x, int& y) const override;

	std::array<IntRect, 4> lines() const;
};

struct DiscreteRectFillInterpolator : public DiscreteInterpolator
{
	IPosition delta = {};

	virtual void sync_with_endpoints() override;
	virtual void at(int i, int& x, int& y) const override;
};

struct DiscreteEllipseOutlineInterpolator : public DiscreteInterpolator
{
	IPosition delta = {};

	virtual void sync_with_endpoints() override;
	virtual void at(int i, int& x, int& y) const override;

private:
	std::vector<IPosition> points;
};

struct DiscreteEllipseFillInterpolator : public DiscreteInterpolator
{
	IPosition delta = {};

	virtual void sync_with_endpoints() override;
	virtual void at(int i, int& x, int& y) const override;

private:
	std::vector<IPosition> points;
};

struct DiscreteRectDifference
{
	const DiscreteInterpolator* first;
	const DiscreteInterpolator* second;
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

	std::array<IntRect, 8> modified_regions() const;
};

// LATER given that paint actions use weak ptrs, make sure that a list of shared_ptrs of past canvas images is kept after changing the canvas image, to keep these actions alive.
// Or else, clear history upon loading new canvas image.
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
