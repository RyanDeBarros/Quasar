#pragma once

#include "Widget.h"
#include "edit/color/Color.h"

struct RoundRect : public W_UnitRenderable
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

	virtual void draw() override;

	const RoundRect& update_transform() const;
	const RoundRect& update_border_color() const;
	const RoundRect& update_fill_color() const;
	const RoundRect& update_corner_radius(float scale = 1.0f) const;
	const RoundRect& update_thickness(float scale = 1.0f) const;
	const RoundRect& update_all() const;
	void send_buffer() const;
};

inline RoundRect& rr_wget(Widget& w, size_t i)
{
	return *w.get<RoundRect>(i);
}

inline const RoundRect& rr_wget(const Widget& w, size_t i)
{
	return *w.get<RoundRect>(i);
}
