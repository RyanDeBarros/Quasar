#pragma once

#include "edit/color/ColorScheme.h"
#include "Widget.h"
#include "user/Platform.h"

struct ColorSubpalette : public Widget
{
	std::shared_ptr<ColorSubscheme> subscheme = nullptr;
	int scroll_offset = 0;
	int hover_index = -1;
	WidgetPlacement hover_wp;
	int primary_index = 0;
	WidgetPlacement primary_wp;
	int alternate_index = 0;
	WidgetPlacement alternate_wp;

	ColorSubpalette(Shader* color_square_shader, Shader* outline_rect_shader);
	ColorSubpalette(const ColorSubpalette&) = delete;
	ColorSubpalette(ColorSubpalette&&) noexcept = delete;
	
	class ColorPalette& palette();
	const class ColorPalette& palette() const;

	void reload_subscheme();
	virtual void draw() override;
	void draw_selectors();
	void sync_hover_selector();
	void sync_primary_selector();
	void resync_primary_selector();
	void sync_alternate_selector();
	void resync_alternate_selector();
	void sync_with_palette();
	void setup_color_buffer(size_t i, IndexedRenderable& squares) const;
	void setup_color_buffer(size_t i);
	void process();
	void unprocess();
	bool check_primary();
	bool check_alternate();
	void set_primary_selector(int index);
	void set_alternate_selector(int index);
	
	bool get_visible_square_under_pos(Position pos, int& index) const;
	void switch_primary_and_alternate();
	void update_primary_color_in_picker() const;
	void focus(bool update_primary_color);

	void scroll_by(int delta);
	void scroll_to_view(int i);
	WidgetPlacement square_wp(int i) const;
	WidgetPlacement global_square_wp(int i) const;
	int first_square() const;
	int num_squares_visible() const;
	bool is_square_visible(int i) const;
	Position cursor_world_pos() const;

	void overwrite_current_color(RGBA color, bool update_in_picker);
	void new_color(RGBA color, bool adjacent, bool update_primary, bool create_action);
	void send_color(size_t i, RGBA color);
	void send_color(size_t i);
	void remove_square_under_cursor(bool send_vb, bool create_action);
	void clean_extra_buffer_space();
	void insert_color_at(int index, int to_primary_index, int to_alternate_index, RGBA color);
	void remove_color_at(int index, int to_primary_index, int to_alternate_index, bool send_vb);

	bool move_in_1d = true;
	int moving_color = -1;
	bool moving_color_start = false;

	void begin_moving_color_under_cursor(bool move_in_1d);
	void stop_moving_color();
	bool is_moving_a_color() const { return moving_color >= 0; }
	void move_color();

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

	std::shared_ptr<ColorScheme> scheme;
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

	FlatTransform gui_transform;
	bool imgui_editing;
	
	bool renaming_subpalette = false;
	bool renaming_subpalette_start = false;
	char rename_buf[ColorSubscheme::MAX_NAME_LENGTH] = "";

public:
	ColorSubpalette& get_subpalette(size_t pos);
	const ColorSubpalette& get_subpalette(size_t pos) const;
	ColorSubpalette& current_subpalette();
	const ColorSubpalette& current_subpalette() const;
	std::shared_ptr<ColorSubpalette> subpalette_ref(ColorSubpalette* subpalette) const;
	std::shared_ptr<ColorSubpalette> subpalette_ref(size_t pos) const;
	std::shared_ptr<ColorSubpalette> current_subpalette_ref() const;
	size_t subpalette_index_in_widget(size_t pos) const;

	static inline const float SQUARE_SEP = 28;
	static inline const float SQUARE_SIZE = 24;

	bool clear_history_on_import = false; // SETTINGS

private:
	float grid_offset_y = 0;
	int _col_count = 8;
	int _row_count = 8;

public:
	int row_count() const { return _row_count; }
	int col_count() const { return _col_count; }
	
	const std::function<void(RGBA)>* primary_color_update;
	const std::function<RGBA()>* get_picker_rgba;

	ColorPalette(glm::mat3* vp, MouseButtonHandler& parent_mb_handler, KeyHandler& parent_key_handler, ScrollHandler& parent_scroll_handler,
		const std::function<void(RGBA)>* primary_color_update, const std::function<RGBA()>* get_picker_rgba);
	ColorPalette(const ColorPalette&) = delete;
	ColorPalette(ColorPalette&&) noexcept = delete;
	~ColorPalette();

	virtual void draw() override;
	void process();
	void send_vp();
	void import_color_scheme(const std::shared_ptr<ColorScheme>& color_scheme, bool create_action);
	void import_color_scheme(std::shared_ptr<ColorScheme>&& color_scheme, bool create_action);
	void assign_color_subscheme(size_t pos, const std::shared_ptr<ColorSubscheme>& subscheme, bool create_action);
	void assign_color_subscheme(size_t pos, std::shared_ptr<ColorSubscheme>&& subscheme, bool create_action);
	void insert_subpalette(size_t pos, const std::shared_ptr<ColorSubpalette>& subpalette);
	void insert_subpalette(size_t pos, std::shared_ptr<ColorSubpalette>&& subpalette);
	void new_subpalette();
	void delete_subpalette(size_t pos);
	void switch_to_subpalette(size_t pos, bool update_primary_color);
	void switch_to_subpalette(ColorSubpalette* subpalette, bool update_primary_color);
	size_t num_subpalettes() const;
	void set_size(Scale pos, bool sync);
	Scale minimum_display() const;

private:
	void render_imgui();
	void connect_input_handlers();
	void initialize_widget();
	void import_color_scheme();
	void sync_widget_with_vp();
	void set_grid_metrics(int col_count, int row_count, bool sync);

	bool cursor_in_bkg() const;
	glm::mat3 grid_vp() const;
	float subpalette_pos_y() const;
	float absolute_y_off_bkg_top(float y) const;
	float absolute_y_off_bkg_bot(float y) const;

public:
	enum : size_t
	{
		BACKGROUND,
		BLACK_GRID,
		BUTTON_OVERWRITE_COLOR,
		BUTTON_INSERT_NEW_COLOR,
		BUTTON_SUBPALETTE_NEW,
		BUTTON_SUBPALETTE_RENAME,
		BUTTON_SUBPALETTE_DELETE,
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
