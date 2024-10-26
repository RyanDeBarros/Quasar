#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Macros.h"
#include "variety/FileSystem.h"

struct Shader
{
	GLuint rid = 0;
	unsigned short stride = 0;
	std::unordered_map<std::string, GLint> uniform_locations;
	std::vector<unsigned short> attributes;
	std::vector<unsigned short> attribute_offsets;

	Shader(const FilePath& vertex_shader, const FilePath& fragment_shader, std::vector<unsigned short>&& attributes, std::vector<std::string>&& uniforms);
	Shader(const Shader&) = delete;
	Shader(Shader&&) noexcept;
	Shader& operator=(Shader&&) noexcept = delete;
	~Shader();

	operator GLuint() const { return rid; }

	void query_location(std::string&& uniform);
};
