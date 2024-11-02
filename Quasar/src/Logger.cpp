#include "Logger.h"

#include <iostream>

Logger& Logger::operator<<(const _nl& nl)
{
	stream << '\n';
	return *this;
}

Logger& Logger::operator<<(const _flush& flush)
{
	std::cout << stream.str(); // TODO logger target
	stream.clear();
	return *this;
}

Logger& Logger::operator<<(const _endl& endl)
{
	return *this << nl << flush;
}

Logger& Logger::operator<<(const char* text)
{
	stream << text;
	return *this;
}

Logger& Logger::operator<<(const unsigned char* text)
{
	stream << text;
	return *this;
}
