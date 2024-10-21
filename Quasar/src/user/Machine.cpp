#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "UserInput.h"
#include "GUI.h"
#include "pipeline/Easel.h"
#include "variety/GLutility.h"

#define QUASAR_INVALIDATE_PTR(ptr) delete ptr; ptr = nullptr;
#define QUASAR_INVALIDATE_ARR(arr) delete[] arr; arr = nullptr;

static Easel* easel = nullptr;

bool MachineImpl::create_main_window()
{
	main_window = new Window("Quasar", 2160, 1440, true);
	if (main_window)
	{
		main_window->set_raw_mouse_motion(true); // LATER settable from user settings
		return true;
	}
	return false;
}

void MachineImpl::init_renderer()
{
	glfwSwapInterval(GLFW_FALSE); // LATER off by default, but add to user settings.
	QUASAR_GL(glClearColor(0.1f, 0.1f, 0.1f, 0.1f));
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

	easel->minor_gridlines.set_color(ColorFrame(RGBA(31_UC, 63_UC, 127_UC, 255_UC)));
	easel->minor_gridlines.line_width = 1.5f; // cannot be < 1.0
	easel->major_gridlines.set_color(ColorFrame(RGBA(31_UC, 72_UC, 144_UC, 255_UC)));
	easel->major_gridlines.line_width = 3.0f; // cannot be < 1.0

	set_easel_scale(1.5f, 1.5f); // TODO 1-dimensional app scale
	//import_file("ex/oddtux.png");
	//show_major_gridlines();
}

void MachineImpl::destroy()
{
	Images.clear();
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
	// TODO draw gridlines
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

Transform& MachineImpl::canvas_transform() const
{
	return easel->canvas.transform();
}

void MachineImpl::sync_canvas_transform() const
{
	easel->sync_canvas_transform();
}

void MachineImpl::sync_canvas_transform_p() const
{
	easel->sync_canvas_transform_p();
}

void MachineImpl::sync_canvas_transform_rs() const
{
	easel->sync_canvas_transform_rs();
}

bool MachineImpl::cursor_in_easel() const
{
	return easel->cursor_in_clipping();
}

void MachineImpl::set_easel_scale(float sx, float sy) const
{
	easel->set_app_scale(sx, sy);
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

bool MachineImpl::open_file()
{
	if (unsaved)
	{
		// LATER ask if user wants to save
	}
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	const char* openfile = tinyfd_openFileDialog("Open file", "", num_filters, filters, "", false);
	if (!openfile) return false;
	current_filepath = openfile;
	mark();
	open_file(openfile);
	return true;
}

bool MachineImpl::import_file()
{
	static const int num_filters = 4;
	static const char* const filters[num_filters] = { "*.png", "*.gif", "*.jpg", "*.bmp" };
	const char* importfile = tinyfd_openFileDialog("Import file", "", num_filters, filters, "", false);
	if (!importfile) return false;
	mark();
	import_file(importfile);
	return true;
}

bool MachineImpl::export_file() const
{
	static const int num_filters = 4;
	static const char* const filters[num_filters] = { "*.png", "*.gif", "*.jpg", "*.bmp" };
	// LATER open custom dialog for export settings first
	const char* exportfile = tinyfd_saveFileDialog("Export file", "", num_filters, filters, "");
	if (!exportfile) return false;
	// LATER actually export file
	return true;
}

bool MachineImpl::save_file()
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	if (current_filepath.empty())
	{
		const char* savefile = tinyfd_saveFileDialog("Save file", "", num_filters, filters, "");
		if (!savefile) return false;
		// LATER create new file
		current_filepath = savefile;
	}
	save_file(current_filepath.c_str());
	unmark();
	return true;
}

bool MachineImpl::save_file_as()
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	const char* savefile = tinyfd_saveFileDialog("Save file as", "", num_filters, filters, "");
	if (!savefile) return false;
	// LATER create new file
	current_filepath = savefile;
	save_file(savefile);
	unmark();
	return true;
}

bool MachineImpl::save_file_copy()
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	const char* savefile = tinyfd_saveFileDialog("Save file copy", "", num_filters, filters, "");
	if (!savefile) return false;
	// LATER create new file
	save_file(savefile);
	unmark();
	return true;
}

