#include "IO.h"

#include <sstream>
#include <iostream>

#include "Macros.h"
#include "user/Machine.h"

bool IO_impl::read_file(const FilePath& filepath, std::string& content, std::ios_base::openmode mode)
{
	std::ifstream file(filepath.c_str(), mode);
	if (file)
	{
		std::ostringstream oss;
		oss << file.rdbuf();
		content = oss.str();
		return true;
	}
	return false;
}

bool IO_impl::parse_toml(const FilePath& filepath, const char* header, toml::v3::parse_result& parse_result)
{
	try
	{
		parse_result = toml::parse_file(filepath.c_str());
		if (auto head = parse_result["header"].value<std::string>())
		{
			if (head.value() == header)
				return true;
			// LATER use logging system. Don't use log functions, but rather log streams similar to std::cout, std::cerr, and std::endl.
			// Of course, this will probably get printed to a log file, in addition to console.
			std::cerr << "[Error TOML]: Header \"" << head.value() << "\" does not match with expected header \"" << header << "\"." << std::endl;
			return false;
		}
		std::cerr << "[Error TOML]: Header \"" << header << "\" is missing." << std::endl;
		return false;
	}
	catch (const toml::parse_error& err)
	{
		std::cerr << "Cannot parse toml file: \"" << filepath.c_str() << "\": " << err.description() << std::endl;
		return false;
	}
}

void IO_impl::load_quasar_settings()
{
	toml::v3::parse_result _TOML;
	if (!parse_toml("./Quasar.toml", "settings", _TOML))
	{
		QUASAR_ASSERT(false);
	}

	auto _FileSystem = _TOML["FileSystem"];
	QUASAR_ASSERT(_FileSystem);
	auto _FileSystem_resources_root = _FileSystem["resources_root"];
	FileSystem::resources_root = _FileSystem_resources_root.value_or<std::string>("./res").c_str();

	auto _Renderer = _TOML["Renderer"];
	QUASAR_ASSERT(_Renderer)
	Machine.vsync = (int)_Renderer["vsync"].value_or<int64_t>(0);
	Machine.raw_mouse_motion = _Renderer["raw_mouse_motion"].value_or<bool>(true);

	load_workspace_preferences(FileSystem::resources_path("global_workspace.toml"), "global");
}

void IO_impl::load_workspace_preferences(const FilePath& filepath, const char* workspace)
{
	WorkspacePreferences& preferences = Machine.preferences;
	preferences = {};
	toml::v3::parse_result _TOML;
	if (!parse_toml(filepath, "preferences", _TOML))
	{
		QUASAR_ASSERT(false);
	}
	auto _wspc = _TOML["workspace"].value<std::string>();
	if (!_wspc)
	{
		std::cerr << "Workspace \"" << workspace << "\" not present in preferences file." << std::endl;
		QUASAR_ASSERT(false);
	}
	if (_wspc.value() != workspace)
	{
		std::cerr << "Workspace \"" << _wspc.value() << "\" does not match up with expected workspace \"" << workspace << "\"" << std::endl;
		QUASAR_ASSERT(false);
	}

	auto _FileSystem = _TOML["FileSystem"];
	QUASAR_ASSERT(_FileSystem);
	auto _FileSystem_workspace_root = _FileSystem["workspace_root"];
	FileSystem::workspace_root = _FileSystem_workspace_root.value_or<std::string>(".").c_str();

	if (auto _Easel = _TOML["Easel"])
	{
		if (auto _Canvas = _Easel["Canvas"])
		{
			if (auto _Checkerboard = _Canvas["Checkerboard"])
			{
				if (auto _Checkerboard_checker1 = _Checkerboard["checker1"].as_array())
				{
					auto c1 = _Checkerboard_checker1->get_as<int64_t>(0);
					auto c2 = _Checkerboard_checker1->get_as<int64_t>(1);
					auto c3 = _Checkerboard_checker1->get_as<int64_t>(2);
					auto c4 = _Checkerboard_checker1->get_as<int64_t>(3);
					if (c1 && c2 && c3 && c4)
						preferences.checker1 = RGBA((unsigned char)c1->get(), (unsigned char)c2->get(), (unsigned char)c3->get(), (unsigned char)c4->get());
				}
				if (auto _Checkerboard_checker2 = _Checkerboard["checker2"].as_array())
				{
					auto c1 = _Checkerboard_checker2->get_as<int64_t>(0);
					auto c2 = _Checkerboard_checker2->get_as<int64_t>(1);
					auto c3 = _Checkerboard_checker2->get_as<int64_t>(2);
					auto c4 = _Checkerboard_checker2->get_as<int64_t>(3);
					if (c1 && c2 && c3 && c4)
						preferences.checker2 = RGBA((unsigned char)c1->get(), (unsigned char)c2->get(), (unsigned char)c3->get(), (unsigned char)c4->get());
				}
				if (auto _Checkerboard_checker_size = _Checkerboard["checker_size"].as_array())
				{
					auto x = _Checkerboard_checker_size->get_as<int64_t>(0);
					auto y = _Checkerboard_checker_size->get_as<int64_t>(1);
					if (x && y)
						preferences.checker_size = { x->get(), y->get() };
				}
			}
		}
		if (auto _Gridlines = _Easel["Gridlines"])
		{
			if (auto _Gridlines_min_initial_image_window_proportion = _Gridlines["min_initial_image_window_proportion"].value<double>())
				preferences.min_initial_image_window_proportion = (float)_Gridlines_min_initial_image_window_proportion.value();
		}
	}
}
