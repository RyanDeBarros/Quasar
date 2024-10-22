#pragma once

#include <string>

#include "variety/Geometry.h"
#include "variety/History.h"
#include "Platform.h"

struct MachineImpl
{
	MachineImpl() = default;
	MachineImpl(const MachineImpl&) = delete;
	MachineImpl(MachineImpl&&) noexcept = delete;
	~MachineImpl() = default;

	ActionHistory history;
	Window* main_window = nullptr;

	std::string current_filepath = "";
	bool unsaved = true;

	std::vector<std::string> recent_files;
	std::vector<std::string> recent_image_files;

	// Canvas camera
	struct
	{
		Position initial_cursor_pos{};
		Position initial_canvas_pos{};
		bool panning = false;
	} panning_info;
	struct
	{
		// LATER put these zoom constants somewhere else? In settings? They would be variable in that case.
		constexpr static float initial = 0.5f;
		constexpr static float in_min = 0.01f;
		constexpr static float in_max = 100.0f;
		constexpr static float factor = 1.5f;
		constexpr static float factor_shift = 1.05f;
		float zoom = initial;
	} zoom_info;

	bool create_main_window();
	void init_renderer();
	void destroy();
	void exit() const { main_window->request_close(); }
	bool should_exit() const;
	void on_render() const;
	void mark();
	void unmark();

	// Easel
	bool cursor_in_easel() const;
	void set_easel_app_scale(float sc) const;

	// Canvas
	FlatTransform& canvas_transform() const;
	Position& canvas_position() const { return canvas_transform().position; }
	Scale& canvas_scale() const { return canvas_transform().scale; }
	void sync_canvas_transform() const;

	// File menu
	bool new_file();
	bool open_file();
	bool import_file();
	bool export_file() const;
	bool save_file();
	bool save_file_as();
	bool save_file_copy();

	void open_file(const char* filepath);
	void import_file(const char* filepath);
	void save_file(const char* filepath);

	void undo() { history.undo(); }
	bool undo_enabled() const { return history.undo_size() != 0; }
	void redo() { history.redo(); }
	bool redo_enabled() const { return history.redo_size() != 0; }

	// User controls
	void canvas_begin_panning();
	void canvas_end_panning();
	void canvas_cancel_panning();
	void canvas_update_panning() const;
	void canvas_zoom_by(float zoom);

	// Edit menu
	void flip_horizontally();
	void flip_vertically();
	void rotate_180();

	// View menu
	void canvas_reset_camera();
	bool minor_gridlines_visible();
	void show_minor_gridlines();
	void hide_minor_gridlines();
	bool major_gridlines_visible();
	void show_major_gridlines();
	void hide_major_gridlines();
};

inline MachineImpl Machine;
