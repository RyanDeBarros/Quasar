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

	Shader(const FilePath& vertex_shader, const FilePath& fragment_shader);
	Shader(const FilePath& vertex_shader, const FilePath& fragment_shader, const std::unordered_map<std::string, std::string>& template_variables);
	Shader(const Shader&) = delete;
	Shader(Shader&&) noexcept;
	Shader& operator=(Shader&&) noexcept = delete;
	~Shader();

	operator GLuint() const { return rid; }

private:
	void setup();
	void load_vertex_attributes();
	void setup_attribute_offsets();
	void load_uniforms();
};
