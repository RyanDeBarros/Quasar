#pragma once

#include <filesystem>

class FilePath
{
	std::string path;
	void to_unix_format();

public:
	FilePath() = default;
	FilePath(const char* path) : path(path) { to_unix_format(); }
	FilePath(std::string&& path) : path(std::move(path)) { to_unix_format(); }
	FilePath& operator=(const char* path_) { path = path_; return *this; }
	FilePath& operator=(std::string&& path_) { path = std::move(path_); return *this; }

	bool operator==(const FilePath& other) const { return path == other.path; }
	FilePath operator/(const FilePath& relative) const
	{
		if (relative.is_relative())
			return path + "/" + relative.path;
		return *this;
	}

	bool is_absolute() const { return std::filesystem::path(path).is_absolute(); }
	bool is_relative() const { return std::filesystem::path(path).is_relative(); }
	std::string native_path() const;

	void clear() { path.clear(); }
	bool empty() { return path.empty(); }
	const char* c_str() const { return path.c_str(); }
	FilePath extension() const;
	bool has_extension(const char* ext) const { return extension() == ext; }
	bool has_any_extension(const char* const* exts, size_t num_exts) const
	{
		for (size_t i = 0; i < num_exts; ++i)
			if (has_extension(exts[i]))
				return true;
		return false;
	}
	size_t hash() const { return std::hash<std::filesystem::path>{}(path); }
};

template<>
struct std::hash<FilePath>
{
	size_t operator()(const FilePath& fp) const { return fp.hash(); }
};

struct FileSystem
{
	static FilePath app_root;
	static FilePath resources_root;
	static FilePath workspace_root;

	static FilePath app_path(const FilePath& relative)
	{
		return relative.is_relative() ? app_root / relative : relative;
	}

	static FilePath resources_path(const FilePath& relative)
	{
		return relative.is_relative() ? resources_root / relative : relative;
	}

	static FilePath workspace_path(const FilePath& relative)
	{
		return relative.is_relative() ? workspace_root / relative : relative;
	}
};

inline FilePath FileSystem::app_root;
inline FilePath FileSystem::resources_root;
inline FilePath FileSystem::workspace_root;
