#pragma once

#include <fstream>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>
#include <functional>

#include "variety/FileSystem.h"

class IO
{
	IO() = default;
	IO(const IO&) = delete;
	IO(IO&&) = delete;
	~IO() = default;

	static IO io;

	bool _read_file(const FilePath& filepath, std::string& content, std::ios_base::openmode mode = std::ios_base::in);

public:
	static bool read_file(const FilePath& filepath, std::string& content, std::ios_base::openmode mode = std::ios_base::in)
	{
		return io._read_file(filepath, content, mode);
	}
};
