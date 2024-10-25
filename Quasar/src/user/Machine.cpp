#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "UserInput.h"
#include "GUI.h"
#include "pipeline/Easel.h"
#include "variety/GLutility.h"
#include "variety/Utils.h"

#define QUASAR_INVALIDATE_PTR(ptr) delete ptr; ptr = nullptr;
#define QUASAR_INVALIDATE_ARR(arr) delete[] arr; arr = nullptr;

static Easel* easel = nullptr;

bool MachineImpl::create_main_window()
{
	main_window = new Window("Quasar", 2160, 1440, true);
	if (main_window)
	{
		update_raw_mouse_motion();
		update_vsync();
		return true;
	}
	return false;
}

void MachineImpl::init_renderer()
{
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f)); // SETTINGS
	QUASAR_GL(glEnable(GL_SCISSOR_TEST));
	QUASAR_GL(glEnable(GL_BLEND));
	QUASAR_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	main_window->focus_context();

	easel = new Easel(main_window);

	canvas_reset_camera();
	attach_canvas_controls();
	attach_global_user_controls();

	easel->clip.window_size_to_bounds = [](int w, int h) -> glm::ivec4 { return {
		w / 10, h / 10, 8 * w / 10, 8 * h / 10
	}; };
	easel->clip.update_window_size(main_window->width(), main_window->height());

	easel->canvas.minor_gridlines.set_color(ColorFrame(RGBA(31_UC, 63_UC, 107_UC, 255_UC))); // SETTINGS
	easel->canvas.minor_gridlines.line_width = 1.0f; // cannot be < 1.0 // SETTINGS
	easel->canvas.major_gridlines.set_color(ColorFrame(RGBA(31_UC, 72_UC, 127_UC, 255_UC))); // SETTINGS
	easel->canvas.major_gridlines.line_width = 4.0f; // cannot be < 1.0 // SETTINGS

	set_easel_app_scale(1.5f); // SETTINGS
	import_file(FileSystem::workspace_path("ex/flag.png"));
	//show_major_gridlines();
}

void MachineImpl::destroy()
{
	// NOTE no Image shared_ptrs should remain before destroying window.
	QUASAR_INVALIDATE_PTR(easel);
	QUASAR_INVALIDATE_PTR(main_window); // invalidate window last
}

bool MachineImpl::should_exit() const
{
	return main_window->should_close();
}

void MachineImpl::on_render() const
{
	canvas_update_panning();
	main_window->new_frame();
	easel->render();
	render_gui();
	update_currently_bound_shader();
	main_window->end_frame();
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

FlatTransform& MachineImpl::canvas_transform() const
{
	return easel->canvas.transform();
}

void MachineImpl::sync_canvas_transform() const
{
	easel->sync_canvas_transform();
}

bool MachineImpl::canvas_image_ready() const
{
	return easel->canvas_image();
}

bool MachineImpl::cursor_in_easel() const
{
	return easel->cursor_in_clipping();
}

void MachineImpl::set_easel_app_scale(float sc) const
{
	easel->set_app_scale(sc);
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
	easel->canvas_visible = false;
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
	if (!easel->canvas_image())
		return false;
	// LATER open custom dialog first (or only) for export settings first, including file format, upscale/downscale, frame/layer options, etc.
	// If tinyfd is still used for file selection, make sure to use the proper filter corresponding to the image format, as well as possibly a default path.
	// Also make sure that the selected file's extension matches the image format.
	FilePath exportfile = prompt_save_image_file("Export file");
	if (exportfile.empty()) return false;
	return easel->canvas_image()->write_to_file(exportfile, ImageFormat::PNG);
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
	easel->set_canvas_image(std::make_shared<Image>(filepath));
	auto title = "Quasar - " + filepath.filename();
	main_window->set_title(title.c_str());
	canvas_reset_camera();
}

void MachineImpl::save_file(const FilePath& filepath)
{
	// LATER actually save changes to filepath
}

void MachineImpl::canvas_begin_panning()
{
	if (!panning_info.panning)
	{
		panning_info.initial_canvas_pos = canvas_position();
		panning_info.initial_cursor_pos = easel->get_app_cursor_pos();
		panning_info.panning = true;
		main_window->override_gui_cursor_change(true);
		main_window->set_cursor(create_cursor(StandardCursor::RESIZE_OMNI));
	}
}

void MachineImpl::canvas_end_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		main_window->override_gui_cursor_change(false);
		main_window->set_cursor(create_cursor(StandardCursor::ARROW));
		main_window->set_mouse_mode(MouseMode::VISIBLE);
	}
}

void MachineImpl::canvas_cancel_panning()
{
	if (panning_info.panning)
	{
		panning_info.panning = false;
		canvas_position() = panning_info.initial_canvas_pos;
		sync_canvas_transform();
		main_window->override_gui_cursor_change(false);
		main_window->set_cursor(create_cursor(StandardCursor::ARROW));
		main_window->set_mouse_mode(MouseMode::VISIBLE);
	}
}

