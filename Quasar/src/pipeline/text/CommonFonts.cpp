#include "CommonFonts.h"

FontRange* Fonts::roboto_regular = nullptr;
FontRange* Fonts::roboto_bolditalic = nullptr;

static const int standard_sizes[] = { 8, 10, 12, 14, 16, 18, 20, 24, 30, 36, 48, 64 };

void Fonts::load_common_fonts()
{
	roboto_regular =		new FontRange(FileSystem::font_path("Roboto-Regular.ttf"), FileSystem::font_path("Roboto-Regular.kern"));
	roboto_bolditalic =		new FontRange(FileSystem::font_path("Roboto-BoldItalic.ttf"));

	for (int size : standard_sizes)
	{
		TextureParams params = size < 17 ? TextureParams{} : TextureParams::linear;
		roboto_regular		->construct_fontsize(float(size), Fonts::COMMON, params);
		roboto_bolditalic	->construct_fontsize(float(size), Fonts::COMMON, params);
	}
}
