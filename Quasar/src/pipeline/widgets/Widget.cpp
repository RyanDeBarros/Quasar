#include "Widget.h"

#include "user/Machine.h"

bool Widget::contains_global_point(Position pos) const
{
	auto bl = global_of({ -0.5f, -0.5f });
	auto tr = global_of({ 0.5f, 0.5f });
	return in_diagonal_rect(pos, bl, tr);
}

bool Widget::contains_screen_point(Position pos, glm::mat3* vp) const
{
	// TODO to_world_coordinates instead
	auto bl = Machine.to_screen_coordinates(global_of({ -0.5f, -0.5f }), *vp);
	auto tr = Machine.to_screen_coordinates(global_of({  0.5f,  0.5f }), *vp);
	return in_diagonal_rect(pos, bl, tr);
}
