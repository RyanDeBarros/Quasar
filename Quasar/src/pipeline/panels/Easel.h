#pragma once

#include <array>

#include "Panel.h"
#include "BrushHeader.h"
#include "user/Platform.h"
#include "../render/Gridlines.h"
#include "../widgets/Widget.h"
#include "edit/image/Image.h"
#include "edit/image/PaintActions.h"
#include "variety/History.h"

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
	bool pipette_ready = false;

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

	void set_image(const std::shared_ptr<Image>& img);
	void set_image(std::shared_ptr<Image>&& img);
	void sync_checkerboard_with_image();
	void sync_brush_preview_with_image();
	void sync_gridlines_with_image();
	void sync_transform();
	void sync_gfx_with_image();

	void create_checkerboard_image();
	void sync_checkerboard_colors() const;
	void sync_checkerboard_texture() const;
	void set_checkerboard_uv_size(float width, float height) const;

	void update_brush_tool();
	void update_brush_tip();
	
	IPosition brush_pos_under_cursor() const;
	void hover_pixel_under_cursor();
	void unhover();
	void hover_pixel_at(Position pos);
	
	Position pixel_position(IPosition pos);
	Position pixel_position(int x, int y);
	RGBA applied_color() const;
	RGBA color_under_cursor() const;
	PixelRGBA pixel_color_at(IPosition pos) const;
	PixelRGBA pixel_color_at(int x, int y) const;
	
	void set_cursor_color(RGBA color);
	void set_primary_color(RGBA color);
	void set_alternate_color(RGBA color);

	void cursor_press(MouseButton button);
	void cursor_release();
	bool cursor_cancel();

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

	enum : size_t
	{
		CHECKERBOARD,
		CURSOR_PENCIL,
		CURSOR_PEN,
		CURSOR_ERASER,
		CURSOR_SELECT,
		BRUSH_PREVIEW,
		ERASER_PREVIEW,
		SELECT_PREVIEW,
		SPRITE, // LATER SPRITE_START
		_W_COUNT
	};

	struct Brush
	{
		bool brushing = false;
		BrushTip tip;
		BrushTool tool;
		IPosition starting_pos = { -1, -1 };
		IPosition brush_pos = { -1, -1 };
		IntBounds brushing_bbox = { -1, -1, -1, -1 };
		bool show_preview = false;
		std::shared_ptr<Image> preview_image;
		std::shared_ptr<Image> eraser_preview_image;
		std::unordered_map<IPosition, std::pair<PixelRGBA, PixelRGBA>> painted_colors;
		std::unordered_map<IPosition, PixelRGBA> preview_positions;

		enum
		{
			PAINTED_COLORS,
			PREVIEW_POSITIONS
		} storage_mode = PAINTED_COLORS;

		struct
		{
			DiscreteLineInterpolator line = {};
			DiscreteRectFillInterpolator rect_fill = {};
		} interps;
		
		struct
		{
			DiscreteRectFillDifference rect_fill = {};
		} interp_diffs;

		enum
		{
			LINE,
			RECT_FILL,
		} preview_mode = LINE;

		void reset();
	} binfo;
};

struct Easel : public Panel
{
	Shader color_square_shader;
	Widget widget;
	glm::mat3 vp;

	MouseButtonHandler mb_handler;
	KeyHandler key_handler;
	ScrollHandler scroll_handler;

	Easel();
	Easel(const Easel&) = delete;
	Easel(Easel&&) noexcept = delete;

	virtual void initialize() override;

private:
	void initialize_widget();
	void connect_input_handlers();

public:
	virtual void draw() override;
	virtual void _send_view() override;

	void process();
	void sync_widget();

private:
	void sync_ur(size_t subw);

public:
	void sync_canvas_transform();

	const Image* canvas_image() const;
	Image* canvas_image();

	bool minor_gridlines_are_visible() const;
	void set_minor_gridlines_visibility(bool visible);
	bool major_gridlines_are_visible() const;
	void set_major_gridlines_visibility(bool visible);

	struct
	{
		Position initial_cursor_pos{};
		Position initial_canvas_pos{};
		bool panning = false;
	private:
		friend Easel;
		WindowHandle wh;
	} panning_info;
	
	struct
	{
		// SETTINGS (only some of them?)
		constexpr static float initial = 0.5f;
		constexpr static float in_min = 0.01f;
		constexpr static float in_max = 100.0f;
		constexpr static float factor = 1.5f;
		constexpr static float factor_shift = 1.05f;
		float zoom = initial;
	} zoom_info;

	void begin_panning();
	void end_panning();
	void cancel_panning();
	void update_panning();
	void zoom_by(float zoom);
	void reset_camera();

	bool image_edit_perf_mode = false; // SETTINGS this mode prioritizes performance over memory usage in action history. good for high-end computers, bad for low-end computers.

	void flip_image_horizontally();
	void flip_image_vertically();
	void rotate_image_90();
	void rotate_image_180();
	void rotate_image_270();

	Canvas& canvas() { return *widget.get<Canvas>(CANVAS); }
	const Canvas& canvas() const { return *widget.get<Canvas>(CANVAS); }

	enum : size_t
	{
		BACKGROUND,
		CANVAS,
		_W_COUNT
	};
};
