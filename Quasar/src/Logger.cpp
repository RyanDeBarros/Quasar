#include "Logger.h"

#include <iostream>

Logger& Logger::operator<<(const _nl& nl)
{
	return *this << '\n';
}

Logger& Logger::operator<<(const _flush& flush)
{
	std::cout << stream.str(); // TODO logger target
	stream.str(std::string());
	stream.clear();
	return *this;
}

Logger& Logger::operator<<(const _endl& endl)
{
	return *this << nl << flush;
}

Logger& Logger::operator<<(const _start& start)
{
	// TODO only log for level if that level is enabled
	switch (level)
	{
	case Level::DEBUG:
		return *this << "[Debug] ";
	case Level::INFO:
		return *this << "[Info] ";
	case Level::WARNING:
		return *this << "[Warning] ";
	case Level::ERROR:
		return *this << "[Error] ";
	case Level::FATAL:
		return *this << "[Fatal] ";
	}
	return *this;
}

Logger& Logger::operator<<(const _start_gl& start_gl)
{
	// TODO only log for level if that level is enabled
	switch (level)
	{
	case Level::DEBUG:
		return *this << "[Debug - OpenGL(" << start_gl.code << ")] ";
	case Level::INFO:
		return *this << "[Info - OpenGL(" << start_gl.code << ")] ";
	case Level::WARNING:
		return *this << "[Warning - OpenGL(" << start_gl.code << ")] ";
	case Level::ERROR:
		return *this << "[Error - OpenGL(" << start_gl.code << ")] ";
	case Level::FATAL:
		return *this << "[Fatal - OpenGL(" << start_gl.code << ")] ";
	}
	return *this;
}

Logger& Logger::operator<<(const _start_glfw& start_glfw)
{
	// TODO only log for level if that level is enabled
	switch (level)
	{
	case Level::DEBUG:
		return *this << "[Debug - GLFW(" << start_glfw.code << ")] ";
	case Level::INFO:
		return *this << "[Info - GLFW(" << start_glfw.code << ")] ";
	case Level::WARNING:
		return *this << "[Warning - GLFW(" << start_glfw.code << ")] ";
	case Level::ERROR:
		return *this << "[Error - GLFW(" << start_glfw.code << ")] ";
	case Level::FATAL:
		return *this << "[Fatal - GLFW(" << start_glfw.code << ")] ";
	}
	return *this;
}

Logger& Logger::operator<<(const _start_toml& start_toml)
{
	// TODO only log for level if that level is enabled
	switch (level)
	{
	case Level::DEBUG:
		return *this << "[Debug - TOML] ";
	case Level::INFO:
		return *this << "[Info - TOML] ";
	case Level::WARNING:
		return *this << "[Warning - TOML] ";
	case Level::ERROR:
		return *this << "[Error - TOML] ";
	case Level::FATAL:
		return *this << "[Fatal - TOML] ";
	}
	return *this;
}

Logger& Logger::operator<<(const _level_debug& debug)
{
	level = Level::DEBUG;
	return *this;
}

Logger& Logger::operator<<(const _level_info& info)
{
	level = Level::INFO;
	return *this;
}

Logger& Logger::operator<<(const _level_warning& warning)
{
	level = Level::WARNING;
	return *this;
}

Logger& Logger::operator<<(const _level_error& error)
{
	level = Level::ERROR;
	return *this;
}

Logger& Logger::operator<<(const _level_fatal& fatal)
{
	level = Level::FATAL;
	return *this;
}

Logger& Logger::operator<<(const void* x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(bool x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(char text)
{
	stream << text;
	return *this;
}

Logger& Logger::operator<<(const char* text)
{
	stream << text;
	return *this;
}

Logger& Logger::operator<<(short x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(int x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(long x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(long long x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(unsigned char text)
{
	stream << text;
	return *this;
}

Logger& Logger::operator<<(const unsigned char* text)
{
	stream << text;
	return *this;
}

Logger& Logger::operator<<(unsigned short x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(unsigned int x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(unsigned long x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(unsigned long long x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(float x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(double x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(long double x)
{
	stream << x;
	return *this;
}

Logger& Logger::operator<<(const std::string& str)
{
	stream << str;
	return *this;
}

Logger& Logger::operator<<(const std::string_view& str)
{
	stream << str;
	return *this;
}
