#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "UserInput.h"
#include "GUI.h"
#include "pipeline/Renderer.h"
#include "pipeline/Easel.h"

#define QUASAR_INVALIDATE_PTR(ptr) delete ptr; ptr = nullptr;
#define QUASAR_INVALIDATE_ARR(arr) delete[] arr; arr = nullptr;

static Sprite* easel_background = nullptr;
static Canvas* canvas = nullptr;
static Renderer* easel_renderer = nullptr;

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
	easel_renderer = new Renderer(main_window, Shader("res/standard.vert", "res/standard.frag", { 1, 2, 2, 2, 4, 4 }, { "u_VP" })); // LATER don't really need modulation in future
	
	easel_background = new Sprite();
	easel_background->set_image(ImageHandle(0), 1, 1);
	easel_background->set_modulation(ColorFrame(HSV(0.5f, 0.15f, 0.15f), 0.5f));
	easel_renderer->sprites().push_back(easel_background);
	auto canvas_background_resize = [this](const Callback::WindowSize& ws) {
		easel_background->transform.scale = { float(ws.width), float(ws.height) };
		easel_background->sync_transform_rs();
		};
	canvas_background_resize(Callback::WindowSize(main_window->width(), main_window->height()));
	main_window->clbk_window_size.push_back(std::move(canvas_background_resize));

	canvas = new Canvas(RGBA(HSV(0.5f, 0.2f, 0.2f).to_rgb(), 0.5f), RGBA(HSV(0.5f, 0.3f, 0.3f).to_rgb(), 0.5f));
	easel_renderer->sprites().push_back(&canvas->checkerboard);
	easel_renderer->sprites().push_back(&canvas->sprite);

	canvas_reset_camera();
	easel_renderer->set_window_resize_callback();
	attach_canvas_controls();
	attach_global_user_controls();

	easel_renderer->clipping_rect().window_size_to_bounds = [](int w, int h) -> glm::ivec4 { return {
		w / 10, h / 10, 8 * w / 10, 8 * h / 10
	}; };
	easel_renderer->clipping_rect().update_window_size(main_window->width(), main_window->height());

}

void MachineImpl::destroy()
{
	Images.clear();
	QUASAR_INVALIDATE_PTR(easel_renderer);
	QUASAR_INVALIDATE_PTR(canvas);
	QUASAR_INVALIDATE_PTR(easel_background);
	QUASAR_INVALIDATE_PTR(main_window); // invalidate window last
}

bool MachineImpl::should_exit() const
{
	return main_window->should_close();
}

void MachineImpl::on_render()
{
	canvas_update_panning();
	main_window->new_frame();
	easel_renderer->frame_cycle();
	draw_gridlines();
	render_gui();
	main_window->end_frame();
}

void MachineImpl::draw_gridlines()
{
	// TODO gridlines
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
	return canvas->transform();
}

void MachineImpl::sync_canvas_transform() const
{
	canvas->sync_transform();
}

void MachineImpl::sync_canvas_transform_p() const
{
	canvas->sync_transform_p();
}

void MachineImpl::sync_canvas_transform_rs() const
{
	canvas->sync_transform_rs();
}

bool MachineImpl::cursor_in_easel() const
{
	return easel_renderer->cursor_in_clipping();
}

void MachineImpl::set_easel_scale(float sx, float sy) const
{
	easel_renderer->set_app_scale(sx, sy);
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
	for (auto iter = easel_renderer->sprites().begin(); iter != easel_renderer->sprites().end(); ++iter)
	{
		if (*iter == &canvas->sprite)
		{
			easel_renderer->sprites().erase(iter);
			break;
		}
	}
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
	canvas->set_image(img);
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
		pan_initial_cursor_pos = easel_renderer->get_app_cursor_pos();
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
		cursor_world = easel_renderer->to_world_coordinates(main_window->cursor_pos());

	float factor = main_window->is_shift_pressed() ? zoom_factor_shift : zoom_factor;
	float new_zoom = std::clamp(zoom * glm::pow(factor, z), zoom_in_min, zoom_in_max);
	float zoom_change = new_zoom / zoom;
	canvas_scale() *= zoom_change;
	Position delta_position = (canvas_position() - cursor_world) * zoom_change;
	canvas_position() = cursor_world + delta_position;

	sync_canvas_transform();
	zoom = new_zoom;
}

void MachineImpl::canvas_reset_camera()
{
	zoom = zoom_initial;
	canvas_transform() = {};
	if (canvas->image)
	{
		float fit_scale = std::min(easel_renderer->get_app_width() / canvas->image->width, easel_renderer->get_app_height() / canvas->image->height);
		if (fit_scale < 1.0f)
			canvas_scale() *= fit_scale;
		zoom *= fit_scale;
	}
	sync_canvas_transform();
}

void MachineImpl::canvas_update_panning() const
{
	if (panning)
	{
		Position pan_delta = easel_renderer->get_app_cursor_pos() - pan_initial_cursor_pos;
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

		if (main_window->mouse_mode() != MouseMode::VIRTUAL && !easel_renderer->cursor_in_clipping())
			main_window->set_mouse_mode(MouseMode::VIRTUAL);
	}
}

void MachineImpl::flip_horizontally()
{
	static Action a([this]() { canvas->image->flip_horizontally(); mark(); }, [this]() { canvas->image->flip_horizontally(); mark(); });
	history.execute(a);
}

void MachineImpl::flip_vertically()
{
	static Action a([this]() { canvas->image->flip_vertically(); mark(); }, [this]() { canvas->image->flip_vertically(); mark(); });
	history.execute(a);
}

void MachineImpl::rotate_180()
{
	static Action a([this]() { canvas->image->rotate_180(); mark(); }, [this]() { canvas->image->rotate_180(); mark(); });
	history.execute(a);
}
