#pragma once

#include "Widgets.h"
#include "edit/Color.h"

struct RoundRect : public WP_UnitRenderable
{
private:
	enum : size_t
	{
		VERTEX_POS,
		BORDER_COLOR,
		FILL_COLOR,
		BOTTOM_LEFT,
		RECT_SIZE,
		CORNER_RADIUS,
		THICKNESS,
	};
public:

	RoundRect(Shader* round_rect_shader);
	RoundRect(const RoundRect&) = delete;
	RoundRect(RoundRect&&) noexcept = delete;

	RGBA border_color;
	RGBA fill_color;
	float corner_radius = 0.0f;
	float thickness = 0.0f;

	void update_transform() const;
	void send_transform_under_parent(FlatTransform parent) const;
	void update_border_color() const;
	void update_fill_color() const;
	void update_corner_radius() const;
	void update_thickness() const;
	void update_all() const;
};

inline RoundRect& rr_wget(Widget& w, size_t i)
{
	return *w.get<RoundRect>(i);
}

inline const RoundRect& rr_wget(const Widget& w, size_t i)
{
	return *w.get<RoundRect>(i);
}
