#include "FileSystem.h"

#include <algorithm>

void FilePath::to_unix_format()
{
#ifdef _WIN32
	std::replace(path.begin(), path.end(), '\\', '/');
#endif
}

std::string FilePath::native_path() const
{
	std::string native_filepath = path;
#ifdef _WIN32
	std::replace(native_filepath.begin(), native_filepath.end(), '\\', '/');
#endif
	return native_filepath;
}

FilePath FilePath::extension() const
{
	for (long long i = path.size() - 1; i >= 0; --i)
	{
		if (path[i] == '.')
			return path.c_str() + i;
	}
	return "";
}
