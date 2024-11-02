#include "IO.h"

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
	LOG << LOG.warning << LOG.start << "Could not open \"" << filepath.c_str() << "\" for reading" << LOG.endl;
	return false;
}

bool IO_impl::read_template_file(const FilePath& filepath, std::string& content, const std::unordered_map<std::string, std::string>& tmplate, std::ios_base::openmode mode)
{
	if (!read_file(filepath, content, mode))
		return false;
	for (const auto& [placeholder, value] : tmplate)
	{
		size_t pos = 0;
		while ((pos = content.find(placeholder, pos)) != std::string::npos)
		{
			content.replace(pos, placeholder.length(), value);
			pos += value.length();
		}
	}
	return true;
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
			LOG << LOG.error << LOG.start_toml << "Header \"" << head.value() << "\" does not match with expected header \"" << header << "\"." << LOG.endl;
			return false;
		}
		LOG << LOG.error << LOG.start_toml << "Header \"" << header << "\" is missing." << LOG.endl;
		return false;
	}
	catch (const toml::parse_error& err)
	{
		LOG << LOG.error << LOG.start << "Cannot parse toml file: \"" << filepath.c_str() << "\": " << err.description() << LOG.endl;
		return false;
	}
}

void IO_impl::load_quasar_settings()
{
	toml::v3::parse_result _TOML;
	if (!parse_toml("./Quasar.toml", "settings", _TOML))
	{
		LOG << LOG.fatal << LOG.start << "Quasar settings could not be loaded. Press any key to quit..." << LOG.endl;
		std::cin.get();
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
		LOG << LOG.fatal << LOG.start << "Could not load preferences file \"" << filepath.c_str() << LOG.endl;
		QUASAR_ASSERT(false);
	}
	auto _wspc = _TOML["workspace"].value<std::string>();
	if (!_wspc)
	{
		LOG << LOG.error << LOG.start << "Workspace \"" << workspace << "\" not present in preferences file." << LOG.endl;
		QUASAR_ASSERT(false);
	}
	if (_wspc.value() != workspace)
	{
		LOG << LOG.error << LOG.start << "Workspace \"" << _wspc.value() << "\" does not match up with expected workspace \"" << workspace << "\"" << LOG.endl;
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
					auto c1 = _Checkerboard_checker1->get_as<double>(0);
					auto c2 = _Checkerboard_checker1->get_as<double>(1);
					auto c3 = _Checkerboard_checker1->get_as<double>(2);
					auto c4 = _Checkerboard_checker1->get_as<double>(3);
					if (c1 && c2 && c3 && c4)
						preferences.checker1 = RGBA((float)c1->get(), (float)c2->get(), (float)c3->get(), (float)c4->get());
				}
				if (auto _Checkerboard_checker2 = _Checkerboard["checker2"].as_array())
				{
					auto c1 = _Checkerboard_checker2->get_as<double>(0);
					auto c2 = _Checkerboard_checker2->get_as<double>(1);
					auto c3 = _Checkerboard_checker2->get_as<double>(2);
					auto c4 = _Checkerboard_checker2->get_as<double>(3);
					if (c1 && c2 && c3 && c4)
						preferences.checker2 = RGBA((float)c1->get(), (float)c2->get(), (float)c3->get(), (float)c4->get());
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
