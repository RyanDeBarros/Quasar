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

	const RoundRect& update_transform() const;
	const RoundRect& update_transform(const WidgetPlacement& wp) const;
	const RoundRect& update_border_color() const;
	const RoundRect& update_fill_color() const;
	const RoundRect& update_corner_radius() const;
	const RoundRect& update_thickness() const;
	const RoundRect& update_all() const;
	void send_vp(const glm::mat3& vp) const;
};

inline RoundRect& rr_wget(Widget& w, size_t i)
{
	return *w.get<RoundRect>(i);
}

inline const RoundRect& rr_wget(const Widget& w, size_t i)
{
	return *w.get<RoundRect>(i);
}
