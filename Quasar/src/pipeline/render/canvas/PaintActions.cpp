#include "PaintActions.h"

#include "SelectionMants.h"
#include "../../panels/Easel.h"
#include "user/Machine.h"
#include "Canvas.h"
#include "../FlatSprite.h"

// LATER since there's bounds checking here, not really necessary to do it elsewhere.

void buffer_set_pixel_color(const Buffer& buf, int x, int y, PixelRGBA c)
{
	if (buf.bbox().contains({ x, y }))
	{
		for (CHPP i = 0; i < buf.chpp; ++i)
			buf.pos(x, y)[i] = c[i];
	}
}

void buffer_set_pixel_alpha(const Buffer& buf, int x, int y, int alpha)
{
	if (buf.bbox().contains({ x, y }))
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

static void fill_horizontal_span(std::vector<IPosition>& points, int cx, int dx, int y, int& y_record)
{
	if (y != y_record)
	{
		y_record = y;
		int start = std::min(0, dx);
		int end = std::max(0, dx);
		while (start <= end)
		{
			points.push_back(IPosition(cx + start, y));
			++start;
		}
	}
}

static void midpoint_tall_ellipse_fill_algorithm_quadrant(std::vector<IPosition>& points, int cx, int cy, int rx, int ry, int qx, int qy)
{
	float x = 0;
	float y = (float)ry;
	int y_record = INT_MAX;

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
		fill_horizontal_span(points, cx, int(qx * x), int(cy + qy * y), y_record);
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
		fill_horizontal_span(points, cx, int(qx * x), int(cy + qy * y), y_record);
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

static void midpoint_wide_ellipse_fill_algorithm_quadrant(std::vector<IPosition>& points, int cx, int cy, int rx, int ry, int qx, int qy)
{
	float x = (float)rx;
	float y = 0;
	int y_record = INT_MIN;

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
		fill_horizontal_span(points, cx, int(qx * x), int(cy + qy * y), y_record);
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
		fill_horizontal_span(points, cx, int(qx * x), int(cy + qy * y), y_record);
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

static void midpoint_ellipse_fill_algorithm_quadrant(std::vector<IPosition>& points, int cx, int cy, int rx, int ry, int qx, int qy)
{
	if (rx < ry)
		midpoint_tall_ellipse_fill_algorithm_quadrant(points, cx, cy, rx, ry, qx, qy);
	else
		midpoint_wide_ellipse_fill_algorithm_quadrant(points, cx, cy, rx, ry, qx, qy);
}

void DiscreteEllipseFillInterpolator::sync_with_endpoints()
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
		midpoint_ellipse_fill_algorithm_quadrant(points, (int)center.x + coffx, (int)center.y + coffy, (int)rx, (int)ry, 1, 1);
		midpoint_ellipse_fill_algorithm_quadrant(points, (int)center.x, (int)center.y + coffy, (int)rx, (int)ry, -1, 1);
		midpoint_ellipse_fill_algorithm_quadrant(points, (int)center.x, (int)center.y, (int)rx, (int)ry, -1, -1);
		midpoint_ellipse_fill_algorithm_quadrant(points, (int)center.x + coffx, (int)center.y, (int)rx, (int)ry, 1, -1);
	}

	length = (unsigned int)points.size();
}

void DiscreteEllipseFillInterpolator::at(int i, int& x, int& y) const
{
	x = points[i].x;
	y = points[i].y;
}

OneColorAction::OneColorAction(const std::shared_ptr<Image>& image, PixelRGBA color, IntBounds bbox, std::unordered_map<IPosition, PixelRGBA>&& painted_colors)
	: image(image), color(color), bbox(bbox), painted_colors(std::move(painted_colors))
{
	weight = sizeof(OneColorAction) + this->painted_colors.size() * (sizeof(IPosition) + sizeof(PixelRGBA));
}

void OneColorAction::forward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, color);
		img->update_subtexture(bbox);
	}
}

void OneColorAction::backward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second);
		img->update_subtexture(bbox);
	}
}

TwoColorAction::TwoColorAction(const std::shared_ptr<Image>& image, IntBounds bbox, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors)
	: image(image), bbox(bbox), painted_colors(std::move(painted_colors))
{
	weight = sizeof(TwoColorAction) + this->painted_colors.size() * (sizeof(IPosition) + 2 * sizeof(PixelRGBA));
}

