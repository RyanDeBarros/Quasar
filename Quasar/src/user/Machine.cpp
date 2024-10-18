#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "pipeline/Sprite.h"
#include "UserInput.h"
#include "GUI.h"

static Sprite* easel_background = nullptr;
static Sprite* canvas_sprite = nullptr;

struct Checkerboard
{
	Sprite sprite;
	RGBA c1, c2;
	Transform transform;

	Checkerboard(RGBA c1, RGBA c2)
		: c1(c1), c2(c2)
	{
		Image img;
		img.width = 2;
		img.height = 2;
		img.chpp = 4;
		img.pixels = new Image::Byte[img.area()];
		TextureParams chkp;
		chkp.wrap_s = TextureWrap::Repeat;
		chkp.wrap_t = TextureWrap::Repeat;
		img.gen_texture(chkp);
		sprite.set_image(Images.add(std::move(img)));
		sync_colors();
	}

	void sync_colors() const
	{
		Image* img = Images.get(sprite.image);
		if (img)
		{
			for (size_t i = 0; i < 2; ++i)
			{
				img->pixels[0 + 12 * i] = c1.rgb.r;
				img->pixels[1 + 12 * i] = c1.rgb.g;
				img->pixels[2 + 12 * i] = c1.rgb.b;
				img->pixels[3 + 12 * i] = c1.alpha;
			}
			for (size_t i = 0; i < 2; ++i)
			{
				img->pixels[4 + 4 * i] = c2.rgb.r;
				img->pixels[5 + 4 * i] = c2.rgb.g;
				img->pixels[6 + 4 * i] = c2.rgb.b;
				img->pixels[7 + 4 * i] = c2.alpha;
			}
			img->resend_texture();
		}
	}

	void sync_transform()
	{
		sprite.transform.position = transform.position;
		sprite.transform.rotation = transform.rotation;
		sprite.transform.scale = transform.scale * 16.0f;
		sprite.sync_transform();
	}

	void sync_transform_p()
	{
		sprite.transform.position = transform.position;
		sprite.sync_transform_p();
	}

	void sync_transform_rs()
	{
		sprite.transform.rotation = transform.rotation;
		sprite.transform.scale = transform.scale * 16.0f;
		sprite.sync_transform_rs();
	}
};

static Checkerboard* canvas_checkerboard = nullptr;


bool MachineImpl::create_main_window()
{
	main_window = new Window("Quasar", 1440, 1080, true);
	if (main_window)
	{
		main_window->set_raw_mouse_motion(true); // TODO settable from user settings
		return true;
	}
	return false;
}

void MachineImpl::init_renderer()
{
	canvas_renderer = new Renderer(main_window, Shader());
	
	easel_background = new Sprite();
	easel_background->set_image(ImageHandle(0), 1, 1);
	easel_background->set_modulation(ColorFrame(HSV(0.5f, 0.15f, 0.15f), 0.5f));
	canvas_renderer->sprites().push_back(easel_background);
	auto canvas_background_resize = [this](const Callback::WindowSize& ws) {
		easel_background->transform.scale = { float(ws.width), float(ws.height) };
		easel_background->sync_transform_rs();
		};
	canvas_background_resize(Callback::WindowSize(main_window->width(), main_window->height()));
	main_window->clbk_window_size.push_back(std::move(canvas_background_resize));

	canvas_checkerboard = new Checkerboard(RGBA(HSV(0.5f, 0.2f, 0.2f).to_rgb(), 0.5f), RGBA(HSV(0.5f, 0.3f, 0.3f).to_rgb(), 0.5f));
	canvas_renderer->sprites().push_back(&canvas_checkerboard->sprite);

	canvas_sprite = new Sprite();
	canvas_sprite->set_image(ImageHandle(0));
	canvas_renderer->sprites().push_back(canvas_sprite);

	set_canvas_transform({});
	canvas_renderer->set_window_resize_callback();
	attach_canvas_controls();
	attach_global_user_controls();
}

