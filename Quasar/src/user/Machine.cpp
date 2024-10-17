#include "Machine.h"

#include <tinyfd/tinyfiledialogs.h>

#include "pipeline/Sprite.h"
#include "UserInput.h"

void MachineImpl::init_renderer()
{
	canvas_renderer = new Renderer(main_window, Shader());
	canvas_sprite = new Sprite(ImageHandle(0));
	canvas_renderer->sprites().push_back(canvas_sprite);

	attach_canvas_controls();
	attach_global_user_controls();
}

void MachineImpl::destroy()
{
	images.clear();
	shaders.clear();
	delete canvas_renderer;
	canvas_renderer = nullptr;
	delete main_window;
	main_window = nullptr;
	delete canvas_sprite;
	canvas_sprite = nullptr;
}

void MachineImpl::mark()
{
	unsaved = true;
	// TODO edit title to include (*)
}

void MachineImpl::unmark()
{
	unsaved = false;
	// TODO remove (*) from title if it exists
}

bool MachineImpl::new_file()
{
	if (unsaved)
	{
		int response = tinyfd_messageBox("Notice", "Do you want to save your changes first?", "yesnocancel", "question", 1);
		if (response == 0) return false;
		if (response == 1) if (!save_file()) return false;
	}
	current_filepath.clear();
	for (auto iter = canvas_renderer->sprites().begin(); iter != canvas_renderer->sprites().end(); ++iter)
	{
		if (*iter == canvas_sprite)
		{
			canvas_renderer->sprites().erase(iter);
			break;
		}
	}
	mark();
	// TODO clear palletes/frames/layers/etc.
	return true;
}

bool MachineImpl::open_file()
{
	if (unsaved)
	{
		// TODO ask if user wants to save
	}
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	const char* openfile = tinyfd_openFileDialog("Open file", "", num_filters, filters, "", false);
	if (!openfile) return false;
	current_filepath = openfile;
	mark();
	open_file(openfile);
}

bool MachineImpl::import_file()
{
	static const int num_filters = 4;
	static const char* const filters[num_filters] = { "*.png", "*.gif", "*.jpg", "*.bmp" };
	const char* importfile = tinyfd_openFileDialog("Import file", "", num_filters, filters, "", false);
	if (!importfile) return false;
	mark();
	import_file(importfile);
	return true;
}

bool MachineImpl::export_file() const
{
	static const int num_filters = 4;
	static const char* const filters[num_filters] = { "*.png", "*.gif", "*.jpg", "*.bmp" };
	// TODO open custom dialog for export settings first
	const char* exportfile = tinyfd_saveFileDialog("Export file", "", num_filters, filters, "");
	if (!exportfile) return false;
	// TODO actually export file
	return true;
}

bool MachineImpl::save_file()
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	if (current_filepath.empty())
	{
		const char* savefile = tinyfd_saveFileDialog("Save file", "", num_filters, filters, "");
		if (!savefile) return false;
		// TODO create new file
		current_filepath = savefile;
	}
	save_file(current_filepath.c_str());
	unmark();
	return true;
}

bool MachineImpl::save_file_as()
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	const char* savefile = tinyfd_saveFileDialog("Save file as", "", num_filters, filters, "");
	if (!savefile) return false;
	// TODO create new file
	current_filepath = savefile;
	save_file(savefile);
	unmark();
	return true;
}

bool MachineImpl::save_file_copy()
{
	static const int num_filters = 1;
	static const char* const filters[num_filters] = { "*.qua" };
	const char* savefile = tinyfd_saveFileDialog("Save file copy", "", num_filters, filters, "");
	if (!savefile) return false;
	// TODO create new file
	save_file(savefile);
	unmark();
	return true;
}

void MachineImpl::open_file(const char* filepath)
{
	// TODO actually open quasar file
}

void MachineImpl::import_file(const char* filepath)
{
	// TODO register instead to ensure unique? or use secondary registry specifically for canvas_image
	auto img = images.construct(ImageConstructor(filepath));
	canvas_image = images.get(img);
	canvas_sprite->set_image(img);
}

void MachineImpl::save_file(const char* filepath)
{
	// TODO actually save changes to filepath
}

void MachineImpl::flip_horizontally()
{
	static Action a([this]() { canvas_image->flip_horizontally(); mark(); }, [this]() { canvas_image->flip_horizontally(); mark(); });
	history.execute(a);
}

void MachineImpl::flip_vertically()
{
	static Action a([this]() { canvas_image->flip_vertically(); mark(); }, [this]() { canvas_image->flip_vertically(); mark(); });
	history.execute(a);
}

void MachineImpl::rotate_180()
{
	static Action a([this]() { canvas_image->rotate_180(); mark(); }, [this]() { canvas_image->rotate_180(); mark(); });
	history.execute(a);
}
