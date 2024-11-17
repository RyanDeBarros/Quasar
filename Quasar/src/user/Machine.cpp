#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "UserInput.h"
#include "GUI.h"
#include "pipeline/panels/Panel.h"
#include "pipeline/panels/Easel.h"
#include "pipeline/panels/Palette.h"
#include "pipeline/text/TextRender.h"
#include "pipeline/text/CommonFonts.h"
#include "variety/GLutility.h"
#include "variety/Utils.h"

#define QUASAR_INVALIDATE_PTR(ptr) delete ptr; ptr = nullptr;
#define QUASAR_INVALIDATE_ARR(arr) delete[] arr; arr = nullptr;

namespace Data
{
	static double current_time = 0.0;
	static double last_processed_time = 0.0;
	static double delta_time = 0.0;
	static void update_time()
	{
		current_time = glfwGetTime();
		delta_time = current_time - last_processed_time;
		last_processed_time = current_time;
	}

	static Scale app_inverse_scale{};
	
	namespace History
	{
		static bool held_undo = false, held_redo = false, on_starting_interval = true;
		static double held_time = 0.0;
		static const double held_interval = 0.1, held_start_interval = 0.5; // SETTINGS not const, but editable
	}
}

static PanelGroup* panels = nullptr;

static Easel* easel()
{
	return dynamic_cast<Easel*>(panels->panels[0].get());
}

static Palette* palette()
{
	return dynamic_cast<Palette*>(panels->panels[1].get());
}

void MachineImpl::init_panels_layout()
{
	easel()->bounds.x1 = window_layout_info.initial_brush_panel_width;
	easel()->bounds.x2 = window_layout_info.initial_width - window_layout_info.initial_palette_panel_width;
	easel()->bounds.y1 = window_layout_info.initial_views_panel_height;
	easel()->bounds.y2 = window_layout_info.initial_height - window_layout_info.initial_menu_panel_height;
	palette()->bounds.x1 = easel()->bounds.x2;
	palette()->bounds.x2 = window_layout_info.initial_width;
	palette()->bounds.y1 = easel()->bounds.y1;
	palette()->bounds.y2 = easel()->bounds.y2;
}

static void update_panels_to_window_size(int width, int height)
{
	//easel()->bounds.x1 = 
	if (palette()->visible)
		easel()->bounds.x2 = width - Machine.window_layout_info.initial_palette_panel_width;
	else
		easel()->bounds.x2 = width;
	//easel()->bounds.y1 = 
	easel()->bounds.y2 = height - Machine.window_layout_info.initial_menu_panel_height;
	palette()->bounds.x1 = easel()->bounds.x2;
	palette()->bounds.x2 = width;
	palette()->bounds.y1 = easel()->bounds.y1;
	palette()->bounds.y2 = easel()->bounds.y2;
}

MachineImpl::MachineImpl()
	: history(4'000'000) // SETTINGS and test that it works. currently it is 4MB, which is small to medium sized.
	// Possible levels could be: Lightweight (1-2MB) | Moderate (4-8MB) | Intense (16+MB). Due to underestimations of action sizes, always be on low end of history pool sizes.
	// Inevitably, the history's actual memory usage will be estimated, whether below or above the actual amount.
{
}

bool MachineImpl::create_main_window()
{
	main_window = new Window("Quasar", window_layout_info.initial_width, window_layout_info.initial_height);
	if (main_window)
	{
		query_gl_constants();
		update_raw_mouse_motion();
		update_vsync();

		main_window->root_window_size.children.push_back(&resize_handler);
		main_window->root_display_scale.children.push_back(&rescale_handler);
		main_window->root_mouse_button.children.push_back(&palette_mb_handler);
		main_window->root_mouse_button.children.push_back(&easel_mb_handler);
		main_window->root_key.children.push_back(&palette_key_handler);
		main_window->root_scroll.children.push_back(&palette_scroll_handler);
		main_window->root_scroll.children.push_back(&easel_scroll_handler);
		main_window->root_key.children.push_back(&global_key_handler);
		main_window->root_path_drop.children.push_back(&path_drop_handler);
		return true;
	}
	return false;
}

