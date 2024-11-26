#pragma once

#include <filesystem>

class FilePath
{
	std::string path;
	void to_unix_format();

public:
	FilePath() = default;
	FilePath(const char* path) : path(path ? path : "") { to_unix_format(); }
	FilePath(std::string&& path) : path(std::move(path)) { to_unix_format(); }
	FilePath& operator=(const char* path_) { path = path_ ? path_ : ""; to_unix_format(); return *this; }
	FilePath& operator=(std::string&& path_) { path = std::move(path_); to_unix_format(); return *this; }
	FilePath(const FilePath&) = default;
	FilePath(FilePath&&) noexcept = default;
	FilePath& operator=(const FilePath&) = default;
	FilePath& operator=(FilePath&&) = default;

	bool operator==(const FilePath& other) const { return path == other.path; }
	FilePath operator/(const FilePath& relative) const
	{
		if (path.ends_with("/"))
			return path + relative.path;
		else
			return path + "/" + relative.path;
	}
	FilePath& operator+=(const std::string& addon) { path += addon; }
	FilePath operator+(const std::string& addon) const { FilePath temp = path.c_str(); temp += addon; return temp; }

	bool is_absolute() const { return std::filesystem::path(path).is_absolute(); }
	bool is_relative() const { return std::filesystem::path(path).is_relative(); }
	std::string native_path() const;

	void clear() { path.clear(); }
	bool empty() const { return path.empty(); }
	const char* c_str() const { return path.c_str(); }
	std::string string() const { return path; }
	std::string& path_ref() { return path; }
	const std::string& path_ref() const { return path; }
	std::string filename() const { return std::filesystem::path(path).filename().string(); }
	FilePath extension() const;
	bool has_extension(const char* ext) const;
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
	static FilePath resources_root;
	static FilePath workspace_root;

	static FilePath app_path(const FilePath& relative)
	{
		return relative.is_relative() ? FilePath(".") / relative : relative;
	}

	static FilePath resources_path(const FilePath& relative)
	{
		return relative.is_relative() ? resources_root / relative : relative;
	}

	static FilePath shader_path(const FilePath& relative)
	{
		return relative.is_relative() ? resources_root / "shaders/" / relative : relative;
	}

	static FilePath font_path(const FilePath& relative)
	{
		return relative.is_relative() ? resources_root / "fonts/" / relative : relative;
	}

	static FilePath texture_path(const FilePath& relative)
	{
		return relative.is_relative() ? resources_root / "textures/" / relative : relative;
	}

	static FilePath workspace_path(const FilePath& relative)
	{
		return relative.is_relative() ? workspace_root / relative : relative;
	}
};

inline FilePath FileSystem::resources_root;
inline FilePath FileSystem::workspace_root;
