#pragma once

#include <string>

#include "variety/Geometry.h"
#include "variety/History.h"
#include "variety/FileSystem.h"
#include "Platform.h"
#include "Preferences.h"

struct MachineImpl
{
	MachineImpl() = default;
	MachineImpl(const MachineImpl&) = delete;
	MachineImpl(MachineImpl&&) noexcept = delete;
	~MachineImpl() = default;

	ActionHistory history;
	Window* main_window = nullptr;

	struct
	{
		constexpr static int initial_width = 2160;
		constexpr static int initial_height = 1440;
		constexpr static int initial_menu_panel_height = 32;
		constexpr static int initial_brush_panel_width = 432;
		constexpr static int initial_palette_panel_width = 432;
		constexpr static int initial_views_panel_height = 288;
		int menu_panel_height = initial_menu_panel_height;
		int brush_panel_width = initial_brush_panel_width;
		int palette_panel_width = initial_palette_panel_width;
		int views_panel_height = initial_views_panel_height;
	} window_layout_info;

	FilePath current_filepath = "";
	bool unsaved = true;
	WorkspacePreferences preferences;

	std::vector<std::string> recent_files;
	std::vector<std::string> recent_image_files;

	int vsync = 0;
	void update_vsync() const { glfwSwapInterval(vsync); }
	bool raw_mouse_motion = true;
	void update_raw_mouse_motion() const { main_window->set_raw_mouse_motion(raw_mouse_motion); }

	// Canvas camera
	struct
	{
		Position initial_cursor_pos{};
		Position initial_canvas_pos{};
		bool panning = false;
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

	bool create_main_window();
	void init_renderer();
	void destroy();
	void exit() const { main_window->request_close(); }
	bool should_exit() const;
	void on_render() const;
	void mark();
	void unmark();
	void set_app_scale(glm::vec2 sc) const;
	void sync_window_panel_sizes() const;

	// Easel
	bool cursor_in_easel() const;
	
	// Canvas
	FlatTransform& canvas_transform() const;
	Position& canvas_position() const { return canvas_transform().position; }
	Scale& canvas_scale() const { return canvas_transform().scale; }
	void sync_canvas_transform() const;
	bool canvas_image_ready() const;

	// File menu
	bool new_file();
	bool open_file();
	bool import_file();
	bool export_file() const;
	bool save_file();
	bool save_file_as();
	bool save_file_copy();

	void open_file(const FilePath& filepath);
	void import_file(const FilePath& filepath);
	void save_file(const FilePath& filepath);

	void undo() { history.undo(); mark(); }
	bool undo_enabled() const { return history.undo_size() != 0; }
	void redo() { history.redo(); mark(); }
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
	void rotate_90();
	void rotate_180();
	void rotate_270();

	// View menu
	bool brush_panel_visible() const;
	void open_brush_panel() const;
	void close_brush_panel() const;
	bool palette_panel_visible() const;
	void open_palette_panel() const;
	void close_palette_panel() const;
	bool views_panel_visible() const;
	void open_views_panel() const;
	void close_views_panel() const;
	void canvas_reset_camera();
	bool minor_gridlines_visible();
	void show_minor_gridlines();
	void hide_minor_gridlines();
	bool major_gridlines_visible();
	void show_major_gridlines();
	void hide_major_gridlines();

	// Help menu
	void download_user_manual();
};

inline MachineImpl Machine;
