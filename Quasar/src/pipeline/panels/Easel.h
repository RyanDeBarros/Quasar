#pragma once

#include <array>

#include "Panel.h"
#include "user/Platform.h"
#include "../render/Shader.h"
#include "../widgets/Widget.h"
#include "edit/color/Color.h"
#include "edit/image/Image.h"

struct Gridlines
{
	unsigned short width = 0, height = 0;
	GLuint vao = 0, vb = 0;
	Shader shader;
	GLfloat* varr = nullptr;
	glm::vec2 line_spacing = { 1.0f, 1.0f };
	float line_width = 1.0f; // SETTINGS
	float self_intersection_threshold = 1.0f; // SETTINGS

	GLint* arrays_firsts = nullptr;
	GLsizei* arrays_counts = nullptr;

private:
	bool _visible = false;
	mutable bool _nonobstructing = true;
	mutable bool _send_flat_transform = false;
	mutable bool _sync_with_image = false;

public:
	Gridlines();
	Gridlines(const Gridlines&) = delete;
	Gridlines(Gridlines&&) noexcept = delete;
	~Gridlines();

	void resize_grid(Scale scale);
	void update_scale(Scale scale) const;
	void draw() const;

	bool visible() const { return _visible; }
	unsigned short num_cols() const;
	unsigned short num_rows() const;
	GLsizei num_quads() const { return num_rows() + num_cols(); }
	GLsizei num_vertices() const { return num_quads() * 4; }

	void set_color(ColorFrame color) const;

	void send_buffer() const;
	void send_flat_transform(FlatTransform canvas_transform) const;
	void sync_with_image(const Buffer& buf, Scale canvas_scale);

	void set_visible(bool visible, const struct Canvas& canvas);
};

struct Canvas : public Widget
{
	RGBA checker1, checker2;
	Gridlines minor_gridlines;
	Gridlines major_gridlines;

	bool visible = false;
	bool cursor_in_canvas = false;
	enum class CursorState
	{
		UP,
		DOWN_PRIMARY,
		DOWN_ALTERNATE
	} cursor_state = CursorState::UP;
	IPosition brush_pos = { -1, -1 };
	bool brushing = false;

private:
	glm::vec2 checker_size_inv = glm::vec2(1.0f / 16.0f);
public:
	glm::ivec2 get_checker_size() const { return { roundf(1.0f / checker_size_inv.x), roundf(1.0f / checker_size_inv.y) }; }
	void set_checker_size(glm::ivec2 checker_size);

	Canvas(Shader* sprite_shader, Shader* cursor_shader);
	Canvas(const Canvas&) = delete;
	Canvas(Canvas&&) noexcept = delete;

	virtual void draw() override;
	void sync_cursor_with_widget();
	void sync_ur(size_t subw);

	const Image* image() const;
	Image* image();
	void set_image(const std::shared_ptr<Image>& img);
	void set_image(std::shared_ptr<Image>&& img);
	void sync_checkerboard_with_image();
	void sync_gridlines_with_image();
	void sync_transform();
	void sync_gfx_with_image();

	void create_checkerboard_image();
	void sync_checkerboard_colors() const;
	void sync_checkerboard_texture() const;
	void set_checkerboard_uv_size(float width, float height) const;

	void hover_pixel_at(Position pos);
	void set_primary_color(RGBA color);
	void set_alternate_color(RGBA color);
	void cursor_press(MouseButton button);
	void cursor_release();
	bool cursor_cancel();

private:
	RGBA primary_color, alternate_color;
	std::array<Byte, 4> pric_pxs;
	std::array<Byte, 4> altc_pxs;
	std::array<Byte, 4> pric_pen_pxs;
	std::array<Byte, 4> altc_pen_pxs;

	void brush();
	void brush_submit();
	void brush_cancel();

	void brush_paint_tool();

public:
	enum : size_t
	{
		CHECKERBOARD,
		CURSOR_PENCIL,
		CURSOR_PEN,
		CURSOR_ERASER,
		CURSOR_SELECT,
		SPRITE, // LATER SPRITE_START
		_W_COUNT
	};
};

struct Easel : public Panel
{
	Shader color_square_shader, sprite_shader;
	Widget widget;
	glm::mat3 vp;

	MouseButtonHandler mb_handler;
	KeyHandler key_handler;
	ScrollHandler scroll_handler;

	Easel();
	Easel(const Easel&) = delete;
	Easel(Easel&&) noexcept = delete;

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

	void hover_pixel_under_cursor();

	enum : size_t
	{
		BACKGROUND,
		CANVAS,
		_W_COUNT
	};
};