void TwoColorAction::forward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second.first);
		img->update_subtexture(bbox);
	}
}

void TwoColorAction::backward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second.second);
		img->update_subtexture(bbox);
	}
}

TwoColorMoveAction::TwoColorMoveAction(const std::shared_ptr<Image>& image, IntBounds bbox_remove, IntBounds bbox_add, std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>>&& painted_colors)
	: image(image), bbox_remove(bbox_remove), bbox_add(bbox_add), painted_colors(std::move(painted_colors))
{
	weight = sizeof(TwoColorAction) + this->painted_colors.size() * (sizeof(IPosition) + 2 * sizeof(PixelRGBA));
}

void TwoColorMoveAction::forward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second.first);
		img->update_subtexture(bbox_remove);
		img->update_subtexture(bbox_add);
	}
}

void TwoColorMoveAction::backward()
{
	if (painted_colors.empty())
		return;
	if (auto img = image.lock())
	{
		Buffer& buf = img->buf;
		for (const auto& iter : painted_colors)
			buffer_set_pixel_color(buf, iter.first.x, iter.first.y, iter.second.second);
		img->update_subtexture(bbox_remove);
		img->update_subtexture(bbox_add);
	}
}

SmantsModifyAction::SmantsModifyAction(SelectionMants* smants, IntBounds bbox, std::unordered_set<IPosition>&& remove_points, std::unordered_set<IPosition>&& add_points)
	: smants(smants), bbox(bbox), remove_points(std::move(remove_points)), add_points(std::move(add_points))
{
	weight = sizeof(SmantsModifyAction) + (this->remove_points.size() + this->add_points.size()) * sizeof(IPosition);
}

void SmantsModifyAction::forward()
{
	for (auto iter = remove_points.begin(); iter != remove_points.end(); ++iter)
		smants->remove(*iter);
	for (auto iter = add_points.begin(); iter != add_points.end(); ++iter)
		smants->add(*iter);
	smants->send_buffer(bbox);
}

void SmantsModifyAction::backward()
{
	for (auto iter = add_points.begin(); iter != add_points.end(); ++iter)
		smants->remove(*iter);
	for (auto iter = remove_points.begin(); iter != remove_points.end(); ++iter)
		smants->add(*iter);
	smants->send_buffer(bbox);
}

SmantsMoveAction::SmantsMoveAction(IPosition delta, std::unordered_set<IPosition>&& premove_points)
	: delta(delta), premove_points(std::move(premove_points))
{
	weight = sizeof(SmantsMoveAction) + this->premove_points.size() * sizeof(IPosition);
}

void SmantsMoveAction::forward()
{
	BrushInfo& binfo = MEasel->canvas().binfo;
	binfo.smants->send_buffer(binfo.smants->clear());
	IntBounds bbox = IntBounds::INVALID;
	for (IPosition pos : premove_points)
	{
		pos += delta;
		if (binfo.smants->add(pos))
			update_bbox(bbox, pos.x, pos.y);
	}
	binfo.move_selpxs_offset += delta;
	binfo.smants->send_buffer(bbox);
}

void SmantsMoveAction::backward()
{
	BrushInfo& binfo = MEasel->canvas().binfo;
	binfo.smants->send_buffer(binfo.smants->clear());
	IntBounds bbox = IntBounds::INVALID;
	for (IPosition pos : premove_points)
	{
		if (binfo.smants->add(pos))
			update_bbox(bbox, pos.x, pos.y);
	}
	binfo.move_selpxs_offset -= delta;
	binfo.smants->send_buffer(bbox);
}

MoveSubimgAction::MoveSubimgAction(bool from_image)
	: from_image(from_image)
{
	weight = sizeof(MoveSubimgAction);
	BrushInfo& binfo = MEasel->canvas().binfo;
	binfo.state = BrushInfo::State::SUBIMG_READY;
	final = binfo.move_selpxs_offset;
	initial = binfo.starting_move_selpxs_offset;
	auto& points = binfo.smants->get_points();
	auto delta = final - initial;
	for (auto iter = points.begin(); iter != points.end(); ++iter)
	{
		IPosition prepos = *iter - delta;
		auto pxit = binfo.raw_selection_pixels.find(prepos);
		if (pxit != binfo.raw_selection_pixels.end())
			premove_pixels[prepos] = pxit->second;
		else
			premove_pixels[prepos] = {};
	}
	weight += premove_pixels.size() * (sizeof(IPosition) + sizeof(PixelRGBA));
	if (from_image)
		binfo.smants->flip_direction();
}

