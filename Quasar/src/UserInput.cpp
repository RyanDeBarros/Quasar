#include "UserInput.h"

UserInputManager::UserInputManager(Renderer* renderer)
	: renderer(renderer)
{
	renderer->get_window()->clbk_mouse_button.push_back([this](const Callback::MouseButton& mb) {
		if (mb.button == MouseButton::MIDDLE)
		{
			if (mb.action == Action::PRESS)
				begin_panning();
			else if (mb.action == Action::RELEASE)
				end_panning();
		}
		if (mb.button == MouseButton::LEFT)
		{
			if (mb.action == Action::PRESS)
			{
				if (this->renderer->get_window()->is_key_pressed(Key::SPACE))
					begin_panning();
			}
			else if (mb.action == Action::RELEASE)
				end_panning();
		}
		});
	renderer->get_window()->clbk_key.push_back([this](const Callback::Key& k) {
		if (k.key == Key::SPACE && k.action == Action::RELEASE)
			end_panning();
		});
}

void UserInputManager::update() const
{
	if (panning)
	{
		renderer->set_view(Transform{ glm::vec2{-1.0f, 1.0f} * (renderer->get_window()->cursor_pos() * renderer->get_app_scale()) + pan_initial_delta });
	}
}

void UserInputManager::begin_panning()
{
	pan_initial_delta = this->renderer->get_view().position - glm::vec2{ -1.0f, 1.0f } *this->renderer->get_window()->cursor_pos() * this->renderer->get_app_scale();
	panning = true;
}

void UserInputManager::end_panning()
{
	panning = false;
}
