#pragma once

#include "Panel.h"
#include "user/Platform.h"

struct MenuPanel : public Panel
{
	KeyHandler key_handler;

private:
	bool escape_to_close_menu = false;
	bool _close_menu = false;
	bool submenus_open = false;

public:
	MenuPanel();
	MenuPanel(const MenuPanel&) = delete;
	MenuPanel(MenuPanel&&) noexcept = delete;

	virtual void _send_view() override;
	virtual void draw() override;

private:
	void main_menu_setup();
};