void MoveSubimgAction::update()
{
	BrushInfo& binfo = MEasel->canvas().binfo;
	final = binfo.move_selpxs_offset;
	bool prev_from_image = from_image;
	from_image = false;
	forward();
	from_image = prev_from_image;
}

void MoveSubimgAction::forward()
{
	Canvas& canvas = MEasel->canvas();
	BrushInfo& binfo = canvas.binfo;
	binfo.state = BrushInfo::State::SUBIMG_READY;

	binfo.smants->send_buffer(binfo.smants->clear());
	IntBounds bbox = IntBounds::INVALID;
	binfo.raw_selection_pixels.clear();
	Buffer& imgbuf = canvas.image->buf;
	Buffer& selbuf = binfo.selection_subimage->buf;
	IntBounds img_bounds = selbuf.bbox();
	IntBounds img_bbox = IntBounds::INVALID;
	auto delta = final - initial;
	for (auto iter = premove_pixels.begin(); iter != premove_pixels.end(); ++iter)
	{
		IPosition pos = iter->first + delta;
		if (binfo.smants->add(pos))
			update_bbox(bbox, pos.x, pos.y);
		binfo.raw_selection_pixels[iter->first] = iter->second;
		if (from_image && img_bounds.contains(iter->first))
		{
			buffer_set_pixel_color(selbuf, iter->first.x, iter->first.y, binfo.apply_selection_with_pencil ? iter->second : iter->second.no_alpha_equivalent());
			buffer_set_pixel_alpha(imgbuf, iter->first.x, iter->first.y, 0);
			update_bbox(img_bbox, iter->first.x, iter->first.y);
		}
	}
	binfo.smants->send_buffer(bbox);
	if (from_image)
	{
		canvas.image->update_subtexture(img_bbox);
		binfo.selection_subimage->update_subtexture(img_bbox);
	}

	binfo.sel_subimg_sprite->self.transform.position = final;
	binfo.sel_subimg_sprite->update_transform().ur->send_buffer();
	binfo.move_selpxs_offset = final;
	if (from_image)
		binfo.smants->flip_direction();
}

void MoveSubimgAction::backward()
{
	Canvas& canvas = MEasel->canvas();
	BrushInfo& binfo = canvas.binfo;
	if (from_image)
		binfo.state = BrushInfo::State::NEUTRAL;
	else
		binfo.state = BrushInfo::State::SUBIMG_READY;

	binfo.smants->send_buffer(binfo.smants->clear());
	IntBounds bbox = IntBounds::INVALID;
	binfo.raw_selection_pixels.clear();
	Buffer& imgbuf = canvas.image->buf;
	Buffer& selbuf = binfo.selection_subimage->buf;
	IntBounds img_bounds = selbuf.bbox();
	IntBounds img_bbox = IntBounds::INVALID;
	if (!from_image)
	{
		for (auto iter = premove_pixels.begin(); iter != premove_pixels.end(); ++iter)
		{
			if (binfo.smants->add(iter->first))
				update_bbox(bbox, iter->first.x, iter->first.y);
			binfo.raw_selection_pixels[iter->first] = iter->second;
		}
	}
	else
	{
		for (auto iter = premove_pixels.begin(); iter != premove_pixels.end(); ++iter)
		{
			if (img_bounds.contains(iter->first))
			{
				buffer_set_pixel_color(imgbuf, iter->first.x, iter->first.y, iter->second);
				buffer_set_pixel_alpha(selbuf, iter->first.x, iter->first.y, 0);
				update_bbox(img_bbox, iter->first.x, iter->first.y);
			}
		}
	}
	binfo.smants->send_buffer(bbox);
	if (from_image)
	{
		canvas.image->update_subtexture(img_bbox);
		binfo.selection_subimage->update_subtexture(img_bbox);
	}

	binfo.sel_subimg_sprite->self.transform.position = initial;
	binfo.sel_subimg_sprite->update_transform().ur->send_buffer();
	binfo.move_selpxs_offset = initial;
	if (from_image)
		binfo.smants->flip_direction();
}