void MachineImpl::init_renderer()
{
	QUASAR_GL(glEnable(GL_SCISSOR_TEST));
	QUASAR_GL(glEnable(GL_BLEND));
	QUASAR_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	main_window->focus_context();
	set_clear_color(ColorFrame(RGB(0.1f, 0.1f, 0.1f), 0.1f)); // SETTINGS
	
	Fonts::load_common_fonts();

	panels = new PanelGroup();
	panels->panels.push_back(std::make_unique<Easel>());
	panels->panels.push_back(std::make_unique<Palette>());

	init_panels_layout();

	panels->sync_panels();

	resize_handler.callback = [this](const WindowSizeEvent& ws) {
		update_panels_to_window_size(ws.width, ws.height);
		panels->set_projection();
		QUASAR_GL(glViewport(0, 0, ws.width, ws.height));
		// LATER while resizing, just color window (block content) until resizing is done, for smoother transitioning.
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
		main_window->swap_buffers();
		Machine.on_render();
		};
	rescale_handler.callback = [](const DisplayScaleEvent& ds) {
		Machine.set_app_scale(ds.scale);
		};

	canvas_reset_camera();
	attach_canvas_controls();
	attach_global_user_controls();
	
	easel()->canvas.minor_gridlines.set_color(ColorFrame(RGBA(31, 63, 107, 255))); // SETTINGS
	easel()->canvas.minor_gridlines.line_width = 1.0f; // cannot be < 1.0 // SETTINGS
	easel()->canvas.major_gridlines.set_color(ColorFrame(RGBA(31, 72, 127, 255))); // SETTINGS
	easel()->canvas.major_gridlines.line_width = 4.0f; // cannot be < 1.0 // SETTINGS

	set_app_scale(main_window->display_scale());

	main_window->set_size_limits(window_layout_info.initial_brush_panel_width + window_layout_info.initial_brush_panel_width,
		//window_layout_info.initial_menu_panel_height + window_layout_info.initial_views_panel_height, GLFW_DONT_CARE, GLFW_DONT_CARE);
		//window_layout_info.initial_height, GLFW_DONT_CARE, GLFW_DONT_CARE); // LATER add status bar at bottom of window. also, add min/max limits to individual panels, and add up here.
		window_layout_info.menu_panel_height + window_layout_info.views_panel_height + (int)palette()->minimum_screen_display().y, GLFW_DONT_CARE, GLFW_DONT_CARE);
	
	Data::update_time();

	import_file(FileSystem::workspace_path("ex/flag.png"));
	//show_major_gridlines();
}

void MachineImpl::destroy()
{
	// NOTE no Image shared_ptrs should remain before destroying window.
	QUASAR_INVALIDATE_PTR(panels);
	Fonts::invalidate_common_fonts();
	history.clear_history();
	QUASAR_INVALIDATE_PTR(main_window); // invalidate window last
}

bool MachineImpl::should_exit() const
{
	return main_window->should_close();
}

void MachineImpl::on_render()
{
	process();
	main_window->new_frame();
	panels->render();
	main_window_clip().scissor();
	render_main_menu_bar();
	update_currently_bound_shader();
	main_window->end_frame();
}

static void process_undo()
{
	if (Machine.main_window->is_key_pressed(Key::Z) && Machine.main_window->is_ctrl_pressed())
	{
		if (!Machine.main_window->is_shift_pressed())
		{
			Data::History::held_time += Data::delta_time;
			if (Data::History::on_starting_interval)
			{
				if (Data::History::held_time > Data::History::held_start_interval)
				{
					Data::History::held_time -= Data::History::held_start_interval;
					while (Data::History::held_time > Data::History::held_interval)
						Data::History::held_time -= Data::History::held_interval;
					Machine.undo();
					Data::History::on_starting_interval = false;
				}
			}
			else if (Data::History::held_time > Data::History::held_interval)
			{
				do { Data::History::held_time -= Data::History::held_interval; } while (Data::History::held_time > Data::History::held_interval);
				Machine.undo();
			}
		}
		else
			Machine.start_held_redo();
	}
	else
		Data::History::held_undo = false;
}

