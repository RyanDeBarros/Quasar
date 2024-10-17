#include "UserInput.h"

#include "pipeline/Renderer.h"
#include "Machine.h"

void attach_canvas_controls()
{
	// Panning
	Machine.main_window->clbk_mouse_button.push_back([](const Callback::MouseButton& mb) {
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == IAction::PRESS && Machine.canvas_renderer->cursor_in_clipping())
				Machine.canvas_renderer->begin_panning();
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_renderer->end_panning();
		}
		if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == IAction::PRESS && Machine.main_window->is_key_pressed(Key::SPACE) && Machine.canvas_renderer->cursor_in_clipping())
				Machine.canvas_renderer->begin_panning();
			else if (mb.action == IAction::RELEASE)
				Machine.canvas_renderer->end_panning();
		}
		});
	// Zooming
	Machine.main_window->clbk_scroll.push_back([](const Callback::Scroll& s) {
		if (!Machine.canvas_renderer->clipping_rect().contains_point(Machine.main_window->cursor_pos()))
			return;
		Machine.canvas_renderer->zoom_by(s.yoff);
		});
	// Reset camera
	Machine.main_window->clbk_key.push_back([](const Callback::Key& k) {
		if (k.key == Key::ROW0 && k.action == IAction::PRESS)
			Machine.canvas_renderer->reset_camera();
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
}
