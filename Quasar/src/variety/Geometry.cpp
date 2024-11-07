#include "Geometry.h"

// TODO put in Machine.cpp instead
Position to_view_coordinates(Position screen_coordinates)
{

	glm::vec3 ndc{};
	ndc.x = 1.0f - 2.0f * (screen_coordinates.x / Machine.main_window->width());
	ndc.y = 1.0f - 2.0f * (screen_coordinates.y / Machine.main_window->height());
	ndc.z = 1.0f;

	glm::mat3 invVP = glm::inverse(pgroup->projection);
	glm::vec3 view_pos = invVP * ndc;

	if (view_pos.z != 0.0f)
		view_pos / view_pos.z;

	return { view_pos.x, view_pos.y };
}
