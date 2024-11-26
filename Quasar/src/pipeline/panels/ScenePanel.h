#pragma once

#include "Panel.h"

struct ScenePanel : public Panel
{
	virtual void draw() override;
	virtual void _send_view() override;
	virtual Scale minimum_screen_display() const override;
};
