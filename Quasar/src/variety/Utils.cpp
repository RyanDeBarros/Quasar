#include "Utils.h"

#include <cstring>

const char* query_file_extension(const char* filepath, size_t filepath_len)
{
	if (filepath_len > 1)
	{
		const char* character = filepath + filepath_len - 2;
		while (character != filepath)
		{
			if (*character == '.')
				return character + 1;
			character--;
		}
	}
	return nullptr;
}

bool file_extension_is_in(const char* filepath, size_t filepath_len, const char* const* extensions, size_t num_extensions)
{
	if (filepath_len > 1 && num_extensions > 0)
	{
		const char* character = filepath + filepath_len - 2;
		while (character != filepath)
		{
			if (*character == '.')
			{
				for (size_t i = 0; i < num_extensions; ++i)
					if (strcmp(character + 1, extensions[i]) == 0)
						return true;
				break;
			}
			character--;
		}
	}
	return false;
}
