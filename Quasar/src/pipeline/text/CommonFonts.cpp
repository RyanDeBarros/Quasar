#include "CommonFonts.h"

FontRange* Fonts::label_regular = nullptr;
FontRange* Fonts::label_bolditalic = nullptr;

// LATER only keep fonts being used
void Fonts::load_common_fonts()
{
	label_regular =       new FontRange(FileSystem::font_path("Roboto-Regular.ttf"), FileSystem::font_path("Roboto-Regular.kern"));
	label_bolditalic =    new FontRange(FileSystem::font_path("Roboto-BoldItalic.ttf"));

	label_regular         ->construct_fontsize(84);
	label_bolditalic      ->construct_fontsize(84);
}
