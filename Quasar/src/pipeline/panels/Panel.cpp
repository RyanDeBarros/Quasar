#include "Panel.h"

#include "user/Machine.h"

void Panel::render()
{
	if (visible)
	{
		bounds.clip().scissor();
		draw();
	}
}

bool Panel::cursor_in_clipping() const
{
	return bounds.clip().contains_point(Machine.main_window->cursor_pos());
}

float Panel::get_app_width() const
{
	return bounds.clip().screen_w * pgroup->app_scale.x;
}

float Panel::get_app_height() const
{
	return bounds.clip().screen_h * pgroup->app_scale.y;
}

glm::vec2 Panel::get_app_cursor_pos() const
{
	return Machine.main_window->cursor_pos() * pgroup->app_scale;
}

glm::mat3 Panel::vp_matrix() const
{
	return pgroup->projection * view.camera();
}

void PanelGroup::sync_panels()
{
	for (auto& panel : panels)
		panel->pgroup = this;
	set_projection();
}

void PanelGroup::render()
{
	for (auto& panel : panels)
		panel->render();
}

void PanelGroup::set_projection()
{
	projection = glm::ortho<float>(0.0f, Machine.main_window->width() * app_scale.x, 0.0f, Machine.main_window->height() * app_scale.y);
	for (auto& panel : panels)
		panel->send_view();
}

void PanelGroup::set_app_scale(Scale sc)
{
	app_scale = 1.0f / sc;
	set_projection();
}

Scale PanelGroup::get_app_scale() const
{
	return 1.0f / app_scale;
}