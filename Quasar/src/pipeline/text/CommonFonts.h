#pragma once

#include "Font.h"

namespace Fonts
{
	extern Font* roboto_regular;
	extern Font* roboto_bolditalic;

	extern void load_common_fonts();
	extern void invalidate_common_fonts();
}
