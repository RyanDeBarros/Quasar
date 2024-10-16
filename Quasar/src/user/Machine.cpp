#include "Machine.h"

void MachineImpl::destroy()
{
	images.clear();
	shaders.clear();
	delete canvas_renderer;
	canvas_renderer = nullptr;
	delete main_window;
	main_window = nullptr;
}

void MachineImpl::flip_horizontally()
{
	static Action a([this]() { canvas_image->flip_horizontally(); }, [this]() { canvas_image->flip_horizontally(); });
	history.execute(a);
}

void MachineImpl::flip_vertically()
{
	static Action a([this]() { canvas_image->flip_vertically(); }, [this]() { canvas_image->flip_vertically(); });
	history.execute(a);
}

void MachineImpl::rotate_180()
{
	static Action a([this]() { canvas_image->rotate_180(); }, [this]() { canvas_image->rotate_180(); });
	history.execute(a);
}
