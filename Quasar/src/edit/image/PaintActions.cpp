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

void DiscreteRectOutlineInterpolator::sync_with_endpoints()
{
	delta = finish - start;
	int dw = std::abs(delta.x);
	int dh = std::abs(delta.y);
	if (dw == 0)
		length = dh + 1;
	else if (dh == 0)
		length = dw + 1;
	else
		length = 2 * (dw + dh);
}

void DiscreteRectOutlineInterpolator::at(int i, int& x, int& y) const
{
	int dw = std::abs(delta.x);
	int dh = std::abs(delta.y);
	if (dw == 0)
	{
		x = start.x;
		y = start.y + glm::sign(delta.y) * i;
	}
	else if (dh == 0)
	{
		x = start.x + glm::sign(delta.x) * i;
		y = start.y;
	}
	else
	{
		if (i < dw)
		{
			x = start.x + glm::sign(delta.x) * i;
			y = start.y;
		}
		else
		{
			i -= dw;
			if (i < dh)
			{
				x = finish.x;
				y = start.y + glm::sign(delta.y) * i;
			}
			else
			{
				i -= dh;
				if (i < dh)
				{
					x = start.x;
					y = start.y + glm::sign(delta.y) * (i + 1);
				}
				else
				{
					x = start.x + glm::sign(delta.x) * (i - dh + 1);
					y = finish.y;
				}
			}
		}
	}
}

