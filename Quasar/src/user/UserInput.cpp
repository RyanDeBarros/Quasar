#include "UserInput.h"

#include "Machine.h"

// TODO disable panning and zooming at same time
void attach_canvas_controls()
{
	// Panning
	Machine.main_window->clbk_mouse_button.push_back([](const Callback::MouseButton& mb) {
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == IAction::PRESS && Machine.cursor_in_easel())
				Machine.canvas_begin_panning();
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_end_panning();
		}
		else if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == IAction::PRESS && Machine.cursor_in_easel() && Machine.main_window->is_key_pressed(Key::SPACE))
				Machine.canvas_begin_panning();
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_end_panning();
		}
		});
	// Zooming
	Machine.main_window->clbk_scroll.push_back([](const Callback::Scroll& s) {
		if (Machine.cursor_in_easel())
			Machine.canvas_zoom_by(s.yoff);
		});
	// Reset camera
	Machine.main_window->clbk_key.push_back([](const Callback::Key& k) {
		if (k.key == Key::ROW0 && k.action == IAction::PRESS)
			Machine.canvas_reset_camera();
		});
}

void attach_global_user_controls()
{
	Machine.main_window->clbk_key.push_back([](const Callback::Key& k) {
		if (k.action == IAction::PRESS)
		{
			switch (k.key)
			{
			case Key::Z:
				if (Machine.main_window->is_ctrl_pressed())
				{
					if (Machine.main_window->is_shift_pressed())
						Machine.redo();
					else
						Machine.undo();
				}
				break;
			case Key::N:
				if (Machine.main_window->is_ctrl_pressed())
					Machine.new_file();
				break;
			case Key::O:
				if (Machine.main_window->is_ctrl_pressed())
					Machine.open_file();
				break;
			case Key::I:
				if (Machine.main_window->is_ctrl_pressed())
					Machine.import_file();
				break;
			case Key::E:
				if (Machine.main_window->is_ctrl_pressed())
					Machine.export_file();
				break;
			case Key::S:
				if (Machine.main_window->is_ctrl_pressed())
				{
					if (Machine.main_window->is_shift_pressed())
					{
						if (!Machine.main_window->is_alt_pressed())
							Machine.save_file_as();
					}
					else if (Machine.main_window->is_alt_pressed())
						Machine.save_file_copy();
					else
						Machine.save_file();
				}
				break;
			case Key::G:
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
				Machine.canvas_cancel_panning();
			}
		}
		});
	Machine.main_window->clbk_path_drop.push_back([](const Callback::PathDrop& pd) {
		// LATER if file extension is qua use open(), else if image format that's supported use import(), else do nothing.
		if (pd.num_paths >= 1 && Machine.cursor_in_easel())
			Machine.import_file(pd.paths[0]);
		Machine.main_window->focus();
		});
}
