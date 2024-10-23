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
	std::replace(native_filepath.begin(), native_filepath.end(), '/', '\\');
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

bool FilePath::has_extension(const char* ext) const
{
	auto actual = extension();
	size_t i = 0;
	while (i < actual.path.size())
	{
		if (ext[i] == '\0')
			return false;
		if (toupper(ext[i]) != toupper(actual.path[i]))
			return false;
		++i;
	}
	return ext[i] == '\0';
}