static void process_redo()
{
	if (Machine.main_window->is_key_pressed(Key::Z) && Machine.main_window->is_ctrl_pressed())
	{
		if (Machine.main_window->is_shift_pressed())
		{
			Data::History::held_time += Data::delta_time;
			if (Data::History::on_starting_interval)
			{
				if (Data::History::held_time > Data::History::held_start_interval)
				{
					Data::History::held_time -= Data::History::held_start_interval;
					while (Data::History::held_time > Data::History::held_interval)
						Data::History::held_time -= Data::History::held_interval;
					Machine.redo();
					Data::History::on_starting_interval = false;
				}
			}
			else if (Data::History::held_time > Data::History::held_interval)
			{
				do { Data::History::held_time -= Data::History::held_interval; } while (Data::History::held_time > Data::History::held_interval);
				Machine.redo();
			}
		}
		else
			Machine.start_held_undo();
	}
	else
		Data::History::held_redo = false;
}

void MachineImpl::process()
{
	Data::update_time();
	easel()->update_panning();
	if (Data::History::held_undo)
		process_undo();
	else if (Data::History::held_redo)
		process_redo();
	LOG << LOG.flush; // flush all pending logs so that elsewhere, LOG.nl can be used without LOG.endl unless necessary.
}

void MachineImpl::mark()
{
	unsaved = true;
	// LATER edit title to include (*)
}

void MachineImpl::unmark()
{
	unsaved = false;
	// LATER remove (*) from title if it exists
}

bool MachineImpl::canvas_image_ready() const
{
	return easel()->canvas_image();
}

bool MachineImpl::canvas_is_panning() const
{
	return easel()->panning_info.panning;
}

void MachineImpl::canvas_begin_panning() const
{
	easel()->begin_panning();
}

void MachineImpl::canvas_end_panning() const
{
	easel()->end_panning();
}

void MachineImpl::canvas_cancel_panning() const
{
	easel()->cancel_panning();
}

void MachineImpl::canvas_zoom_by(float z) const
{
	easel()->zoom_by(z);
}

void MachineImpl::palette_insert_color()
{
	palette()->new_color();
}

void MachineImpl::palette_overwrite_color()
{
	palette()->overwrite_color();
}

void MachineImpl::palette_delete_color()
{
	palette()->delete_color();
}

void MachineImpl::palette_new_subpalette()
{
	palette()->new_subpalette();
}

void MachineImpl::palette_rename_subpalette()
{
	palette()->rename_subpalette();
}

void MachineImpl::palette_delete_subpalette()
{
	palette()->delete_subpalette();
}

Scale MachineImpl::inv_app_scale() const
{
	return Data::app_inverse_scale;
}

Scale MachineImpl::get_app_scale() const
{
	return 1.0f / Data::app_inverse_scale;
}

void MachineImpl::set_app_scale(Scale scale) const
{
	Data::app_inverse_scale = 1.0f / scale;
	panels->set_projection();
	float scale1d = mean2d1d(scale.x, scale.y);
	static const float gui_scale_factor = 1.15f; // SETTINGS
	float gui_scale = scale1d * gui_scale_factor;
	ImGui::GetStyle().ScaleAllSizes(gui_scale);
	ImGui::GetIO().FontGlobalScale = gui_scale;
}

void MachineImpl::set_clear_color(ColorFrame color)
{
	auto vec = color.rgba().as_vec();
	QUASAR_GL(glClearColor(vec[0], vec[1], vec[2], vec[3]));
}

Position MachineImpl::to_world_coordinates(Position screen_coordinates, const glm::mat3& inverse_vp) const
{
	glm::vec3 ndc{
		1.0f - 2.0f * (screen_coordinates.x / main_window->width()),
		1.0f - 2.0f * (screen_coordinates.y / main_window->height()),
		1.0f
	};

	glm::vec3 world_pos = inverse_vp * ndc;
	if (world_pos.z != 0.0f)
		world_pos / world_pos.z;

	return { -world_pos.x, -world_pos.y };
}

