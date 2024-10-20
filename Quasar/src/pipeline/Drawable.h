#pragma once

struct Drawable
{
	virtual void on_draw(class Renderer*) = 0;
};
