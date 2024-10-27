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

glm::vec2 Panel::get_app_size() const
{
	return glm::vec2{ bounds.clip().screen_w, bounds.clip().screen_h } * pgroup->app_scale;
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

void Panel::send_view()
{
	view.position = to_view_coordinates(bounds.clip().center_point());
	_send_view();
}

glm::vec2 Panel::to_view_coordinates(const glm::vec2& screen_coordinates) const
{
	glm::vec3 ndc{};
	ndc.x = 1.0f - 2.0f * (screen_coordinates.x / Machine.main_window->width());
	ndc.y = 1.0f - 2.0f * (screen_coordinates.y / Machine.main_window->height());
	ndc.z = 1.0f;

	glm::mat3 invVP = glm::inverse(pgroup->projection);
	glm::vec3 view_pos = invVP * ndc;

	if (view_pos.z != 0.0f)
		view_pos / view_pos.z;

	return glm::vec2{ view_pos.x, view_pos.y };

}

glm::vec2 Panel::to_world_coordinates(const glm::vec2& screen_coordinates) const
{
	glm::vec3 ndc{};
	ndc.x = 1.0f - 2.0f * (screen_coordinates.x / Machine.main_window->width());
	ndc.y = 1.0f - 2.0f * (screen_coordinates.y / Machine.main_window->height());
	ndc.z = 1.0f;

	glm::mat3 invVP = glm::inverse(vp_matrix());
	glm::vec3 world_pos = invVP * ndc;

	if (world_pos.z != 0.0f)
		world_pos / world_pos.z;

	return -glm::vec2{ world_pos.x, world_pos.y };
}

glm::vec2 Panel::to_screen_coordinates(const glm::vec2& world_coordinates) const
{
	glm::vec3 world_pos{ world_coordinates.x, -world_coordinates.y, 1.0f };
	glm::vec3 clip_space_pos = vp_matrix() * world_pos;
	glm::vec2 screen_coo{};
	screen_coo.x = (1.0f + clip_space_pos.x) * 0.5f * Machine.main_window->width();
	screen_coo.y = (1.0f + clip_space_pos.y) * 0.5f * Machine.main_window->height();
	return screen_coo;
}

const Scale& Panel::app_scale() const
{
	return pgroup->app_scale;
}

Scale& Panel::app_scale()
{
	return pgroup->app_scale;
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