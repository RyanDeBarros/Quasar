#include "PaintActions.h"

void buffer_set_pixel_color(const Buffer& buf, int x, int y, PixelRGBA c)
{
	for (CHPP i = 0; i < buf.chpp; ++i)
		buf.pos(x, y)[i] = c[i];
}

DiscreteLineInterpolator::DiscreteLineInterpolator(IPosition start, IPosition finish)
	: start(start), finish(finish), delta(finish - start), length(std::max(std::abs(delta.x), std::abs(delta.y)) + 1) {}

void DiscreteLineInterpolator::at(int i, int& x, int& y) const
{
	float fraction = length == 1 ? 0 : (float)i / (length - 1);
	x = start.x + glm::sign(delta.x) * roundi_down_on_half(std::abs(delta.x) * fraction);
	y = start.y + glm::sign(delta.y) * roundi_down_on_half(std::abs(delta.y) * fraction);
}

DiscreteRectFillInterpolator::DiscreteRectFillInterpolator(IPosition start, IPosition finish)
	: start(start), finish(finish), delta(finish - start), length((std::abs(delta.x) + 1) * (std::abs(delta.y) + 1)) {}

void DiscreteRectFillInterpolator::at(int i, int& x, int& y) const
{
	int xi = i % (1 + std::abs(delta.x));
	int yi = i / (1 + std::abs(delta.x));
	x = start.x + glm::sign(delta.x) * xi;
	y = start.y + glm::sign(delta.y) * yi;
}

PaintToolAction::PaintToolAction(const std::shared_ptr<Image>& image, IntBounds bbox, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors)
	: image(image), bbox(bbox), painted_colors(std::move(painted_colors))
{
	weight = sizeof(PaintToolAction) + this->painted_colors.size() * (sizeof(CanvasPixel) + sizeof(PixelRGBA));
}

void PaintToolAction::forward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		int x1 = INT_MAX, y1 = INT_MAX, x2 = INT_MIN, y2 = INT_MIN;
		for (auto iter = painted_colors.begin(); iter != painted_colors.end(); ++iter)
		{
			int x = iter->first.x, y = iter->first.y;
			if (x < x1)
				x1 = x;
			if (x > x2)
				x2 = x;
			if (y < y1)
				y1 = y;
			if (y > y2)
				y2 = y;
			buffer_set_pixel_color(buf, x, y, iter->second);
		}
		img->update_subtexture(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	}
}

void PaintToolAction::backward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		int x1 = INT_MAX, y1 = INT_MAX, x2 = INT_MIN, y2 = INT_MIN;
		for (auto iter = painted_colors.begin(); iter != painted_colors.end(); ++iter)
		{
			int x = iter->first.x, y = iter->first.y;
			if (x < x1)
				x1 = x;
			if (x > x2)
				x2 = x;
			if (y < y1)
				y1 = y;
			if (y > y2)
				y2 = y;
			buffer_set_pixel_color(buf, x, y, iter->first.c);
		}
		img->update_subtexture(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	}
}

OneColorPenAction::OneColorPenAction(const std::shared_ptr<Image>& image, PixelRGBA color, IPosition start, IPosition finish, std::unordered_map<IPosition, PixelRGBA>&& painted_colors)
	: image(image), color(color), painted_colors(std::move(painted_colors))
{
	weight = sizeof(OneColorPenAction) + this->painted_colors.size() * (sizeof(IPosition) + sizeof(PixelRGBA));
	bbox.x1 = std::min(start.x, finish.x);
	bbox.x2 = std::max(start.x, finish.x);
	bbox.y1 = std::min(start.y, finish.y);
	bbox.y2 = std::max(start.y, finish.y);
}

void OneColorPenAction::forward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, color);
		img->update_subtexture(bbox.x1, bbox.y1, bbox.x2 - bbox.x1 + 1, bbox.y2 - bbox.y1 + 1);
	}
}

void OneColorPenAction::backward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second);
		img->update_subtexture(bbox.x1, bbox.y1, bbox.x2 - bbox.x1 + 1, bbox.y2 - bbox.y1 + 1);
	}
}

OneColorPencilAction::OneColorPencilAction(const std::shared_ptr<Image>& image, IPosition start, IPosition finish, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors)
	: image(image), painted_colors(painted_colors)
{
	weight = sizeof(OneColorPencilAction) + this->painted_colors.size() * (sizeof(CanvasPixel) + sizeof(PixelRGBA));
	bbox.x1 = std::min(start.x, finish.x);
	bbox.x2 = std::max(start.x, finish.x);
	bbox.y1 = std::min(start.y, finish.y);
	bbox.y2 = std::max(start.y, finish.y);
}

void OneColorPencilAction::forward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.first.c);
		img->update_subtexture(bbox.x1, bbox.y1, bbox.x2 - bbox.x1 + 1, bbox.y2 - bbox.y1 + 1);
	}
}

void OneColorPencilAction::backward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second);
		img->update_subtexture(bbox.x1, bbox.y1, bbox.x2 - bbox.x1 + 1, bbox.y2 - bbox.y1 + 1);
	}
}
