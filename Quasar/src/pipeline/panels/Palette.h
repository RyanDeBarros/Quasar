#pragma once

#include "Panel.h"
#include "user/Platform.h"
#include "../render/FlatSprite.h"
#include "../render/Shader.h"
#include "../widgets/Widget.h"

struct PalettePanel : public Panel
{
	Shader bkg_shader;
	glm::mat3 vp;
	Widget widget;

	KeyHandler key_handler;
	MouseButtonHandler mb_handler;
	ScrollHandler scroll_handler;

	std::function<void(RGBA)> update_pri_color;
	std::function<void(RGBA)> update_alt_color;
	std::function<RGBA()> get_picker_pri_rgba;
	std::function<RGBA()> get_picker_alt_rgba;
	std::function<bool()> use_primary;
	std::function<bool()> use_alternate;
	std::function<void()> swap_picker_colors;
	std::function<void(RGBA)> emit_modified_primary;
	std::function<void(RGBA)> emit_modified_alternate;

	PalettePanel();
	PalettePanel(const PalettePanel&) = delete;
	PalettePanel(PalettePanel&&) noexcept = delete;

	virtual void _send_view() override;
	virtual void draw() override;
	void process();
	void render_widget();
	virtual Scale minimum_screen_display() const override;

	void new_color();
	void overwrite_color();
	void delete_color();
	void new_subpalette();
	void rename_subpalette();
	void delete_subpalette();

	void set_pri_color(RGBA color);
	void set_alt_color(RGBA color);

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
