#include "CommonFonts.h"

FontRange* Fonts::roboto_regular = nullptr;
FontRange* Fonts::roboto_bolditalic = nullptr;

void Fonts::load_common_fonts()
{
	roboto_regular =		new FontRange(FileSystem::font_path("Roboto-Regular.ttf"), FileSystem::font_path("Roboto-Regular.kern"));
	roboto_bolditalic =		new FontRange(FileSystem::font_path("Roboto-BoldItalic.ttf"));

	roboto_regular->construct_fontsize(8);
	roboto_regular->construct_fontsize(32);
	roboto_regular->construct_fontsize(96);
	roboto_bolditalic->construct_fontsize(8);
	roboto_bolditalic->construct_fontsize(32);
	roboto_bolditalic->construct_fontsize(96);
}
