#pragma once

#include "Platform.h"

extern void handle_global_key_event(const KeyEvent& k);

enum class ControlScheme
{
	FILE,
	PALETTE
};
