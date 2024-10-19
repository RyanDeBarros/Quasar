#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Macros.h"

struct Shader
{
	GLuint rid = 0;
	unsigned short stride = 0;
	std::unordered_map<std::string, GLint> uniform_locations;
	std::vector<unsigned short> attributes;

	Shader(const char* vertex_shader, const char* fragment_shader, std::vector<unsigned short>&& attributes, std::vector<std::string>&& uniforms);
	Shader(const Shader&) = delete;
	Shader(Shader&&) noexcept;
	Shader& operator()(Shader&&) noexcept = delete;
	~Shader();

	operator bool() const { return rid != 0; }

	void query_location(std::string&& uniform);
};