Position MachineImpl::to_screen_coordinates(Position world_coordinates, const glm::mat3& vp) const
{
	glm::vec3 world_pos{ world_coordinates.x, world_coordinates.y, 1.0f };
	glm::vec3 clip_space_pos = vp * world_pos;
	return {
		(1.0f + clip_space_pos.x) * 0.5f * main_window->width(),
		(1.0f + clip_space_pos.y) * 0.5f * main_window->height()
	};
}

Position MachineImpl::cursor_screen_pos() const
{
	return main_window->cursor_pos();
}

Position MachineImpl::cursor_screen_x() const
{
	return main_window->cursor_x();
}

Position MachineImpl::cursor_screen_y() const
{
	return main_window->cursor_y();
}

Position MachineImpl::cursor_world_pos(const glm::mat3& inverse_vp) const
{
	return to_world_coordinates(main_window->cursor_pos(), inverse_vp);
}

glm::vec2 MachineImpl::easel_cursor_world_pos() const
{
	return easel()->to_world_coordinates(cursor_screen_pos());
}

glm::vec2 MachineImpl::palette_cursor_world_pos() const
{
	return palette()->to_world_coordinates(cursor_screen_pos());
}

bool MachineImpl::cursor_in_easel() const
{
	return easel()->cursor_in_clipping();
}

bool MachineImpl::cursor_in_palette() const
{
	return palette()->cursor_in_clipping();
}

bool MachineImpl::new_file()
{
	if (unsaved)
	{
		int response = tinyfd_messageBox("Notice", "Do you want to save your changes first?", "yesnocancel", "question", 1);
		if (response == 0) return false;
		if (response == 1) if (!save_file()) return false;
	}
	current_filepath.clear();
	easel()->canvas_visible = false;
	mark();
	// LATER clear palletes/frames/layers/etc.
	return true;
}

static FilePath prompt_open_quasar_file(const char* message, const char* default_path = "", bool allow_multiple_selects = false)
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	FilePath filepath = tinyfd_openFileDialog(message, default_path, num_filters, filters, "", allow_multiple_selects);
	static const char* const fexts[num_filters] = { ".qua" };
	if (filepath.has_any_extension(fexts, num_filters))
		return filepath;
	return "";
}

bool MachineImpl::open_file()
{
	if (unsaved)
	{
		// LATER ask if user wants to save
	}
	FilePath openfile = prompt_open_quasar_file("Open file");
	if (openfile.empty()) return false;
	current_filepath = openfile;
	mark();
	open_file(openfile);
	return true;
}

static FilePath prompt_open_image_file(const char* message, const char* default_path = "", bool allow_multiple_selects = false)
{
	static const int num_filters = 6;
	static const char* const filters[num_filters] = { "*.png", "*.gif", "*.jpg", "*.bmp", "*.tga", "*.hdr" };
	FilePath filepath = tinyfd_openFileDialog(message, default_path, num_filters, filters, "", allow_multiple_selects);
	static const char* const fexts[num_filters] = { ".png", ".gif", ".jpg", ".bmp", ".tga", ".hdr" };
	if (filepath.has_any_extension(fexts, num_filters))
		return filepath;
	return "";
}

bool MachineImpl::import_file()
{
	FilePath importfile = prompt_open_image_file("Import file");
	if (importfile.empty()) return false;
	mark();
	import_file(importfile);
	return true;
}

static FilePath prompt_save_image_file(const char* message, const char* default_path = "")
{
	static const int num_filters = 6;
	static const char* const filters[num_filters] = { "*.png", "*.gif", "*.jpg", "*.bmp", "*.tga", "*.hdr" };
	FilePath filepath = tinyfd_saveFileDialog(message, default_path, num_filters, filters, "");
	static const char* const fexts[num_filters] = { ".png", ".gif", ".jpg", ".bmp", ".tga", ".hdr" };
	if (filepath.has_any_extension(fexts, num_filters))
		return filepath;
	return "";
}

