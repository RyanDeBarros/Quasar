#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "pipeline/Sprite.h"
#include "UserInput.h"
#include "GUI.h"

static Sprite* easel_background = nullptr;

struct Checkerboard : public Sprite
{
	RGBA c1, c2;

	Checkerboard(RGBA c1, RGBA c2)
		: c1(c1), c2(c2)
	{
		Image img;
		img.width = 2;
		img.height = 2;
		img.chpp = 4;
		img.pixels = new Image::Byte[img.area()];
		img.gen_texture();
		set_image(Images.add(std::move(img)));
		sync_colors();
		set_uv_size(0, 0);
	}

	void sync_colors() const
	{
		Image* img = Images.get(image);
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
		sync_texture();
	}

	void sync_texture() const
	{
		Image* img = Images.get(image);
		img->resend_texture();
		TextureParams tparams;
		tparams.wrap_s = TextureWrap::Repeat;
		tparams.wrap_t = TextureWrap::Repeat;
		img->update_texture_params(tparams);
	}

	void set_uv_size(float width, float height) const
	{
		set_uvs(Bounds{ 0.0f, width, 0.0f, height });
		sync_texture();
	}
};

struct Canvas
{
	Image* image = nullptr;
	Sprite sprite;
	Checkerboard checkerboard;
	float checker_size = 16.0f; // TODO settings

	Canvas(RGBA c1, RGBA c2)
		: checkerboard(c1, c2)
	{
		set_image(ImageHandle(0));
	}

	void set_image(ImageHandle img)
	{
		sprite.set_image(img);
		image = Images.get(img);
		if (image)
		{
			checkerboard.set_uv_size(0.5f * image->width / checker_size, 0.5f * image->height / checker_size);
			checkerboard.set_modulation(ColorFrame());
		}
		else
		{
			checkerboard.set_modulation(ColorFrame(0));
		}
	}

	Transform& transform() { return sprite.transform; }
	Position& position() { return sprite.transform.position; }
	Rotation& rotation() { return sprite.transform.rotation; }
	Scale& scale() { return sprite.transform.scale; }

	void sync_transform()
	{
		sprite.sync_transform();
		checkerboard.transform = sprite.transform;
		if (image)
			checkerboard.transform.scale *= glm::vec2{ image->width * 0.5f, image->height * 0.5f };
		checkerboard.sync_transform();
	}

	void sync_transform_p()
	{
		sprite.sync_transform_p();
		checkerboard.transform.position = sprite.transform.position;
		checkerboard.sync_transform_p();
	}

	void sync_transform_rs()
	{
		sprite.sync_transform_rs();
		checkerboard.transform.rotation = sprite.transform.rotation;
		if (image)
			checkerboard.transform.scale = sprite.transform.scale * glm::vec2{ image->width * 0.5f, image->height * 0.5f };
		checkerboard.sync_transform_rs();
	}
};

static Canvas* canvas = nullptr;

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

	canvas = new Canvas(RGBA(HSV(0.5f, 0.2f, 0.2f).to_rgb(), 0.5f), RGBA(HSV(0.5f, 0.3f, 0.3f).to_rgb(), 0.5f));
	canvas_renderer->sprites().push_back(&canvas->checkerboard);
	canvas_renderer->sprites().push_back(&canvas->sprite);

	canvas_reset_camera();
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
	delete canvas;
	canvas = nullptr;
	delete easel_background;
	easel_background = nullptr;
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
		if (*iter == &canvas->sprite)
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
	canvas->set_image(img);
	canvas_reset_camera();
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
		pan_initial_cursor_pos = canvas_renderer->get_app_cursor_pos();
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
		float fit_scale = std::min(canvas_renderer->get_app_width() / canvas->image->width, canvas_renderer->get_app_height() / canvas->image->height);
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
		Position pan_delta = canvas_renderer->get_app_cursor_pos() - pan_initial_cursor_pos;
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

		if (main_window->mouse_mode() != MouseMode::VIRTUAL && !canvas_renderer->cursor_in_clipping())
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
