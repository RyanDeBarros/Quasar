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

Scale app_inverse_scale{};
static PanelGroup* panels = nullptr;

static Easel* easel()
{
	return dynamic_cast<Easel*>(panels->panels[0].get());
}

static Palette* palette()
{
	return dynamic_cast<Palette*>(panels->panels[1].get());
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

bool MachineImpl::create_main_window()
{
	main_window = new Window("Quasar", window_layout_info.initial_width, window_layout_info.initial_height);
	if (main_window)
	{
		update_raw_mouse_motion();
		update_vsync();
		main_window->set_size_limits(window_layout_info.initial_brush_panel_width + window_layout_info.initial_brush_panel_width,
			//window_layout_info.initial_menu_panel_height + window_layout_info.initial_views_panel_height, GLFW_DONT_CARE, GLFW_DONT_CARE);
			window_layout_info.initial_height, GLFW_DONT_CARE, GLFW_DONT_CARE); // LATER add status bar at bottom of window. also, add min/max limits to individual panels, and add up here.

		main_window->root_window_size.children.push_back(&resize_handler);
		main_window->root_display_scale.children.push_back(&rescale_handler);
		main_window->root_mouse_button.children.push_back(&easel_mb_handler);
		main_window->root_mouse_button.children.push_back(&palette_mb_handler);
		main_window->root_key.children.push_back(&palette_key_handler);
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
	
	TextRender::load_shader();
	Fonts::load_common_fonts();

	panels = new PanelGroup();
	panels->panels.push_back(std::make_unique<Easel>());
	panels->panels.push_back(std::make_unique<Palette>());

	easel()->bounds.x1 = window_layout_info.initial_brush_panel_width;
	easel()->bounds.x2 = window_layout_info.initial_width - window_layout_info.initial_palette_panel_width;
	easel()->bounds.y1 = window_layout_info.initial_views_panel_height;
	easel()->bounds.y2 = window_layout_info.initial_height - window_layout_info.initial_menu_panel_height;
	palette()->bounds.x1 = easel()->bounds.x2;
	palette()->bounds.x2 = window_layout_info.initial_width;
	palette()->bounds.y1 = easel()->bounds.y1;
	palette()->bounds.y2 = easel()->bounds.y2;

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

	import_file(FileSystem::workspace_path("ex/flag.png"));
	//show_major_gridlines();

	const Buffer& cpy = Fonts::roboto_regular->common_texture.buf;
	std::shared_ptr<Image> img = std::make_shared<Image>();
	img->buf.chpp = cpy.chpp;
	img->buf.width = cpy.width;
	img->buf.height = cpy.height;
	img->buf.pxnew();
	subbuffer_copy(img->buf, cpy);
	img->gen_texture(TextureParams::linear);
	easel()->set_canvas_image(std::move(img));
	canvas_reset_camera();
}

void MachineImpl::destroy()
{
	// NOTE no Image shared_ptrs should remain before destroying window.
	QUASAR_INVALIDATE_PTR(panels);
	TextRender::invalidate_shader();
	Fonts::invalidate_common_fonts();
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
	panels->render();
	main_window_clip().scissor();
	render_main_menu_bar();
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
	return easel()->canvas.transform();
}

void MachineImpl::sync_canvas_transform() const
{
	easel()->sync_canvas_transform();
}

bool MachineImpl::canvas_image_ready() const
{
	return easel()->canvas_image();
}

Scale MachineImpl::inv_app_scale() const
{
	return app_inverse_scale;
}

Scale MachineImpl::get_app_scale() const
{
	return 1.0f / app_inverse_scale;
}

void MachineImpl::set_app_scale(Scale scale) const
{
	app_inverse_scale = 1.0f / scale;
	panels->set_projection();
	float scale1d = (scale.x + scale.y + std::max(scale.x, scale.y)) / 3.0f;
	static const float gui_scale_factor = 1.25f; // SETTINGS
	float gui_scale = scale1d * gui_scale_factor;
	ImGui::GetStyle().ScaleAllSizes(gui_scale);
	ImGui::GetIO().FontGlobalScale = gui_scale;
}

void MachineImpl::set_clear_color(ColorFrame color)
{
	auto vec = color.rgba().as_vec();
	QUASAR_GL(glClearColor(vec[0], vec[1], vec[2], vec[3]));
}

glm::vec2 MachineImpl::easel_cursor_world_pos() const
{
	return easel()->to_world_coordinates(main_window->cursor_pos());
}

glm::vec2 MachineImpl::palette_cursor_world_pos() const
{
	return palette()->to_world_coordinates(main_window->cursor_pos());
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

void MachineImpl::canvas_begin_panning()
{
	if (!panning_info.panning)
	{
		panning_info.initial_canvas_pos = canvas_position();
		panning_info.initial_cursor_pos = easel()->get_app_cursor_pos();
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
		Position pan_delta = easel()->get_app_cursor_pos() - panning_info.initial_cursor_pos;
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

		if (main_window->mouse_mode() != MouseMode::VIRTUAL && !easel()->cursor_in_clipping())
			main_window->set_mouse_mode(MouseMode::VIRTUAL);
		// LATER weirdly, virtual mouse is actually slower to move than visible mouse, so when virtual, scale deltas accordingly.
		// put factor in settings, and possibly even allow 2 speeds, with holding ALT or something.
	}
}

void MachineImpl::canvas_zoom_by(float z)
{
	Position cursor_world;
	if (!main_window->is_alt_pressed())
		cursor_world = easel()->to_world_coordinates(main_window->cursor_pos());

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
		[this]() { easel()->canvas_image()->flip_horizontally(); },
		[this]() { easel()->canvas_image()->flip_horizontally(); }
	));
	history.execute(a);
}

void MachineImpl::flip_vertically()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel()->canvas_image()->flip_vertically(); },
		[this]() { easel()->canvas_image()->flip_vertically(); }
	));
	history.execute(a);
}

void MachineImpl::rotate_90()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel()->canvas_image()->rotate_90(); easel()->update_canvas_image(); },
		[this]() { easel()->canvas_image()->rotate_270(); easel()->update_canvas_image(); })
	);
	history.execute(a);
}

void MachineImpl::rotate_180()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel()->canvas_image()->rotate_180(); },
		[this]() { easel()->canvas_image()->rotate_180(); }
	));
	history.execute(a);
}

void MachineImpl::rotate_270()
{
	static std::shared_ptr<StandardAction> a(std::make_shared<StandardAction>(
		[this]() { easel()->canvas_image()->rotate_270(); easel()->update_canvas_image(); },
		[this]() { easel()->canvas_image()->rotate_90(); easel()->update_canvas_image(); }
	));
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

void MachineImpl::canvas_reset_camera()
{
	zoom_info.zoom = zoom_info.initial;
	canvas_transform() = {};
	if (easel()->canvas_image())
	{
		float fit_scale = std::min(easel()->get_app_width() / easel()->canvas_image()->buf.width, easel()->get_app_height() / easel()->canvas_image()->buf.height);
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
	return easel()->minor_gridlines_are_visible();
}

void MachineImpl::show_minor_gridlines()
{
	easel()->set_minor_gridlines_visibility(true);
}

void MachineImpl::hide_minor_gridlines()
{
	easel()->set_minor_gridlines_visibility(false);
}

bool MachineImpl::major_gridlines_visible()
{
	return easel()->major_gridlines_are_visible();
}

void MachineImpl::show_major_gridlines()
{
	easel()->set_major_gridlines_visibility(true);
}

void MachineImpl::hide_major_gridlines()
{
	easel()->set_major_gridlines_visibility(false);
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
