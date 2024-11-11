#pragma once

#include "edit/color/ColorScheme.h"
#include "Widget.h"
#include "user/Platform.h"

struct ColorSubpalette : public Widget
{
	std::shared_ptr<ColorSubscheme> subscheme = nullptr;
	int scroll_offset = 0;
	int current_hover_index = -1;
	WidgetPlacement hover_wp;
	int current_primary_index = 0;
	WidgetPlacement primary_wp;
	int current_alternate_index = 0;
	WidgetPlacement alternate_wp;

	ColorSubpalette(Shader* color_square_shader, Shader* outline_rect_shader);
	ColorSubpalette(const ColorSubpalette&) = delete;
	ColorSubpalette(ColorSubpalette&&) noexcept = delete;
	
	void reload_subscheme();
	virtual void draw() override;
	void draw_selectors();
	void sync_hover_selector();
	void sync_primary_selector();
	void resync_primary_selector();
	void sync_alternate_selector();
	void resync_alternate_selector();
	void sync_with_palette();
	void process();
	bool check_primary();
	bool check_alternate();
	
	bool get_visible_square_under_pos(Position pos, int& index) const;

	void scroll_by(int delta);
	WidgetPlacement square_wp(int i) const;
	WidgetPlacement global_square_wp(int i) const;
	int first_square() const;
	int num_squares_visible() const;
	Position cursor_world_pos() const;

	enum : size_t
	{
		SQUARES,
		PRIMARY_SELECTOR_B,
		PRIMARY_SELECTOR_F,
		ALTERNATE_SELECTOR_B,
		ALTERNATE_SELECTOR_F,
		HOVER_SELECTOR,
		_W_COUNT
	};
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
	friend ColorSubpalette;

	ColorScheme scheme;
	size_t current_subscheme = 0;

	Shader color_square_shader, grid_shader, outline_rect_shader, round_rect_shader;
	glm::mat3* vp;

	MouseButtonHandler& parent_mb_handler;
	MouseButtonHandler mb_handler;
	KeyHandler& parent_key_handler;
	KeyHandler key_handler;
	ScrollHandler& parent_scroll_handler;
	ScrollHandler scroll_handler;

	float cached_scale1d = 0.0f;
	float scroll_backlog = 0.0f;

	ColorSubpalette& get_subpalette(size_t pos);
	const ColorSubpalette& get_subpalette(size_t pos) const;
	ColorSubpalette& current_subpalette();
	const ColorSubpalette& current_subpalette() const;
	size_t subpalette_index_in_widget(size_t pos) const;

public:
	static const int COL_COUNT = 8;
	static const int ROW_COUNT = 8;
	static inline const float SQUARE_SEP = 28.0f;
	static inline const float SQUARE_SIZE = 24;
	static inline const float GRID_OFFSET_Y = -60;
	static inline const WidgetPlacement GRID_WP = { { { 0, GRID_OFFSET_Y }, Scale(COL_COUNT * SQUARE_SEP) } };

	const std::function<void(RGBA)>* primary_color_update;

	ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler, ScrollHandler& parent_scroll_handler, const std::function<void(RGBA)>* primary_color_update);
	ColorPalette(const ColorPalette&) = delete;
	ColorPalette(ColorPalette&&) noexcept = delete;
	~ColorPalette();

	virtual void draw() override;
	void send_vp();
	void import_color_scheme(const ColorScheme& color_scheme);
	void import_color_scheme(ColorScheme&& color_scheme);
	void new_subpalette();
	void delete_subpalette(size_t pos);
	size_t num_subpalettes() const;
	void set_size(Scale pos, bool sync = false);

private:
	void connect_input_handlers();
	void initialize_widget();
	void import_color_scheme();
	void sync_widget_with_vp();

	enum : size_t
	{
		BACKGROUND,
		BLACK_GRID,
		SUBPALETTE_START
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