std::array<IntRect, 4> DiscreteRectOutlineInterpolator::lines() const
{
	std::array<IntRect, 4> lines;
	int dw = std::abs(delta.x);
	int dh = std::abs(delta.y);
	int rx = std::min(start.x, finish.x);
	int ry = std::min(start.y, finish.y);
	if (dw == 0)
	{
		lines[0].w = 1;
		lines[0].h = dh + 1;
		lines[0].x = rx;
		lines[0].y = ry;
	}
	else if (dh == 0)
	{
		lines[0].w = dw + 1;
		lines[0].h = 1;
		lines[0].x = rx;
		lines[0].y = ry;
	}
	else
	{
		lines[0].w = dw;
		lines[0].h = 1;
		lines[0].x = rx;
		lines[0].y = ry;

		lines[1].w = 1;
		lines[1].h = dh;
		lines[1].x = rx + dw;
		lines[1].y = ry;

		lines[2].w = 1;
		lines[2].h = dh;
		lines[2].x = rx;
		lines[2].y = ry + 1;

		lines[3].w = dw;
		lines[3].h = 1;
		lines[3].x = rx + 1;
		lines[3].y = ry + dh;
	}
	return lines;
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

static void midpoint_tall_ellipse_outline_algorithm_quadrant(std::vector<IPosition>& points, int cx, int cy, int rx, int ry, int qx, int qy)
{
	float x = 0;
	float y = (float)ry;

	float rx2 = float(rx * rx);
	float ry2 = float(ry * ry);
	float two_rx2 = 2 * rx2;
	float two_ry2 = 2 * ry2;

	float dx = two_ry2 * x;
	float dy = two_rx2 * y;

	// Region 1
	float d1 = ry2 - rx2 * ry + 0.25f * rx2;
	while (dx < dy)
	{
		points.push_back(IPosition(cx + qx * x, cy + qy * y));
		++x;
		dx += two_ry2;
		if (d1 >= 0)
		{
			--y;
			dy -= two_rx2;
			d1 -= dy;
		}
		d1 += dx + ry2;
	}

	// Region 2
	float nx = x + 0.5f;
	float ny = y - 1;
	float d2 = ry2 * nx * nx + rx2 * ny * ny - rx2 * ry2;
	while (y >= 0)
	{
		points.push_back(IPosition(cx + qx * x, cy + qy * y));
		--y;
		dy -= two_rx2;
		if (d2 <= 0)
		{
			++x;
			dx += two_ry2;
			d2 += dx;
		}
		d2 += rx2 - dy;
	}
}

static void midpoint_wide_ellipse_outline_algorithm_quadrant(std::vector<IPosition>& points, int cx, int cy, int rx, int ry, int qx, int qy)
{
	float x = (float)rx;
	float y = 0;

	float rx2 = float(rx * rx);
	float ry2 = float(ry * ry);
	float two_rx2 = 2 * rx2;
	float two_ry2 = 2 * ry2;

	float dx = two_ry2 * x;
	float dy = two_rx2 * y;

	// Region 1
	float d1 = rx2 - ry2 * rx + 0.25f * ry2;
	while (dy < dx)
	{
		points.push_back(IPosition(cx + qx * x, cy + qy * y));
		++y;
		dy += two_rx2;
		if (d1 >= 0)
		{
			--x;
			dx -= two_ry2;
			d1 -= dx;
		}
		d1 += dy + rx2;
	}

	// Region 2
	float nx = x - 1;
	float ny = y + 0.5f;
	float d2 = rx2 * ny * ny + ry2 * nx * nx - rx2 * ry2;
	while (x >= 0)
	{
		points.push_back(IPosition(cx + qx * x, cy + qy * y));
		--x;
		dx -= two_ry2;
		if (d2 <= 0)
		{
			++y;
			dy += two_rx2;
			d2 += dy;
		}
		d2 += ry2 - dx;
	}
}

static void midpoint_ellipse_outline_algorithm_quadrant(std::vector<IPosition>& points, int cx, int cy, int rx, int ry, int qx, int qy)
{
	if (rx < ry)
		midpoint_tall_ellipse_outline_algorithm_quadrant(points, cx, cy, rx, ry, qx, qy);
	else
		midpoint_wide_ellipse_outline_algorithm_quadrant(points, cx, cy, rx, ry, qx, qy);
}

void DiscreteEllipseOutlineInterpolator::sync_with_endpoints()
{
	points.clear();

	Position center = 0.5f * Position(start + finish);
	float rx = std::abs(start.x - center.x);
	float ry = std::abs(start.y - center.y);

	if (rx < 1.0f || ry < 1.0f)
	{
		for (float y = -ry; y <= ry; ++y)
			for (float x = -rx; x <= rx; ++x)
				points.push_back(IPosition(center.x + x, center.y + y));
	}
	else
	{
		int coffx = 1 - ((int)center.x == center.x);
		int coffy = 1 - ((int)center.y == center.y);
		midpoint_ellipse_outline_algorithm_quadrant(points, (int)center.x + coffx, (int)center.y + coffy, (int)rx, (int)ry,  1,  1);
		midpoint_ellipse_outline_algorithm_quadrant(points, (int)center.x, (int)center.y + coffy, (int)rx, (int)ry, -1,  1);
		midpoint_ellipse_outline_algorithm_quadrant(points, (int)center.x, (int)center.y, (int)rx, (int)ry, -1, -1);
		midpoint_ellipse_outline_algorithm_quadrant(points, (int)center.x + coffx, (int)center.y, (int)rx, (int)ry,  1, -1);
	}
	length = (unsigned int)points.size();
}

void DiscreteEllipseOutlineInterpolator::at(int i, int& x, int& y) const
{
	x = points[i].x;
	y = points[i].y;
}

void DiscreteRectDifference::sync_with_interpolators()
{
	first_bbox = abs_bounds(first->start, first->finish);
	second_bbox = abs_bounds(second->start, second->finish);

	if (intersection(first_bbox.x1, first_bbox.x2, second_bbox.x1, second_bbox.x2, middle_bbox.x1, middle_bbox.x2)
		&& intersection(first_bbox.y1, first_bbox.y2, second_bbox.y1, second_bbox.y2, middle_bbox.y1, middle_bbox.y2))
		common_length = middle_bbox.width_no_abs() * middle_bbox.height_no_abs();
	else
		common_length = 0;

	remove_length = first->length - common_length;
	insert_length = second->length - common_length;
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
			x = with.x1 + i % width;
			y = without.y1 + i / width;
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

void DiscreteRectDifference::remove_at(int i, int& x, int& y) const
{
	if (common_length == 0)
	{
		first->at(i, x, y);
		return;
	}
	interpolate_rect_fill_difference(i, x, y, first_bbox, middle_bbox);
}

void DiscreteRectDifference::insert_at(int i, int& x, int& y) const
{
	if (common_length == 0)
	{
		second->at(i, x, y);
		return;
	}
	interpolate_rect_fill_difference(i, x, y, second_bbox, middle_bbox);
}

static void rect_fill_modified_regions(IntBounds with, IntBounds without, IntRect& bottom, IntRect& left, IntRect& right, IntRect& top)
{
	bottom.x = with.x1;
	bottom.y = with.y1;
	bottom.w = with.width_no_abs();
	bottom.h = without.y1 - with.y1;

	left.x = with.x1;
	left.y = without.y1;
	left.w = without.x1 - with.x1;
	left.h = without.height_no_abs();

	right.x = without.x2 + 1;
	right.y = without.y1;
	right.w = with.x2 - without.x2;
	right.h = without.height_no_abs();

	top.x = with.x1;
	top.y = without.y2 + 1;
	top.w = with.width_no_abs();
	top.h = with.y2 - without.y2;
}

std::array<IntRect, 8> DiscreteRectDifference::modified_regions() const
{
	std::array<IntRect, 8> mdfd_rgns;
	rect_fill_modified_regions(first_bbox, middle_bbox, mdfd_rgns[0], mdfd_rgns[1], mdfd_rgns[2], mdfd_rgns[3]);
	rect_fill_modified_regions(second_bbox, middle_bbox, mdfd_rgns[4], mdfd_rgns[5], mdfd_rgns[6], mdfd_rgns[7]);
	return mdfd_rgns;
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
