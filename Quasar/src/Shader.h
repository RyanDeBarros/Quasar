#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Macros.h"

struct Shader
{
	GLuint rid;
	unsigned short stride;
	std::unordered_map<std::string, GLint> locations;
	std::vector<unsigned short> attributes;

	~Shader();

	void query_location(std::string&& attribute);
};

extern Shader load_shader(const char* assetfile = "res/rect_shader.quasar");
