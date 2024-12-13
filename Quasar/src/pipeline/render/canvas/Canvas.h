#pragma once

#include "../../widgets/Widget.h"
#include "user/Platform.h"
#include "BrushInfo.h"
#include "BrushAttributes.h"
#include "Gridlines.h"

struct Canvas : public Widget
{
	friend struct Easel;
	Shader sprite_shader; // LATER have one shader for internal sprites like checkerboard/cursors/previews, and a second for the actual sprites (used for multiple layers/frames).
	RGBA checker1, checker2;
	Gridlines minor_gridlines;
	Gridlines major_gridlines;

	std::shared_ptr<Image> image;

	bool visible = false;
	bool cursor_in_canvas = false;
	enum class CursorState
	{
		UP,
		DOWN_PRIMARY,
		DOWN_ALTERNATE
	} cursor_state = CursorState::UP;

	Buffer dot_cursor_buf;
	std::shared_ptr<Cursor> dot_cursor;
	WindowHandle dot_cursor_wh, pipette_cursor_wh;
	std::shared_ptr<Image> eraser_cursor_img;

private:
	glm::vec2 checker_size_inv = glm::vec2(1.0f / 16.0f);
public:
	glm::ivec2 get_checker_size() const { return { roundf(1.0f / checker_size_inv.x), roundf(1.0f / checker_size_inv.y) }; }
	void set_checker_size(glm::ivec2 checker_size);

	Canvas(Shader* cursor_shader);
	Canvas(const Canvas&) = delete;
	Canvas(Canvas&&) noexcept = delete;
	~Canvas();

	void initialize_widget(Shader* cursor_shader);
	void initialize_dot_cursor();
	void initialize_eraser_cursor_img();

	virtual void draw() override;
	void draw_cursor();
	void send_vp(const glm::mat3& vp);
	void sync_cursor_with_widget();
	void sync_ur(size_t subw);
	void process();

	void set_image(const std::shared_ptr<Image>& img);
	void set_image(std::shared_ptr<Image>&& img);
	void sync_sprite_with_image();
	void sync_checkerboard_with_image();
	void sync_brush_preview_with_image();
	void sync_smants_with_image();
	void sync_gridlines_with_image();
	void sync_transform();
	void sync_gfx_with_image();

	void create_checkerboard_image();
	void sync_checkerboard_colors() const;
	void sync_checkerboard_texture() const;
	void set_checkerboard_uv_size(float width, float height) const;

	void update_brush_tool_and_tip();

	IPosition brush_pos_under_cursor() const;
	bool brush_pos_in_image_bounds(int x, int y) const;
	void hover_pixel_under_cursor();
	void unhover();
	void hover_pixel_at(Position pos);

	Position pixel_position(IPosition pos);
	Position pixel_position(int x, int y);
	RGBA applied_color() const;
	PixelRGBA applied_pencil_rgba() const;
	PixelRGBA applied_pen_rgba() const;
	RGBA color_under_cursor() const;

	void set_cursor_color(RGBA color);
	void set_primary_color(RGBA color);
	void set_alternate_color(RGBA color);

	void cursor_press(MouseButton button);
	bool cursor_enter();
	void cursor_release(MouseButton button);
	bool cursor_cancel();
	void full_brush_reset();

	RGBA primary_color, alternate_color;
	PixelRGBA pric_pxs = {};
	PixelRGBA altc_pxs = {};
	PixelRGBA pric_pen_pxs = {};
	PixelRGBA altc_pen_pxs = {};
	void(*brush_under_tool_and_tip)(Canvas&, int, int);

	void brush(int x, int y);
	void brush_start(int x, int y);
	void brush_submit();
	void brush_cancel();
	void brush_move_to(IPosition pos);

	void select_all();
	bool deselect_all();
	void fill_selection_primary();
	void fill_selection_alternate();
	bool delete_selection();

	void batch_move_selection_to(float dx, float dy);
	void batch_move_selection_to(int dx, int dy);
	bool batch_move_selection_start();
	void batch_move_selection_submit();
	void batch_move_selection_cancel();
	void transition_moving_selection_to_overwrite();
	void transition_moving_selection_to_blend();

public:
	enum : size_t
	{
		CHECKERBOARD,
		CURSOR_PENCIL,
		CURSOR_PEN,
		CURSOR_ERASER,
		CURSOR_SELECT,
		BRUSH_PREVIEW,
		ERASER_PREVIEW,
		SELECTION,
		SELECTION_PREVIEW,
		SELECTION_SUBIMAGE,
		SPRITE, // LATER SPRITE_START
		_W_COUNT
	};

	BrushInfo binfo;
	BrushAttributes battr;
};
