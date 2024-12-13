#pragma once

#include <array>

#include "Panel.h"
#include "user/Platform.h"
#include "../widgets/Widget.h"
#include "edit/image/Image.h"

struct Canvas;

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

	void handle_arrow_key_pressed(const KeyEvent& k);
	void handle_arrow_key_released(const KeyEvent& k);

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

private:
	struct
	{
		Position initial_cursor_pos{};
		Position initial_canvas_pos{};
		bool panning = false;
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

public:
	bool is_panning() const { return panning_info.panning; }
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

private:
	friend struct Canvas;

	struct
	{
		Position initial_cursor_pos{};
		Position initial_canvas_pos{};
		WindowHandle wh;
	} mouse_move_sel_info;

	struct
	{
		bool on_starting_interval = false;
		float held_time = 0.0f;
		int move_x = 0, move_y = 0;
		bool left = false, right = false, up = false, down = false;
		static inline const float held_speed_factor = 1.0f; // SETTINGS
		static inline const float held_interval = 0.1f;
		static inline const float held_start_interval = 0.5f;
	} arrows_move_sel_info;

	enum class MovingMode
	{
		NONE,
		MOUSE,
		ARROWS
	} moving_mode = MovingMode::NONE;

	bool begin_mouse_move_selection();
	void end_mouse_move_selection();
	bool cancel_mouse_move_selection();
	void update_mouse_move_selection();
	
	void update_arrows_move_selection();
	void update_arrows_move_amounts();
	bool cancel_arrows_move_selection();

public:
	Canvas& canvas();
	const Canvas& canvas() const;

	enum : size_t
	{
		BACKGROUND,
		CANVAS,
		_W_COUNT
	};
};
