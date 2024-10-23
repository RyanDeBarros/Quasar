#pragma once

#include <glm/glm.hpp>

#include "edit/Color.h"

struct WorkspacePreferences
{
	RGBA checker1 = RGBA(HSV(0.5f, 0.2f, 0.2f).to_rgb(), 0.5f);
	RGBA checker2 = RGBA(HSV(0.5f, 0.3f, 0.3f).to_rgb(), 0.5f);
	glm::ivec2 checker_size = { 16, 16 };
	float min_initial_image_window_proportion = 3.0f;
};
