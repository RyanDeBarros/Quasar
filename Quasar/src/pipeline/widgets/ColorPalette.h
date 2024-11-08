#pragma once

#include "edit/color/ColorScheme.h"
#include "Widget.h"

struct ColorSubpalette : public Widget
{

};

struct ColorPalette : public Widget
{
	ColorScheme scheme;
};
