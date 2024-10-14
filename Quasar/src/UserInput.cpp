#include "UserInput.h"

#include "Renderer.h"

void attach_canvas_controls(Renderer* renderer)
{
	// Panning
	renderer->get_window()->clbk_mouse_button.push_back([renderer](const Callback::MouseButton& mb) {
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == Action::PRESS)
				renderer->begin_panning();
			else if (mb.action == Action::RELEASE)
				renderer->end_panning();
		}
		if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == Action::PRESS)
			{
				if (renderer->get_window()->is_key_pressed(Key::SPACE))
					renderer->begin_panning();
			}
			else if (mb.action == Action::RELEASE)
				renderer->end_panning();
		}
		});
	renderer->get_window()->clbk_key.push_back([renderer](const Callback::Key& k) {
		if (k.key == Key::SPACE && k.action == Action::RELEASE)
			renderer->end_panning();
		});
	// Zooming
	renderer->get_window()->clbk_scroll.push_back([renderer](const Callback::Scroll& s) {
		renderer->zoom_by(s.yoff);
		});
	// Reset camera
	renderer->get_window()->clbk_key.push_back([renderer](const Callback::Key& k) {
		if (k.key == Key::ROW0 && k.action == Action::PRESS)
			renderer->reset_camera();
		});
}
