#pragma once

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

private:
	glm::vec2 checker_size_inv = glm::vec2(1.0f / 16.0f);
public:
	glm::ivec2 get_checker_size() const { return { roundf(1.0f / checker_size_inv.x), roundf(1.0f / checker_size_inv.y) }; }
	void set_checker_size(glm::ivec2 checker_size);

	Canvas(Shader* sprite_shader);
	Canvas(const Canvas&) = delete;
	Canvas(Canvas&&) noexcept = delete;

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

	enum : size_t
	{
		CHECKERBOARD,
		SPRITE, // LATER SPRITE_START
		_W_COUNT
	};
};

struct Easel : public Panel
{
	Shader bkg_shader;
	Shader sprite_shader;
	Widget widget;

	MouseButtonHandler mb_handler;
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

	void sync_widget();
	void sync_canvas_transform();

	Image* canvas_image() const;

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