bool MachineImpl::export_file() const
{
	if (!easel()->canvas_image())
		return false;
	// LATER open custom dialog first (or only) for export settings first, including file format, upscale/downscale, frame/layer options, etc.
	// If tinyfd is still used for file selection, make sure to use the proper filter corresponding to the image format, as well as possibly a default path.
	// Also make sure that the selected file's extension matches the image format.
	FilePath exportfile = prompt_save_image_file("Export file");
	if (exportfile.empty()) return false;
	return easel()->canvas_image()->write_to_file(exportfile, ImageFormat::PNG);
}

static FilePath prompt_save_quasar_file(const char* message, const char* default_path = "")
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	FilePath filepath = tinyfd_saveFileDialog(message, default_path, num_filters, filters, "");
	static const char* const fexts[num_filters] = { ".qua" };
	if (filepath.has_any_extension(fexts, num_filters))
		return filepath;
	return "";
}

bool MachineImpl::save_file()
{
	if (current_filepath.empty())
	{
		FilePath savefile = prompt_save_quasar_file("Save file");
		if (savefile.empty()) return false;
		// LATER create new file
		current_filepath = savefile;
	}
	save_file(current_filepath);
	unmark();
	return true;
}

bool MachineImpl::save_file_as()
{
	FilePath savefile = prompt_save_quasar_file("Save file as");
	if (savefile.empty()) return false;
	// LATER create new file
	current_filepath = savefile;
	save_file(savefile);
	unmark();
	return true;
}

bool MachineImpl::save_file_copy()
{
	FilePath savefile = prompt_save_quasar_file("Save file copy");
	if (savefile.empty()) return false;
	// LATER create new file
	save_file(savefile);
	unmark();
	return true;
}

void MachineImpl::open_file(const FilePath& filepath)
{
	// LATER actually open quasar file
}

void MachineImpl::import_file(const FilePath& filepath)
{	
	easel()->set_canvas_image(std::make_shared<Image>(filepath));
	auto title = "Quasar - " + filepath.filename();
	main_window->set_title(title.c_str()); // LATER don't set title of window. put image filename in bottom status bar
	canvas_reset_camera();
}

void MachineImpl::save_file(const FilePath& filepath)
{
	// LATER actually save changes to filepath
}

void MachineImpl::start_held_undo()
{
	Data::History::held_undo = true;
	Data::History::held_redo = false;
	Data::History::held_time = 0.0;
	Data::History::on_starting_interval = true;
}

void MachineImpl::start_held_redo()
{
	Data::History::held_redo = true;
	Data::History::held_undo = false;
	Data::History::held_time = 0.0;
	Data::History::on_starting_interval = true;
}

struct FlipHorizontallyAction : public ActionBase
{
	FlipHorizontallyAction() { weight = sizeof(FlipHorizontallyAction); }
	virtual void forward() override { easel()->canvas_image()->flip_horizontally(); }
	virtual void backward() override { easel()->canvas_image()->flip_horizontally(); }
	virtual bool equals(const ActionBase& other) const override { return dynamic_cast<const FlipHorizontallyAction*>(&other); }
};

void MachineImpl::flip_horizontally()
{
	static std::shared_ptr<ActionBase> a = std::make_shared<FlipHorizontallyAction>();
	history.execute(a);
}

struct FlipVerticallyAction : public ActionBase
{
	FlipVerticallyAction() { weight = sizeof(FlipVerticallyAction); }
	virtual void forward() override { easel()->canvas_image()->flip_vertically(); }
	virtual void backward() override { easel()->canvas_image()->flip_vertically(); }
	virtual bool equals(const ActionBase& other) const override { return dynamic_cast<const FlipVerticallyAction*>(&other); }
};

void MachineImpl::flip_vertically()
{
	static std::shared_ptr<ActionBase> a = std::make_shared<FlipVerticallyAction>();
	history.execute(a);
}

