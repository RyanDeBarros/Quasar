#include "PaintActions.h"

void buffer_set_pixel_color(const Buffer& buf, int x, int y, PixelRGBA c)
{
	for (CHPP i = 0; i < buf.chpp; ++i)
		buf.pos(x, y)[i] = c[i];
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

LineToolAction::LineToolAction(const std::shared_ptr<Image>& image, PixelRGBA color, IPosition start, IPosition finish, std::unordered_map<IPosition, PixelRGBA>&& painted_colors)
	: image(image), color(color), painted_colors(std::move(painted_colors))
{
	weight = sizeof(LineToolAction) + this->painted_colors.size() * (sizeof(IPosition) + sizeof(PixelRGBA));
	bbox.x1 = std::min(start.x, finish.x);
	bbox.x2 = std::max(start.x, finish.x);
	bbox.y1 = std::min(start.y, finish.y);
	bbox.y2 = std::max(start.y, finish.y);
}

void LineToolAction::forward()
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

void LineToolAction::backward()
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

LineBlendToolAction::LineBlendToolAction(const std::shared_ptr<Image>& image, IPosition start, IPosition finish, std::unordered_map<CanvasPixel, PixelRGBA>&& painted_colors)
	: image(image), painted_colors(painted_colors)
{
	weight = sizeof(LineBlendToolAction) + this->painted_colors.size() * (sizeof(CanvasPixel) + sizeof(PixelRGBA));
	bbox.x1 = std::min(start.x, finish.x);
	bbox.x2 = std::max(start.x, finish.x);
	bbox.y1 = std::min(start.y, finish.y);
	bbox.y2 = std::max(start.y, finish.y);
}

void LineBlendToolAction::forward()
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

void LineBlendToolAction::backward()
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
