#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "pipeline/Sprite.h"
#include "UserInput.h"

void MachineImpl::init_renderer()
{
	canvas_renderer = new Renderer(main_window, Shader());
	
	canvas_background = new Sprite();
	canvas_background->set_image(ImageHandle(0), 1, 1);
	canvas_background->set_modulation(ColorFrame(HSV(0.5f, 0.2f, 0.2f), 0.5f));
	canvas_renderer->sprites().push_back(canvas_background);
	auto canvas_background_resize = [this](const Callback::WindowSize& ws) {
		canvas_background->transform.scale = { float(ws.width), float(ws.height) };
		canvas_background->sync_transform_rs();
		};
	canvas_background_resize(Callback::WindowSize(main_window->width(), main_window->height()));
	main_window->clbk_window_size.push_back(std::move(canvas_background_resize));
	
	canvas_sprite = new Sprite();
	canvas_sprite->set_image(ImageHandle(0));
	canvas_renderer->sprites().push_back(canvas_sprite);

	canvas_renderer->set_window_resize_callback();
	attach_canvas_controls();
	attach_global_user_controls();
}

void MachineImpl::destroy()
{
	images.clear();
	shaders.clear();
	delete canvas_renderer;
	canvas_renderer = nullptr;
	delete main_window;
	main_window = nullptr;
	delete canvas_sprite;
	canvas_sprite = nullptr;
	delete canvas_background;
	canvas_background = nullptr;
}

void MachineImpl::mark()
{
	unsaved = true;
	// TODO edit title to include (*)
}

void MachineImpl::unmark()
{
	unsaved = false;
	// TODO remove (*) from title if it exists
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
	for (auto iter = canvas_renderer->sprites().begin(); iter != canvas_renderer->sprites().end(); ++iter)
	{
		if (*iter == canvas_sprite)
		{
			canvas_renderer->sprites().erase(iter);
			break;
		}
	}
	mark();
	// TODO clear palletes/frames/layers/etc.
	return true;
}

bool MachineImpl::open_file()
{
	if (unsaved)
	{
		// TODO ask if user wants to save
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
	// TODO open custom dialog for export settings first
	const char* exportfile = tinyfd_saveFileDialog("Export file", "", num_filters, filters, "");
	if (!exportfile) return false;
	// TODO actually export file
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
		// TODO create new file
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
	// TODO create new file
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
	// TODO create new file
	save_file(savefile);
	unmark();
	return true;
}

void MachineImpl::open_file(const char* filepath)
{
	// TODO actually open quasar file
}

void MachineImpl::import_file(const char* filepath)
{
	// TODO register instead to ensure unique? or use secondary registry specifically for canvas_image
	auto img = images.construct(ImageConstructor(filepath));
	canvas_sprite->set_image(img);
	canvas_image = images.get(img);
}

void MachineImpl::save_file(const char* filepath)
{
	// TODO actually save changes to filepath
}

void MachineImpl::on_update()
{
	canvas_update_panning();
}

void MachineImpl::canvas_begin_panning()
{
	if (!panning)
	{
		pan_initial_view_pos = canvas_sprite->transform.position;
		pan_initial_cursor_pos = canvas_renderer->get_app_scale() * main_window->cursor_pos();
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
	glm::vec2 cursor_world;
	if (main_window->is_ctrl_pressed())
		cursor_world = canvas_renderer->to_world_coordinates(canvas_renderer->clipping_rect().center_point());
	else
		cursor_world = canvas_renderer->to_world_coordinates(main_window->cursor_pos());

	float factor = main_window->is_shift_pressed() ? zoom_factor_shift : zoom_factor;
	float new_zoom = std::clamp(zoom * glm::pow(factor, z), zoom_in_min, zoom_in_max);
	canvas_sprite->transform.scale *= new_zoom / zoom;
	
	glm::vec2 delta_position = (canvas_sprite->transform.position - cursor_world) * new_zoom / zoom;
	canvas_sprite->transform.position = cursor_world + delta_position;
	canvas_sprite->sync_transform();

	zoom = new_zoom;
}

void MachineImpl::canvas_reset_camera()
{
	canvas_sprite->transform = {};
	canvas_sprite->sync_transform();
	zoom = zoom_initial;
}

void MachineImpl::canvas_update_panning()
{
	if (panning)
	{
		glm::vec2 pan_delta = canvas_renderer->get_app_scale() * main_window->cursor_pos() - pan_initial_cursor_pos;
		glm::vec2 pos = pan_delta + pan_initial_view_pos;
		if (main_window->is_shift_pressed())
		{
			if (std::abs(pan_delta.x) < std::abs(pan_delta.y))
				pos.x = pan_initial_view_pos.x;
			else
				pos.y = pan_initial_view_pos.y;
		}
		canvas_sprite->transform.position = pos;
		canvas_sprite->sync_transform_p();

		if (main_window->mouse_mode() != MouseMode::VIRTUAL && !canvas_renderer->cursor_in_clipping())
			main_window->set_mouse_mode(MouseMode::VIRTUAL);
	}
}

void MachineImpl::flip_horizontally()
{
	static Action a([this]() { canvas_image->flip_horizontally(); mark(); }, [this]() { canvas_image->flip_horizontally(); mark(); });
	history.execute(a);
}

void MachineImpl::flip_vertically()
{
	static Action a([this]() { canvas_image->flip_vertically(); mark(); }, [this]() { canvas_image->flip_vertically(); mark(); });
	history.execute(a);
}

void MachineImpl::rotate_180()
{
	static Action a([this]() { canvas_image->rotate_180(); mark(); }, [this]() { canvas_image->rotate_180(); mark(); });
	history.execute(a);
}