void MachineImpl::destroy()
{
	Images.clear();
	Shaders.clear();
	delete canvas_renderer;
	canvas_renderer = nullptr;
	delete main_window;
	main_window = nullptr;
	delete canvas_sprite;
	canvas_sprite = nullptr;
	delete easel_background;
	easel_background = nullptr;
	delete canvas_checkerboard;
	canvas_checkerboard = nullptr;
}

bool MachineImpl::should_exit() const
{
	return main_window->should_close();
}

void MachineImpl::on_render()
{
	canvas_update_panning();
	main_window->new_frame();
	canvas_renderer->frame_cycle();
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
	// TODO edit title to include (*)
}

void MachineImpl::unmark()
{
	unsaved = false;
	// TODO remove (*) from title if it exists
}

Transform MachineImpl::canvas_transform() const
{
	return canvas_sprite->transform;
}

void MachineImpl::set_canvas_transform(Transform transform) const
{
	canvas_sprite->transform = transform;
	canvas_sprite->sync_transform();
	canvas_checkerboard->transform = transform;
	canvas_checkerboard->sync_transform();
}

void MachineImpl::set_canvas_position(Position position) const
{
	canvas_sprite->transform.position = position;
	canvas_sprite->sync_transform_p();
	canvas_checkerboard->transform.position = position;
	canvas_checkerboard->sync_transform_p();
}

void MachineImpl::set_canvas_rotation_scale(Rotation rotation, Scale scale) const
{
	canvas_sprite->transform.rotation = rotation;
	canvas_sprite->transform.scale = scale;
	canvas_sprite->sync_transform_rs();
	canvas_checkerboard->transform.rotation = rotation;
	canvas_checkerboard->transform.scale = scale;
	canvas_checkerboard->sync_transform_rs();
}

void MachineImpl::set_canvas_rotation(Rotation rotation) const
{
	canvas_sprite->transform.rotation = rotation;
	canvas_sprite->sync_transform_rs();
	canvas_checkerboard->transform.rotation = rotation;
	canvas_checkerboard->sync_transform_rs();
}

void MachineImpl::set_canvas_scale(Scale scale) const
{
	canvas_sprite->transform.scale = scale;
	canvas_sprite->sync_transform_rs();
	canvas_checkerboard->transform.scale = scale;
	canvas_checkerboard->sync_transform_rs();
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
	auto img = Images.construct(ImageConstructor(filepath));
	canvas_sprite->set_image(img);
	canvas_image = Images.get(img);
}

void MachineImpl::save_file(const char* filepath)
{
	// TODO actually save changes to filepath
}

void MachineImpl::canvas_begin_panning()
{
	if (!panning)
	{
		pan_initial_view_pos = canvas_position();
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
	Position cursor_world;
	if (!main_window->is_ctrl_pressed())
		cursor_world = canvas_renderer->to_world_coordinates(main_window->cursor_pos());

	float factor = main_window->is_shift_pressed() ? zoom_factor_shift : zoom_factor;
	float new_zoom = std::clamp(zoom * glm::pow(factor, z), zoom_in_min, zoom_in_max);
	float zoom_change = new_zoom / zoom;
	Scale scale = canvas_scale() * zoom_change;
	Position delta_position = (canvas_position() - cursor_world) * zoom_change;
	Position position = cursor_world + delta_position;

	set_canvas_transform({ position, 0, scale });
	zoom = new_zoom;
}

void MachineImpl::canvas_reset_camera()
{
	set_canvas_transform({});
	zoom = zoom_initial;
}

void MachineImpl::canvas_update_panning() const
{
	if (panning)
	{
		Position pan_delta = canvas_renderer->get_app_scale() * main_window->cursor_pos() - pan_initial_cursor_pos;
		Position pos = pan_delta + pan_initial_view_pos;
		if (main_window->is_shift_pressed())
		{
			if (std::abs(pan_delta.x) < std::abs(pan_delta.y))
				pos.x = pan_initial_view_pos.x;
			else
				pos.y = pan_initial_view_pos.y;
		}
		set_canvas_position(pos);

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
