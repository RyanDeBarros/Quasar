#pragma once

#include <toml/toml.hpp>

#include "variety/FileSystem.h"

struct IO_impl
{
	IO_impl() = default;
	IO_impl(const IO_impl&) = delete;
	IO_impl(IO_impl&&) = delete;
	~IO_impl() = default;

	bool read_file(const FilePath& filepath, std::string& content, std::ios_base::openmode mode = std::ios_base::in);
	bool parse_toml(const FilePath& filepath, const char* header, toml::v3::parse_result& parse_result);
	void load_quasar_settings();
	void load_workspace_preferences(const FilePath& filepath, const char* workspace);
};

inline IO_impl IO;
