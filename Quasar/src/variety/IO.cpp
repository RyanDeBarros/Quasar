#include "IO.h"

#include <sstream>

IO IO::io;

bool IO::_read_file(const FilePath& filepath, std::string& content, std::ios_base::openmode mode)
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

std::unordered_map<std::string, AssetValue> IO::_load_asset(const char* filepath, const char* type)
{
	std::unordered_map<std::string, AssetValue> values;
	std::string file;
	if (!_read_file(filepath, file))
		return values;
	std::istringstream iss(file);
	std::string line;
	std::getline(iss, line, '\n');
	if (line != type)
		return values;
	std::string id, value;
	char state = 0;
	while (std::getline(iss, line, '\n'))
	{
		for (char c : line)
		{
			if (state == 0)
			{
				if (c == ':')
					state = 1;
				else
					id += c;
			}
			else
				value += c;
		}
		values[std::move(id)] = AssetValue{ std::move(value) };
		id.clear();
		value.clear();
		state = 0;
	}
	return values;
}
