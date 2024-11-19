#pragma once

#include "Panel.h"

struct MenuPanel : public Panel
{
	virtual void _send_view() override;
	virtual void draw() override;
};
