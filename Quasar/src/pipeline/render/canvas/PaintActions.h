#pragma once

#include <array>
#include <unordered_set>

#include "variety/History.h"
#include "edit/image/Image.h"
#include "edit/color/Color.h"

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

// LATER given that paint actions use weak ptrs, make sure that a list of shared_ptrs of past canvas images is kept after changing the canvas image, to keep these actions alive.
// Or else, clear history upon loading new canvas image.
// LATER OneAlphaAction, that only affects alpha, not entire color. Might not be simple to refactor though, since setting alpha/color is a bit inconsistent in CBImpl.
struct OneColorAction : public ActionBase
{
	std::weak_ptr<Image> image;
	PixelRGBA color;
	IntBounds bbox;
	std::unordered_map<IPosition, PixelRGBA> painted_colors;
	OneColorAction(const std::shared_ptr<Image>& image, PixelRGBA color, IntBounds bbox, std::unordered_map<IPosition, PixelRGBA>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct TwoColorAction : public ActionBase
{
	std::weak_ptr<Image> image;
	IntBounds bbox;
	std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>> painted_colors;
	TwoColorAction(const std::shared_ptr<Image>& image, IntBounds bbox, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct TwoColorMoveAction : public ActionBase
{
	std::weak_ptr<Image> image;
	IntBounds bbox_remove, bbox_add;
	std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>> painted_colors;
	TwoColorMoveAction(const std::shared_ptr<Image>& image, IntBounds bbox_remove, IntBounds bbox_add, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors);
	virtual void forward() override;
	virtual void backward() override;
};

struct SmantsModifyAction : public ActionBase
{
	class SelectionMants* smants;
	IntBounds bbox;
	std::unordered_set<IPosition> remove_points;
	std::unordered_set<IPosition> add_points;
	SmantsModifyAction(class SelectionMants* smants, IntBounds bbox, std::unordered_set<IPosition>&& remove_points, std::unordered_set<IPosition>&& add_points);
	virtual void forward() override;
	virtual void backward() override;
};

struct SmantsMoveAction : public ActionBase
{
	IPosition delta;
	std::unordered_set<IPosition> premove_points;
	SmantsMoveAction(IPosition delta, std::unordered_set<IPosition>&& premove_points);
	virtual void forward() override;
	virtual void backward() override;
};
