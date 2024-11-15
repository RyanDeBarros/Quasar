#pragma once

#include "Font.h"

namespace Fonts
{
	extern FontRange* label_regular;
	extern FontRange* label_black;

	extern void load_common_fonts();
	extern void invalidate_common_fonts();
}
