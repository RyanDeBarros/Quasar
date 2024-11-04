#include "CommonFonts.h"

Font* Fonts::roboto_regular = nullptr;
Font* Fonts::roboto_bolditalic = nullptr;

void Fonts::load_common_fonts()
{
	roboto_regular =		new Font(FileSystem::font_path("Roboto-Regular.ttf"),		96, COMMON, TextureParams::linear, FileSystem::font_path("Roboto-Regular.kern")); // TODO FontRange with different font sizes
	roboto_bolditalic =		new Font(FileSystem::font_path("Roboto-BoldItalic.ttf"),	32, COMMON, TextureParams::linear, "");
}

void Fonts::invalidate_common_fonts()
{
	delete roboto_regular;
	roboto_regular = nullptr;
	delete roboto_bolditalic;
	roboto_bolditalic = nullptr;
}
