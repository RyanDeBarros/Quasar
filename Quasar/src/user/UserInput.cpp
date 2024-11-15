#include "UserInput.h"

#include "Machine.h"
#include "variety/Utils.h"

// LATER put these as data members on Easel, and put these functions there. Honestly, get rid of UserInput.h/cpp altogether.
void attach_canvas_controls()
{
	// Panning
	Machine.easel_mb_handler.callback = [](const MouseButtonEvent& mb) {
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == IAction::PRESS && Machine.cursor_in_easel())
			{
				mb.consumed = true;
				Machine.canvas_begin_panning();
			}
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_end_panning();
		}
		else if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == IAction::PRESS && Machine.cursor_in_easel() && Machine.main_window->is_key_pressed(Key::SPACE))
			{
				mb.consumed = true;
				Machine.canvas_begin_panning();
			}
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_end_panning();
		}
		};
	// Zooming
	Machine.easel_scroll_handler.callback = [](const ScrollEvent& s) {
		if (Machine.cursor_in_easel() && !Machine.panning_info.panning)
		{
			Machine.canvas_zoom_by(s.yoff);
			s.consumed = true;
		}
		};
}

void attach_global_user_controls()
{
	Machine.global_key_handler.callback = [](const KeyEvent& k) {
		if (k.action == IAction::PRESS)
		{
			switch (k.key)
			{
			case Key::ROW0:
				k.consumed = true;
				Machine.canvas_reset_camera();
				break;
			case Key::Z:
				if (Machine.main_window->is_ctrl_pressed())
				{
					k.consumed = true;
					if (Machine.main_window->is_shift_pressed())
					{
						if (Machine.redo_enabled())
						{
							Machine.start_held_redo();
							Machine.redo();
						}
					}
					else
					{
						if (Machine.undo_enabled())
						{
							Machine.start_held_undo();
							Machine.undo();
						}
					}
				}
				break;
			case Key::N:
				if (Machine.main_window->is_ctrl_pressed())
				{
					k.consumed = true;
					Machine.new_file();
				}
				break;
			case Key::O:
				if (Machine.main_window->is_ctrl_pressed())
				{
					k.consumed = true;
					Machine.open_file();
				}
				break;
			case Key::I:
				if (Machine.main_window->is_ctrl_pressed())
				{
					k.consumed = true;
					Machine.import_file();
				}
				break;
			case Key::E:
				if (Machine.main_window->is_ctrl_pressed())
				{
					k.consumed = true;
					Machine.export_file();
				}
				break;
			case Key::S:
				if (Machine.main_window->is_ctrl_pressed())
				{
					if (Machine.main_window->is_shift_pressed())
					{
						if (!Machine.main_window->is_alt_pressed())
						{
							k.consumed = true;
							Machine.save_file_as();
						}
					}
					else if (Machine.main_window->is_alt_pressed())
					{
						k.consumed = true;
						Machine.save_file_copy();
					}
					else
					{
						k.consumed = true;
						Machine.save_file();
					}
				}
				break;
			case Key::G:
				k.consumed = true;
				if (Machine.main_window->is_shift_pressed())
				{
					if (Machine.major_gridlines_visible())
						Machine.hide_major_gridlines();
					else
						Machine.show_major_gridlines();
				}
				else
				{
					if (Machine.minor_gridlines_visible())
						Machine.hide_minor_gridlines();
					else
						Machine.show_minor_gridlines();
				}
				break;
			case Key::ESCAPE:
				if (Machine.panning_info.panning)
				{
					k.consumed = true;
					Machine.canvas_cancel_panning();
				}
				break;
			}
		}
		};
	Machine.path_drop_handler.callback = [](const PathDropEvent& pd) {
		if (pd.num_paths >= 1 && Machine.cursor_in_easel())
		{
			FilePath filepath = pd.paths[0];
			static const size_t num_image_formats = 6;
			static const char* image_formats[num_image_formats] = { ".png", ".jpg", ".gif", ".bmp", ".tga", ".hdr" };
			static const size_t num_quasar_formats = 1;
			static const char* quasar_formats[num_quasar_formats] = { ".qua" };
			if (filepath.has_any_extension(image_formats, num_image_formats))
			{
				pd.consumed = true;
				Machine.main_window->focus();
				Machine.import_file(filepath);
			}
			else if (filepath.has_any_extension(quasar_formats, num_quasar_formats))
			{
				pd.consumed = true;
				Machine.main_window->focus();
				Machine.open_file(filepath);
			}
			// LATER note no error popup otherwise?
		}
		};
}