void MachineImpl::canvas_update_panning() const
{
	if (panning_info.panning)
	{
		Position pan_delta = easel->get_app_cursor_pos() - panning_info.initial_cursor_pos;
		Position pos = pan_delta + panning_info.initial_canvas_pos;
		if (main_window->is_shift_pressed())
		{
			if (std::abs(pan_delta.x) < std::abs(pan_delta.y))
				pos.x = panning_info.initial_canvas_pos.x;
			else
				pos.y = panning_info.initial_canvas_pos.y;
		}
		canvas_position() = pos;
		sync_canvas_transform();

		if (main_window->mouse_mode() != MouseMode::VIRTUAL && !easel->cursor_in_clipping())
			main_window->set_mouse_mode(MouseMode::VIRTUAL); // LATER this may annoyingly reduce cursor speed, although that might just be for trackpad.
	}
}

void MachineImpl::canvas_zoom_by(float z)
{
	Position cursor_world;
	if (!main_window->is_alt_pressed())
		cursor_world = easel->to_world_coordinates(main_window->cursor_pos());

	float factor = main_window->is_shift_pressed() ? zoom_info.factor_shift : zoom_info.factor;
	float new_zoom = std::clamp(zoom_info.zoom * glm::pow(factor, z), zoom_info.in_min, zoom_info.in_max);
	float zoom_change = new_zoom / zoom_info.zoom;
	canvas_scale() *= zoom_change;
	Position delta_position = (canvas_position() - cursor_world) * zoom_change;
	canvas_position() = cursor_world + delta_position;

	sync_canvas_transform();
	zoom_info.zoom = new_zoom;
}

void MachineImpl::flip_horizontally()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel->canvas_image()->flip_horizontally(); },
		[this]() { easel->canvas_image()->flip_horizontally(); }
	));
	history.execute(a);
}

void MachineImpl::flip_vertically()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel->canvas_image()->flip_vertically(); },
		[this]() { easel->canvas_image()->flip_vertically(); }
	));
	history.execute(a);
}

void MachineImpl::rotate_90()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel->canvas_image()->rotate_90(); easel->update_canvas_image(); },
		[this]() { easel->canvas_image()->rotate_270(); easel->update_canvas_image(); })
	);
	history.execute(a);
}

void MachineImpl::rotate_180()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel->canvas_image()->rotate_180(); },
		[this]() { easel->canvas_image()->rotate_180(); }
	));
	history.execute(a);
}

void MachineImpl::rotate_270()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel->canvas_image()->rotate_270(); easel->update_canvas_image(); },
		[this]() { easel->canvas_image()->rotate_90(); easel->update_canvas_image(); }
	));
	history.execute(a);
}

void MachineImpl::canvas_reset_camera()
{
	zoom_info.zoom = zoom_info.initial;
	canvas_transform() = {};
	if (easel->canvas_image())
	{
		float fit_scale = std::min(easel->get_app_width() / easel->canvas_image()->buf.width, easel->get_app_height() / easel->canvas_image()->buf.height);
		if (fit_scale < 1.0f)
		{
			canvas_scale() *= fit_scale;
			zoom_info.zoom *= fit_scale;
		}
		else
		{
			fit_scale /= preferences.min_initial_image_window_proportion;
			if (fit_scale > 1.0f)
			{
				canvas_scale() *= fit_scale;
				zoom_info.zoom *= fit_scale;
			}
		}
	}
	sync_canvas_transform();
}

bool MachineImpl::minor_gridlines_visible()
{
	return easel->minor_gridlines_are_visible();
}

void MachineImpl::show_minor_gridlines()
{
	easel->set_minor_gridlines_visibility(true);
}

void MachineImpl::hide_minor_gridlines()
{
	easel->set_minor_gridlines_visibility(false);
}

bool MachineImpl::major_gridlines_visible()
{
	return easel->major_gridlines_are_visible();
}

void MachineImpl::show_major_gridlines()
{
	easel->set_major_gridlines_visibility(true);
}

void MachineImpl::hide_major_gridlines()
{
	easel->set_major_gridlines_visibility(false);
}

void MachineImpl::download_user_manual()
{
	static const int num_filters = 1;
	static const char* filters[num_filters] = { "md" };
	FilePath default_filepath = FileSystem::workspace_path("quasar_user_manual.md");
	FilePath filepath = tinyfd_saveFileDialog("Save user manual as ", default_filepath.c_str(), num_filters, filters, "Markdown (*.md)");
	if (filepath.empty()) return;
	std::filesystem::copy_file(FileSystem::resources_path("user_manual.md").c_str(), filepath.c_str(), std::filesystem::copy_options::overwrite_existing);
}
