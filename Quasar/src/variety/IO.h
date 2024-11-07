#pragma once

#include <unordered_map>
#include <string>

#include <toml/toml.hpp>

#include "variety/FileSystem.h"

struct IO_impl
{
	IO_impl() = default;
	IO_impl(const IO_impl&) = delete;
	IO_impl(IO_impl&&) = delete;
	~IO_impl() = default;

	bool read_file(const FilePath& filepath, std::string& content);
	bool read_file_uc(const FilePath& filepath, unsigned char*& content, size_t& content_length);
	bool read_template_file(const FilePath& filepath, std::string& content, const std::unordered_map<std::string, std::string>& tmplate);
	bool parse_toml(const FilePath& filepath, const char* header, toml::v3::parse_result& parse_result);
	void load_quasar_settings();
	void load_workspace_preferences(const FilePath& filepath, const char* workspace);
};

inline IO_impl IO;
