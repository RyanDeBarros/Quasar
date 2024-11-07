#pragma once

#include <glm/glm.hpp>

#include <sstream>
#include <fstream>

class Logger
{
	Logger() = default;
	Logger(const Logger&) = delete;
	Logger(Logger&&) noexcept = delete;
	~Logger() = default;

	std::stringstream stream;

	enum class Level
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		FATAL
	} level = Level::INFO;

	struct _start_gl { long long code = 0; } start_gl_impl;
	struct _start_glfw { long long code = 0; } start_glfw_impl;
	
	bool targeting_console = true;
	bool targeting_logfile = false;

	std::ofstream file;

public:
	static Logger& instance() { static Logger logger; return logger; }

	struct _nl {} nl;
	struct _flush {} flush;
	struct _endl {} endl;

	struct _start {} start;
	_start_gl& start_gl(long long code) { start_gl_impl.code = code; return start_gl_impl; }
	_start_glfw& start_glfw(long long code) { start_glfw_impl.code = code; return start_glfw_impl; }
	struct _start_toml {} start_toml;

	bool enable_debug = true;
	struct _level_debug {} debug;
	bool enable_info = true;
	struct _level_info {} info;
	bool enable_warning = true;
	struct _level_warning {} warning;
	bool enable_error = true;
	struct _level_error {} error;
	bool enable_fatal = true;
	struct _level_fatal {} fatal;

	Logger& specify_logfile(const char* filepath, bool append = false);
	struct _target_console {} target_console;
	struct _target_logfile {} target_logfile;
	struct _untarget_console {} untarget_console;
	struct _untarget_logfile {} untarget_logfile;

	Logger& operator<<(const _target_console& target_console);
	Logger& operator<<(const _target_logfile& target_logfile);
	Logger& operator<<(const _untarget_console& untarget_console);
	Logger& operator<<(const _untarget_logfile& untarget_logfile);

	Logger& operator<<(const _nl& nl);
	Logger& operator<<(const _flush& flush);
	Logger& operator<<(const _endl& endl);

	Logger& operator<<(const _start& start);
	Logger& operator<<(const _start_gl& start_gl);
	Logger& operator<<(const _start_glfw& start_glfw);
	Logger& operator<<(const _start_toml& start_toml);
	Logger& operator<<(const _level_debug& debug);
	Logger& operator<<(const _level_info& info);
	Logger& operator<<(const _level_warning& warning);
	Logger& operator<<(const _level_error& error);
	Logger& operator<<(const _level_fatal& fatal);

	Logger& operator<<(const void* x);
	Logger& operator<<(bool x);
	Logger& operator<<(char text);
	Logger& operator<<(const char* text);
	Logger& operator<<(short x);
	Logger& operator<<(int x);
	Logger& operator<<(long x);
	Logger& operator<<(long long x);
	Logger& operator<<(unsigned char text);
	Logger& operator<<(const unsigned char* text);
	Logger& operator<<(unsigned short x);
	Logger& operator<<(unsigned int x);
	Logger& operator<<(unsigned long x);
	Logger& operator<<(unsigned long long x);
	Logger& operator<<(float x);
	Logger& operator<<(double x);
	Logger& operator<<(long double x);

	Logger& operator<<(const std::string& str);
	Logger& operator<<(const std::string_view& str);
};

inline Logger& LOG = Logger::instance();

extern Logger& operator<<(Logger&, glm::vec2);
extern Logger& operator<<(Logger&, glm::vec3);
extern Logger& operator<<(Logger&, glm::vec4);
extern Logger& operator<<(Logger&, const glm::mat2&);
extern Logger& operator<<(Logger&, const glm::mat3&);
extern Logger& operator<<(Logger&, const glm::mat4&);
