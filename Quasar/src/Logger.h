#pragma once

#include <sstream>

class Logger
{
	Logger() = default;
	Logger(const Logger&) = delete;
	Logger(Logger&&) noexcept = delete;
	~Logger() = default;

	std::stringstream stream;

public:

	static Logger& instance() { static Logger logger; return logger; }

	struct _nl {} nl;
	struct _flush {} flush;
	struct _endl {} endl;

	Logger& operator<<(const _nl& nl);
	Logger& operator<<(const _flush& flush);
	Logger& operator<<(const _endl& endl);

	Logger& operator<<(const char* text);
	Logger& operator<<(const unsigned char* text);
};

inline Logger& LOG = Logger::instance();
