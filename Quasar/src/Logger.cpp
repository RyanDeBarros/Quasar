#include "Logger.h"

#include <iostream>
#include <chrono>

// LATER translate OpenGL error codes

Logger& Logger::specify_logfile(const char* filepath, bool append)
{
	file.close();
	if (append)
		file.open(filepath, std::ios_base::app);
	else
		file.open(filepath);

	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	auto current_time = std::localtime(&time);
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	const char log_start[] = "--- LOG started at ";
	const char log_end[] = " ---";
	file << std::setfill('-') << std::setw(sizeof(log_start) - 1 + 23 + sizeof(log_end) - 1) << "" << '\n';
	file << log_start;
	file << std::put_time(current_time, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
	file << log_end << '\n';
	file << std::setfill('-') << std::setw(sizeof(log_start) - 1 + 23 + sizeof(log_end) - 1) << "" << '\n';
	file.flush();

	return *this;
}

Logger& Logger::operator<<(const _target_console& target_console)
{
	targeting_console = true;
	return *this;
}

Logger& Logger::operator<<(const _target_logfile& target_logfile)
{
	targeting_logfile = true;
	return *this;
}

Logger& Logger::operator<<(const _untarget_console& untarget_console)
{
	targeting_console = false;
	return *this;
}

Logger& Logger::operator<<(const _untarget_logfile& untarget_logfile)
{
	targeting_logfile = false;
	return *this;
}

Logger& Logger::operator<<(const _nl& nl)
{
	return *this << '\n';
}

Logger& Logger::operator<<(const _flush& flush)
{
	if (targeting_logfile)
	{
		file << stream.str();
		file.flush();
	}
	if (targeting_console)
	{
		std::cout << stream.str();
		std::cout.flush();
	}
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
	switch (level)
	{
	case Level::DEBUG:
		if (enable_debug)
			return *this << "[Debug] ";
		break;
	case Level::INFO:
		if (enable_info)
			return *this << "[Info] ";
		break;
	case Level::WARNING:
		if (enable_warning)
			return *this << "[Warning] ";
		break;
	case Level::ERROR:
		if (enable_error)
			return *this << "[Error] ";
		break;
	case Level::FATAL:
		if (enable_fatal)
			return *this << "[Fatal] ";
		break;
	}
	return *this;
}

Logger& Logger::operator<<(const _start_gl& start_gl)
{
	switch (level)
	{
	case Level::DEBUG:
		if (enable_debug)
			return *this << "[Debug - OpenGL(" << start_gl.code << ")] ";
		break;
	case Level::INFO:
		if (enable_info)
			return *this << "[Info - OpenGL(" << start_gl.code << ")] ";
		break;
	case Level::WARNING:
		if (enable_warning)
			return *this << "[Warning - OpenGL(" << start_gl.code << ")] ";
		break;
	case Level::ERROR:
		if (enable_error)
			return *this << "[Error - OpenGL(" << start_gl.code << ")] ";
		break;
	case Level::FATAL:
		if (enable_fatal)
			return *this << "[Fatal - OpenGL(" << start_gl.code << ")] ";
		break;
	}
	return *this;
}

Logger& Logger::operator<<(const _start_glfw& start_glfw)
{
	switch (level)
	{
	case Level::DEBUG:
		if (enable_debug)
			return *this << "[Debug - GLFW(" << start_glfw.code << ")] ";
		break;
	case Level::INFO:
		if (enable_info)
			return *this << "[Info - GLFW(" << start_glfw.code << ")] ";
		break;
	case Level::WARNING:
		if (enable_warning)
			return *this << "[Warning - GLFW(" << start_glfw.code << ")] ";
		break;
	case Level::ERROR:
		if (enable_error)
			return *this << "[Error - GLFW(" << start_glfw.code << ")] ";
		break;
	case Level::FATAL:
		if (enable_fatal)
			return *this << "[Fatal - GLFW(" << start_glfw.code << ")] ";
		break;
	}
	return *this;
}

Logger& Logger::operator<<(const _start_toml& start_toml)
{
	switch (level)
	{
	case Level::DEBUG:
		if (enable_debug)
			return *this << "[Debug - TOML] ";
		break;
	case Level::INFO:
		if (enable_info)
			return *this << "[Info - TOML] ";
		break;
	case Level::WARNING:
		if (enable_warning)
			return *this << "[Warning - TOML] ";
		break;
	case Level::ERROR:
		if (enable_error)
			return *this << "[Error - TOML] ";
		break;
	case Level::FATAL:
		if (enable_fatal)
			return *this << "[Fatal - TOML] ";
		break;
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