void MachineImpl::open_file(const char* filepath)
{
	// LATER actually open quasar file
}

void MachineImpl::import_file(const char* filepath)
{
	// LATER register instead to ensure unique? or use secondary registry specifically for canvas_image
	auto img = Images.construct(ImageConstructor(filepath));
	easel->set_canvas_image(img);
	canvas_reset_camera();
}

void MachineImpl::save_file(const char* filepath)
{
	// LATER actually save changes to filepath
}

void MachineImpl::canvas_begin_panning()
{
	if (!panning)
	{
		pan_initial_view_pos = canvas_position();
		pan_initial_cursor_pos = easel->get_app_cursor_pos();
		panning = true;
		main_window->override_gui_cursor_change(true);
		main_window->set_cursor(create_cursor(StandardCursor::RESIZE_OMNI));
	}
}

void MachineImpl::canvas_end_panning()
{
	if (panning)
	{
		panning = false;
		main_window->override_gui_cursor_change(false);
		main_window->set_cursor(create_cursor(StandardCursor::ARROW));
		main_window->set_mouse_mode(MouseMode::VISIBLE);
	}
}

void MachineImpl::canvas_zoom_by(float z)
{
	Position cursor_world;
	if (!main_window->is_ctrl_pressed())
		cursor_world = easel->to_world_coordinates(main_window->cursor_pos());

	float factor = main_window->is_shift_pressed() ? zoom_factor_shift : zoom_factor;
	float new_zoom = std::clamp(zoom * glm::pow(factor, z), zoom_in_min, zoom_in_max);
	float zoom_change = new_zoom / zoom;
	canvas_scale() *= zoom_change;
	Position delta_position = (canvas_position() - cursor_world) * zoom_change;
	canvas_position() = cursor_world + delta_position;

	sync_canvas_transform();
	zoom = new_zoom;
}

void MachineImpl::canvas_update_panning() const
{
	if (panning)
	{
		Position pan_delta = easel->get_app_cursor_pos() - pan_initial_cursor_pos;
		Position pos = pan_delta + pan_initial_view_pos;
		if (main_window->is_shift_pressed())
		{
			if (std::abs(pan_delta.x) < std::abs(pan_delta.y))
				pos.x = pan_initial_view_pos.x;
			else
				pos.y = pan_initial_view_pos.y;
		}
		canvas_position() = pos;
		sync_canvas_transform_p();

		if (main_window->mouse_mode() != MouseMode::VIRTUAL && !easel->cursor_in_clipping())
			main_window->set_mouse_mode(MouseMode::VIRTUAL);
	}
}

void MachineImpl::flip_horizontally()
{
	static Action a([this]() { easel->canvas.image->flip_horizontally(); mark(); }, [this]() { easel->canvas.image->flip_horizontally(); mark(); });
	history.execute(a);
}

void MachineImpl::flip_vertically()
{
	static Action a([this]() { easel->canvas.image->flip_vertically(); mark(); }, [this]() { easel->canvas.image->flip_vertically(); mark(); });
	history.execute(a);
}

void MachineImpl::rotate_180()
{
	static Action a([this]() { easel->canvas.image->rotate_180(); mark(); }, [this]() { easel->canvas.image->rotate_180(); mark(); });
	history.execute(a);
}

void MachineImpl::canvas_reset_camera()
{
	zoom = zoom_initial;
	canvas_transform() = {};
	if (easel->canvas.image)
	{
		float fit_scale = std::min(easel->get_app_width() / easel->canvas.image->width, easel->get_app_height() / easel->canvas.image->height);
		if (fit_scale < 1.0f)
			canvas_scale() *= fit_scale;
		zoom *= fit_scale;
	}
	sync_canvas_transform();
}

bool MachineImpl::minor_gridlines_visible()
{
	return easel->minor_gridlines_visible;
}

void MachineImpl::show_minor_gridlines()
{
	easel->minor_gridlines_visible = true;
}

void MachineImpl::hide_minor_gridlines()
{
	easel->minor_gridlines_visible = false;
}

bool MachineImpl::major_gridlines_visible()
{
	return easel->major_gridlines_visible;
}

void MachineImpl::show_major_gridlines()
{
	easel->major_gridlines_visible = true;
}

void MachineImpl::hide_major_gridlines()
{
	easel->major_gridlines_visible = false;
}
