#pragma once

#include "edit/color/ColorScheme.h"
#include "Widget.h"
#include "user/Platform.h"

struct ColorSubpalette : public Widget
{

};

inline ColorSubpalette& cspl_wget(Widget& w, size_t i)
{
	return *w.get<ColorSubpalette>(i);
}

inline const ColorSubpalette& cspl_wget(const Widget& w, size_t i)
{
	return *w.get<ColorSubpalette>(i);
}

class ColorPalette : public Widget
{
	ColorScheme scheme;
	glm::mat3* vp;

	MouseButtonHandler& parent_mb_handler;
	MouseButtonHandler mb_handler;
	KeyHandler& parent_key_handler;
	KeyHandler key_handler;

public:
	ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler);
	ColorPalette(const ColorPalette&) = delete;
	ColorPalette(ColorPalette&&) noexcept = delete;
	~ColorPalette();

	void draw() override;
	void send_vp();
	void import_color_scheme(const ColorScheme& color_scheme);
	void import_color_scheme(ColorScheme&& color_scheme);

private:
	void initialize_widget();
	void connect_input_handlers();

	void import_color_scheme();

	void sync_widget_with_vp();

	enum : size_t
	{
		PRIMARY_SELECTOR,
		ALTERNATE_SELECTOR,
		HOVER_SELECTOR,
		SUBPALETTE_1,
		SUBPALETTE_2,
		SUBPALETTE_3,
		SUBPALETTE_4,
		SUBPALETTE_5,
		SUBPALETTE_6,
		SUBPALETTE_7,
		SUBPALETTE_8,
		SUBPALETTE_9,
		SUBPALETTE_10,
		SUBPALETTE_11,
		SUBPALETTE_12,
		SUBPALETTE_13,
		SUBPALETTE_14,
		SUBPALETTE_15,
		SUBPALETTE_16,
		_W_COUNT // TODO add un-user-editable FREQUENT_SUBPALETTE for most frequently used colors.
	};

};

inline ColorPalette& cpl_wget(Widget& w, size_t i)
{
	return *w.get<ColorPalette>(i);
}

inline const ColorPalette& cpl_wget(const Widget& w, size_t i)
{
	return *w.get<ColorPalette>(i);
}
