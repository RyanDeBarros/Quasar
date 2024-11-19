#pragma once

#include "Panel.h"

struct BrushesPanel : public Panel
{
	void draw() override;
	void _send_view() override;
};
