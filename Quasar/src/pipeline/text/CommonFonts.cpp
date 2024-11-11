#include "CommonFonts.h"

FontRange* Fonts::label_regular = nullptr;
FontRange* Fonts::label_black = nullptr;

// LATER only keep fonts being used
void Fonts::load_common_fonts()
{
	label_regular =       new FontRange(FileSystem::font_path("Merriweather-Regular.ttf")); // LATER put font filepaths in settings file, so they can be configured?
	label_black =         new FontRange(FileSystem::font_path("Merriweather-Black.ttf"));

	label_regular         ->construct_fontsize(84);
	label_black           ->construct_fontsize(84);
}

void Fonts::invalidate_common_fonts()
{
	delete label_regular;
	label_regular = nullptr;
	delete label_black;
	label_black = nullptr;
}
