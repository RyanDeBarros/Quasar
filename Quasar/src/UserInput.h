#pragma once

#include "Renderer.h"

class UserInputManager
{
	Renderer* renderer;
	
	glm::vec2 pan_initial_delta{};
	bool panning = false;

public:
	UserInputManager(Renderer* renderer);

	void update() const;

private:
	void begin_panning();
	void end_panning();
};