struct Rotate90Action : public ActionBase
{
	Rotate90Action() { weight = sizeof(Rotate90Action); }
	virtual void forward() override { easel()->canvas_image()->rotate_90(); easel()->update_canvas_image(); }
	virtual void backward() override { easel()->canvas_image()->rotate_270(); easel()->update_canvas_image(); }
	virtual bool equals(const ActionBase& other) const override { return dynamic_cast<const Rotate90Action*>(&other); }
};

void MachineImpl::rotate_90()
{
	static std::shared_ptr<ActionBase> a = std::make_shared<Rotate90Action>();
	history.execute(a);
}

struct Rotate180Action : public ActionBase
{
	Rotate180Action() { weight = sizeof(Rotate180Action); }
	virtual void forward() override { easel()->canvas_image()->rotate_180(); easel()->update_canvas_image(); }
	virtual void backward() override { easel()->canvas_image()->rotate_180(); easel()->update_canvas_image(); }
	virtual bool equals(const ActionBase& other) const override { return dynamic_cast<const Rotate180Action*>(&other); }
};

void MachineImpl::rotate_180()
{
	static std::shared_ptr<ActionBase> a = std::make_shared<Rotate180Action>();
	history.execute(a);
}

struct Rotate270Action : public ActionBase
{
	Rotate270Action() { weight = sizeof(Rotate270Action); }
	virtual void forward() override { easel()->canvas_image()->rotate_270(); easel()->update_canvas_image(); }
	virtual void backward() override { easel()->canvas_image()->rotate_90(); easel()->update_canvas_image(); }
	virtual bool equals(const ActionBase& other) const override { return dynamic_cast<const Rotate270Action*>(&other); }
};

void MachineImpl::rotate_270()
{
	static std::shared_ptr<ActionBase> a = std::make_shared<Rotate270Action>();
	history.execute(a);
}

bool MachineImpl::brush_panel_visible() const
{
	return true;
}

void MachineImpl::open_brush_panel() const
{
	panels->set_projection();
}

void MachineImpl::close_brush_panel() const
{
	panels->set_projection();
}

bool MachineImpl::palette_panel_visible() const
{
	return palette()->visible;
}

void MachineImpl::open_palette_panel() const
{
	palette()->visible = true;
	easel()->bounds.x2 = main_window->width() - window_layout_info.initial_palette_panel_width;
	palette()->bounds.x1 = easel()->bounds.x2;
	palette()->bounds.x2 = main_window->width();
	panels->set_projection();
}

void MachineImpl::close_palette_panel() const
{
	palette()->visible = false;
	easel()->bounds.x2 = main_window->width();
	panels->set_projection();
}

bool MachineImpl::views_panel_visible() const
{
	return true;
}

void MachineImpl::open_views_panel() const
{
	panels->set_projection();
}

void MachineImpl::close_views_panel() const
{
	panels->set_projection();
}

void MachineImpl::canvas_reset_camera() const
{
	easel()->reset_camera();
}

bool MachineImpl::minor_gridlines_visible() const
{
	return easel()->minor_gridlines_are_visible();
}

void MachineImpl::show_minor_gridlines() const
{
	easel()->set_minor_gridlines_visibility(true);
}

void MachineImpl::hide_minor_gridlines() const
{
	easel()->set_minor_gridlines_visibility(false);
}

bool MachineImpl::major_gridlines_visible() const
{
	return easel()->major_gridlines_are_visible();
}

void MachineImpl::show_major_gridlines() const
{
	easel()->set_major_gridlines_visibility(true);
}

void MachineImpl::hide_major_gridlines() const
{
	easel()->set_major_gridlines_visibility(false);
}

void MachineImpl::download_user_manual() const
{
	static const int num_filters = 1;
	static const char* filters[num_filters] = { "md" };
	FilePath default_filepath = FileSystem::workspace_path("quasar_user_manual.md");
	FilePath filepath = tinyfd_saveFileDialog("Save user manual as ", default_filepath.c_str(), num_filters, filters, "Markdown (*.md)");
	if (filepath.empty()) return;
	std::filesystem::copy_file(FileSystem::resources_path("user_manual.md").c_str(), filepath.c_str(), std::filesystem::copy_options::overwrite_existing);
}
