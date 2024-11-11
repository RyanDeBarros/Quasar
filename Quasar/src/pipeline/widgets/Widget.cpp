#include "Widget.h"

#include "user/Machine.h"

bool Widget::contains_global_point(Position pos) const
{
	auto bl = global_of({ -0.5f, -0.5f });
	auto tr = global_of({ 0.5f, 0.5f });
	return in_diagonal_rect(pos, bl, tr);
}

bool Widget::contains_screen_point(Position pos, const glm::mat3& vp) const
{
	auto bl = global_of({ -0.5f, -0.5f });
	auto tr = global_of({  0.5f,  0.5f });
	return in_diagonal_rect(Machine.cursor_world_pos(glm::inverse(vp)), bl, tr);
}

Logger& operator<<(Logger& log, const WidgetPlacement& wp)
{
	log << "[T: " << wp.transform.position << " | S: " << wp.transform.scale << " | P: " << wp.pivot;
	return log;
}
