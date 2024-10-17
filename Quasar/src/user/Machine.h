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
	struct Sprite* canvas_sprite = nullptr;

	std::string current_filepath = "";
	bool unsaved = true;

	std::vector<std::string> recent_files;
	std::vector<std::string> recent_image_files;

	void init_renderer();
	void destroy();
	void exit() const { main_window->request_close(); }
	void mark();
	void unmark();

	bool new_file();
	bool open_file();
	bool import_file();
	bool export_file() const;
	bool save_file();
	bool save_file_as();
	bool save_file_copy();

	void open_file(const char* filepath);
	void import_file(const char* filepath);
	void save_file(const char* filepath);

	void undo() { history.undo(); }
	bool undo_enabled() const { return history.undo_size() != 0; }
	void redo() { history.redo(); }
	bool redo_enabled() const { return history.redo_size() != 0; }
	
	void flip_horizontally();
	void flip_vertically();
	void rotate_180();
};

inline MachineImpl Machine;
