#include "UserInput.h"

#include "pipeline/Renderer.h"

void attach_canvas_controls(Renderer* renderer)
{
	// Panning
	renderer->get_window()->clbk_mouse_button.push_back([renderer](const Callback::MouseButton& mb) {
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == IAction::PRESS && renderer->cursor_in_clipping())
				renderer->begin_panning();
			else if (mb.action == IAction::RELEASE)
				renderer->end_panning();
		}
		if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == IAction::PRESS && renderer->get_window()->is_key_pressed(Key::SPACE) && renderer->cursor_in_clipping())
				renderer->begin_panning();
			else if (mb.action == IAction::RELEASE)
				renderer->end_panning();
		}
		});
	// Zooming
	renderer->get_window()->clbk_scroll.push_back([renderer](const Callback::Scroll& s) {
		if (!renderer->clipping_rect().contains_point(renderer->get_window()->cursor_pos()))
			return;
		renderer->zoom_by(s.yoff);
		});
	// Reset camera
	renderer->get_window()->clbk_key.push_back([renderer](const Callback::Key& k) {
		if (k.key == Key::ROW0 && k.action == IAction::PRESS)
			renderer->reset_camera();
		});
}
