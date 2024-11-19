#pragma once

#include "Panel.h"

struct ScenePanel : public Panel
{
	void draw() override;
	void _send_view() override;
};
