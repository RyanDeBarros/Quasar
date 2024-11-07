#include "Panel.h"

#include "user/Machine.h"

void Panel::render()
{
	if (visible)
	{
		//bounds.clip().scissor(); // TODO uncomment
		draw();
	}
}

bool Panel::cursor_in_clipping() const
{
	return bounds.clip().contains_point(Machine.main_window->cursor_pos());
}

glm::vec2 Panel::get_app_size() const
{
	return glm::vec2{ bounds.clip().screen_w, bounds.clip().screen_h } * Machine.inv_app_scale();
}

float Panel::get_app_width() const
{
	return bounds.clip().screen_w * Machine.inv_app_scale().x;
}

float Panel::get_app_height() const
{
	return bounds.clip().screen_h * Machine.inv_app_scale().y;
}

glm::vec2 Panel::get_app_cursor_pos() const
{
	return Machine.main_window->cursor_pos() * Machine.inv_app_scale();
}

glm::mat3 Panel::vp_matrix() const
{
	return pgroup->projection * view.camera();
}

glm::mat3 Panel::vp_matrix_inverse() const
{
	return view.matrix() * glm::inverse(pgroup->projection);
}

void Panel::send_view()
{
	view.position = to_view_coordinates(bounds.clip().center_point());
	_send_view();
}

Position Panel::to_view_coordinates(Position screen_coordinates) const
{
	return Machine.to_world_coordinates(screen_coordinates, glm::inverse(pgroup->projection));
}

Position Panel::to_world_coordinates(Position screen_coordinates) const
{
	return Machine.to_world_coordinates(screen_coordinates, vp_matrix_inverse());
}

Position Panel::to_screen_coordinates(Position world_coordinates) const
{
	return Machine.to_screen_coordinates(world_coordinates, vp_matrix());
}

void PanelGroup::sync_panels()
{
	for (auto& panel : panels)
		panel->pgroup = this;
}

void PanelGroup::render()
{
	for (auto& panel : panels)
		panel->render();
}

void PanelGroup::set_projection()
{
	projection = glm::ortho<float>(0.0f, Machine.main_window->width() * Machine.inv_app_scale().x, 0.0f, Machine.main_window->height() * Machine.inv_app_scale().y);
	for (auto& panel : panels)
		panel->send_view();
}
