#pragma once

#include "Panel.h"
#include "../render/FlatSprite.h"
#include "../render/Shader.h"
#include "../widgets/Widget.h"

struct Palette : public Panel
{
	Shader bkg_shader;
	glm::mat3 vp;
	Widget widget;

	std::function<void(RGBA)> update_pri_color;
	std::function<void(RGBA)> update_alt_color;
	std::function<RGBA()> get_picker_pri_rgba;
	std::function<RGBA()> get_picker_alt_rgba;
	std::function<bool()> use_primary;
	std::function<bool()> use_alternate;
	std::function<void()> swap_picker_colors;

	Palette();
	Palette(const Palette&) = delete;
	Palette(Palette&&) noexcept = delete;

	virtual void _send_view() override;
	virtual void draw() override;
	void render_widget();
	virtual Scale minimum_screen_display() const override;

	void new_color();
	void overwrite_color();
	void delete_color();
	void new_subpalette();
	void rename_subpalette();
	void delete_subpalette();

private:
	void initialize_widget();
	void sync_widget();

public:
	enum : size_t
	{
		BACKGROUND,
		COLOR_PICKER,
		COLOR_PALETTE,
		_W_COUNT
	};
};
