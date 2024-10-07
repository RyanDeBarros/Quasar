#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Macros.h"

struct ShaderConstructor
{
	std::string filepath = "res/rect_shader.asset";

	bool operator==(const ShaderConstructor&) const = default;
};

template<>
struct std::hash<ShaderConstructor>
{
	size_t operator()(const ShaderConstructor& sc) const { return std::hash<std::string>{}(sc.filepath); }
};

struct Shader
{
	GLuint rid = 0;
	unsigned short stride = 0;
	std::unordered_map<std::string, GLint> locations;
	std::vector<unsigned short> attributes;

	Shader(const ShaderConstructor& args = {});
	Shader(const Shader&) = default;
	Shader(Shader&&) noexcept = default;
	~Shader();

	operator bool() const { return rid != 0; }

	void query_location(std::string&& attribute);
};
