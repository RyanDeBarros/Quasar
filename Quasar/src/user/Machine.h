#pragma once

#include "variety/Registry.h"
#include "edit/Image.h"
#include "pipeline/Shader.h"
#include "variety/History.h"
#include "pipeline/Renderer.h"

struct MachineImpl
{
	typedef Registry<Image, unsigned short, ImageConstructor> ImageRegistry;
	typedef ImageRegistry::Handle ImageHandle;
	typedef Registry<Shader, unsigned short, ShaderConstructor> ShaderRegistry;
	typedef ShaderRegistry::Handle ShaderHandle;
	
	ImageRegistry images;
	ShaderRegistry shaders;
	ActionHistory history;

	Window* main_window = nullptr;
	Renderer* canvas_renderer = nullptr;
	Image* canvas_image = nullptr;

	void destroy();

	void undo() { history.undo(); }
	bool undo_enabled() const { return history.undo_size() != 0; }
	void redo() { history.redo(); }
	bool redo_enabled() const { return history.redo_size() != 0; }
	
	void flip_horizontally();
	void flip_vertically();
	void rotate_180();
};

inline MachineImpl Machine;
