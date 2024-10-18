#include "UserInput.h"

#include "pipeline/Renderer.h"
#include "Machine.h"

// TODO disable panning and zooming at same time
void attach_canvas_controls()
{
	// TODO ESCAPE to cancel panning
	// Panning
	Machine.main_window->clbk_mouse_button.push_back([](const Callback::MouseButton& mb) {
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == IAction::PRESS && Machine.canvas_renderer->cursor_in_clipping())
				Machine.canvas_begin_panning();
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_end_panning();
		}
		else if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == IAction::PRESS && Machine.canvas_renderer->cursor_in_clipping() && Machine.main_window->is_key_pressed(Key::SPACE))
				Machine.canvas_begin_panning();
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_end_panning();
		}
		});
	// Zooming
	Machine.main_window->clbk_scroll.push_back([](const Callback::Scroll& s) {
		if (Machine.canvas_renderer->clipping_rect().contains_point(Machine.main_window->cursor_pos()))
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
		if (Machine.main_window->is_ctrl_pressed() && k.action == IAction::PRESS)
		{
			switch (k.key)
			{
			case Key::Z:
				if (Machine.main_window->is_shift_pressed())
					Machine.redo();
				else
					Machine.undo();
				break;
			case Key::N:
				Machine.new_file();
				break;
			case Key::O:
				Machine.open_file();
				break;
			case Key::I:
				Machine.import_file();
				break;
			case Key::E:
				Machine.export_file();
				break;
			case Key::S:
				if (Machine.main_window->is_shift_pressed())
				{
					if (!Machine.main_window->is_alt_pressed())
						Machine.save_file_as();
				}
				else if (Machine.main_window->is_alt_pressed())
					Machine.save_file_copy();
				else
					Machine.save_file();
				break;
			}
		}
		});
	Machine.main_window->clbk_path_drop.push_back([](const Callback::PathDrop& pd) {
		// LATER if file extension is qua use open(), else if image format that's supported use import(), else do nothing.
		if (pd.num_paths >= 1 && Machine.canvas_renderer->cursor_in_clipping())
			Machine.import_file(pd.paths[0]);
		});
}
