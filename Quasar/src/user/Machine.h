#pragma once

#include <string>

#include "Platform.h"
#include "Preferences.h"
#include "variety/Geometry.h"
#include "variety/History.h"
#include "variety/FileSystem.h"

struct MachineImpl
{
	MachineImpl();
	MachineImpl(const MachineImpl&) = delete;
	MachineImpl(MachineImpl&&) noexcept = delete;
	~MachineImpl() = default;

	struct Easel* easel() const;
	struct PalettePanel* palette() const;
	struct BrushesPanel* brushes() const;
	struct ScenePanel* scene() const;
	struct MenuPanel* menu() const;

	ActionHistory history;
	Window* main_window = nullptr;
	
	struct
	{
		std::shared_ptr<Cursor> ARROW;
		std::shared_ptr<Cursor> IBEAM;
		std::shared_ptr<Cursor> CROSSHAIR;
		std::shared_ptr<Cursor> HAND;
		std::shared_ptr<Cursor> RESIZE_EW;
		std::shared_ptr<Cursor> RESIZE_NS;
		std::shared_ptr<Cursor> RESIZE_NW_SE;
		std::shared_ptr<Cursor> RESIZE_NE_SW;
		std::shared_ptr<Cursor> RESIZE_OMNI;
		std::shared_ptr<Cursor> CANCEL;
	} cursors;

	struct
	{
		constexpr static int initial_width = 2160;
		constexpr static int initial_height = 1440;
		constexpr static int initial_menu_panel_height = 32;
		constexpr static int initial_brush_panel_width = 380;
		constexpr static int initial_palette_panel_width = 380;
		constexpr static int initial_scene_panel_height = 288;
		int menu_panel_height = initial_menu_panel_height;
		int brushes_panel_width = initial_brush_panel_width;
		int palette_panel_width = initial_palette_panel_width;
		int scene_panel_height = initial_scene_panel_height;
	} window_layout_info;

	FilePath current_filepath = "";
	bool unsaved = true;
	WorkspacePreferences preferences;

	std::vector<std::string> recent_files;
	std::vector<std::string> recent_image_files;

	bool create_main_window();
	void init_renderer();
	void destroy();
	void exit() const { main_window->request_close(); }
	bool should_exit() const;
	void on_render();
	void process();
	void mark();
	void unmark();
	Scale inv_app_scale() const;
	Scale get_app_scale() const;
	void set_app_scale(Scale sc) const;
	void set_clear_color(ColorFrame color);

	ClippingRect main_window_clip() const { return ClippingRect(0, 0, main_window->width(), main_window->height()); }

	Position to_world_coordinates(Position screen_coordinates, const glm::mat3& inverse_vp) const;
	Position to_screen_coordinates(Position world_coordinates, const glm::mat3& vp) const;
	Scale to_world_size(Scale screen_size, const glm::mat3& vp) const;
	Scale to_screen_size(Scale world_size, const glm::mat3& vp) const;

	Position cursor_screen_pos() const;
	Position cursor_screen_x() const;
	Position cursor_screen_y() const;
	Position cursor_world_pos(const glm::mat3& inverse_vp) const;

	int vsync = 0;
	void update_vsync() const { glfwSwapInterval(vsync); }
	bool raw_mouse_motion = true;
	void update_raw_mouse_motion() const { main_window->set_raw_mouse_motion(raw_mouse_motion); }

	float time() const;
	float delta_time() const;

	// Control Scheme
	enum class ControlScheme get_control_scheme() const;
	void set_control_scheme(enum class ControlScheme scheme) const;

	// Panels
	glm::vec2 easel_cursor_world_pos() const;
	glm::vec2 palette_cursor_world_pos() const;
	bool cursor_in_easel() const;
	bool cursor_in_palette() const;

	// Canvas
	bool canvas_image_ready() const;
	bool canvas_is_panning() const;
	void canvas_cancel_panning() const;

	// Palette
	void palette_insert_color();
	void palette_overwrite_color();
	void palette_delete_color();
	void palette_new_subpalette();
	void palette_rename_subpalette();
	void palette_delete_subpalette();

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
	void start_held_undo();
	void start_held_redo();

	// Edit menu
	void flip_horizontally();
	void flip_vertically();
	void rotate_90();
	void rotate_180();
	void rotate_270();

	// View menu
	bool brushes_panel_visible() const;
	void open_brushes_panel() const;
	void close_brushes_panel() const;
	bool palette_panel_visible() const;
	void open_palette_panel() const;
	void close_palette_panel() const;
	bool scene_panel_visible() const;
	void open_scene_panel() const;
	void close_scene_panel() const;
	void canvas_reset_camera() const;
	bool minor_gridlines_visible() const;
	void show_minor_gridlines() const;
	void hide_minor_gridlines() const;
	bool major_gridlines_visible() const;
	void show_major_gridlines() const;
	void hide_major_gridlines() const;

	// Help menu
	void download_user_manual() const;
};

inline MachineImpl Machine;
inline Window* MainWindow = nullptr;
inline struct Easel* MEasel = nullptr;
inline struct PalettePanel* MPalette = nullptr;
inline struct BrushesPanel* MBrushes = nullptr;
inline struct ScenePanel* MScene = nullptr;
inline struct MenuPanel* MMenu = nullptr;
