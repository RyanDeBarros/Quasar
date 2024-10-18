#pragma once

#include "edit/Image.h"
#include "variety/History.h"
#include "pipeline/Renderer.h"

struct MachineImpl
{
	MachineImpl() = default;
	MachineImpl(const MachineImpl&) = delete;
	MachineImpl(MachineImpl&&) noexcept = delete;
	~MachineImpl() = default;

	ActionHistory history;

	Window* main_window = nullptr;
	Renderer* canvas_renderer = nullptr;
	Image* canvas_image = nullptr;

	std::string current_filepath = "";
	bool unsaved = true;

	std::vector<std::string> recent_files;
	std::vector<std::string> recent_image_files;

	// Canvas camera
	Position pan_initial_cursor_pos{};
	Position pan_initial_view_pos{};
	bool panning = false;
	// TODO put these zoom constants somewhere else? In settings? They would be variable in that case.
	constexpr static float zoom_initial = 0.5f;
	constexpr static float zoom_in_min = 0.01f;
	constexpr static float zoom_in_max = 100.0f;
	constexpr static float zoom_factor = 1.5f;
	constexpr static float zoom_factor_shift = 1.05f;
	float zoom = zoom_initial;

	bool create_main_window();
	void init_renderer();
	void destroy();
	void exit() const { main_window->request_close(); }
	bool should_exit() const;
	void on_render();
	void draw_gridlines();
	void mark();
	void unmark();
	Transform canvas_transform() const;
	Position canvas_position() const { return canvas_transform().position; }
	Rotation canvas_rotation() const { return canvas_transform().rotation; }
	Scale canvas_scale() const { return canvas_transform().scale; }
	void set_canvas_transform(Transform transform) const;
	void set_canvas_position(Position position) const;
	void set_canvas_rotation_scale(Rotation rotation, Scale scale) const;
	void set_canvas_rotation(Rotation rotation) const;
	void set_canvas_scale(Scale scale) const;

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
	void canvas_zoom_by(float zoom);
	void canvas_reset_camera();
	void canvas_update_panning() const;

	void flip_horizontally();
	void flip_vertically();
	void rotate_180();
};

inline MachineImpl Machine;
