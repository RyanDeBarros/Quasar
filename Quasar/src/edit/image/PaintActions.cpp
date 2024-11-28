#include "PaintActions.h"

void buffer_set_pixel_color(const Buffer& buf, int x, int y, PixelRGBA c)
{
	for (CHPP i = 0; i < buf.chpp; ++i)
		buf.pos(x, y)[i] = c[i];
}

void buffer_set_pixel_alpha(const Buffer& buf, int x, int y, int alpha)
{
	buf.pos(x, y)[std::max(buf.chpp - 1, 0)] = alpha;
}

void buffer_set_rect_alpha(const Buffer& buf, int x, int y, int w, int h, int alpha, int sx, int sy)
{
	x *= sx;
	y *= sy;
	w *= sx;
	h *= sy;
	for (int i = 0; i < w; ++i)
		for (int j = 0; j < h; ++j)
			buffer_set_pixel_alpha(buf, x + i, y + j, alpha);
}

void DiscreteLineInterpolator::sync_with_endpoints()
{
	delta = finish - start;
	length = std::max(std::abs(delta.x), std::abs(delta.y)) + 1;
}

void DiscreteLineInterpolator::at(int i, int& x, int& y) const
{
	float fraction = length == 1 ? 0 : (float)i / (length - 1);
	x = start.x + glm::sign(delta.x) * roundi_down_on_half(std::abs(delta.x) * fraction);
	y = start.y + glm::sign(delta.y) * roundi_down_on_half(std::abs(delta.y) * fraction);
}

void DiscreteRectFillInterpolator::sync_with_endpoints()
{
	delta = finish - start;
	length = (std::abs(delta.x) + 1) * (std::abs(delta.y) + 1);
}

void DiscreteRectFillInterpolator::at(int i, int& x, int& y) const
{
	int xi = i % (1 + std::abs(delta.x));
	int yi = i / (1 + std::abs(delta.x));
	x = start.x + glm::sign(delta.x) * xi;
	y = start.y + glm::sign(delta.y) * yi;
}

// TODO remove interpolator data members from difference. Likewise, use bounding box for rect interp as well.
void DiscreteRectFillDifference::sync_with_interpolators()
{
	first_bbox = abs_bounds(first.start, first.finish);
	second_bbox = abs_bounds(second.start, second.finish);

	if (intersection(first_bbox.x1, first_bbox.x2, second_bbox.x1, second_bbox.x2, middle_bbox.x1, middle_bbox.x2)
			&& intersection(first_bbox.y1, first_bbox.y2, second_bbox.y1, second_bbox.y2, middle_bbox.y1, middle_bbox.y2))
		common_length = middle_bbox.width_no_abs() * middle_bbox.height_no_abs();
	else
		common_length = 0;

	remove_length = first.length - common_length;
	insert_length = second.length - common_length;
}

static void interpolate_rect_fill_difference(int i, int& x, int& y, IntBounds with, IntBounds without)
{
	int width = with.width_no_abs();
	int area = width * (without.y1 - with.y1);
	if (i < area)
	{
		x = with.x1 + i % width;
		y = with.y1 + i / width;
	}
	else
	{
		i -= area;
		width = without.x1 - with.x1;
		area = width * without.height_no_abs();
		if (i < area)
		{
			x = with.x1 + i % (without.x1 - with.x1);
			y = without.y1 + i / (without.x1 - with.x1);
		}
		else
		{
			i -= area;
			width = with.x2 - without.x2;
			area = width * without.height_no_abs();
			if (i < area)
			{
				x = without.x2 + 1 + i % width;
				y = without.y1 + i / width;
			}
			else
			{
				i -= area;
				width = with.width_no_abs();
				x = with.x1 + i % width;
				y = without.y2 + 1 + i / width;
			}
		}
	}
}

void DiscreteRectFillDifference::remove_at(int i, int& x, int& y) const
{
	if (common_length == 0)
	{
		first.at(i, x, y);
		return;
	}
	interpolate_rect_fill_difference(i, x, y, first_bbox, middle_bbox);
}

void DiscreteRectFillDifference::insert_at(int i, int& x, int& y) const
{
	if (common_length == 0)
	{
		second.at(i, x, y);
		return;
	}
	interpolate_rect_fill_difference(i, x, y, second_bbox, middle_bbox);
}

PaintToolAction::PaintToolAction(const std::shared_ptr<Image>& image, IntBounds bbox, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors)
	: image(image), bbox(bbox), painted_colors(std::move(painted_colors))
{
	weight = sizeof(PaintToolAction) + this->painted_colors.size() * (sizeof(IPosition) + 2 * sizeof(PixelRGBA));
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
			buffer_set_pixel_color(buf, x, y, iter->second.second);
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
			buffer_set_pixel_color(buf, x, y, iter->second.first);
		}
		img->update_subtexture(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	}
}

OneColorPenAction::OneColorPenAction(const std::shared_ptr<Image>& image, PixelRGBA color, IPosition start, IPosition finish, std::unordered_map<IPosition, PixelRGBA>&& painted_colors)
	: image(image), color(color), painted_colors(std::move(painted_colors))
{
	weight = sizeof(OneColorPenAction) + this->painted_colors.size() * (sizeof(IPosition) + sizeof(PixelRGBA));
	bbox = abs_bounds(start, finish);
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

OneColorPencilAction::OneColorPencilAction(const std::shared_ptr<Image>& image, IPosition start, IPosition finish, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors)
	: image(image), painted_colors(std::move(painted_colors))
{
	weight = sizeof(OneColorPencilAction) + this->painted_colors.size() * (sizeof(IPosition) + 2 * sizeof(PixelRGBA));
	bbox = abs_bounds(start, finish);
}

void OneColorPencilAction::forward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second.first);
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
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second.second);
		img->update_subtexture(bbox.x1, bbox.y1, bbox.x2 - bbox.x1 + 1, bbox.y2 - bbox.y1 + 1);
	}
}
