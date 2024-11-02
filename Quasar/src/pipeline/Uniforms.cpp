#include "Uniforms.h"

#include <glm/gtc/type_ptr.inl>

#include "variety/GLutility.h"

void Uniforms::send_1(const Shader& shader, const char* uniform, float value, GLint uniform_offset, bool unbind)
{
	auto iter = shader.uniform_locations.find(uniform);
	if (iter != shader.uniform_locations.end())
	{
		bind_shader(shader);
		QUASAR_GL(glUniform1fv(iter->second + uniform_offset, 1, &value));
		if (unbind)
			unbind_shader();
	}
	else
	{
		std::cerr << "Uniform \"" << uniform << "\" does not exist in shader (" << shader.rid << ")." << std::endl;
	}
}

void Uniforms::send_4(const Shader& shader, const char* uniform, glm::vec4 value, GLint uniform_offset, bool unbind)
{
	auto iter = shader.uniform_locations.find(uniform);
	if (iter != shader.uniform_locations.end())
	{
		bind_shader(shader);
		QUASAR_GL(glUniform4fv(iter->second + uniform_offset, 1, glm::value_ptr(value)));
		if (unbind)
			unbind_shader();
	}
	else
	{
		std::cerr << "Uniform \"" << uniform << "\" does not exist in shader (" << shader.rid << ")." << std::endl;
	}
}

void Uniforms::send_matrix3(const Shader& shader, const char* uniform, const glm::mat3& value, GLint uniform_offset, bool unbind)
{
	auto iter = shader.uniform_locations.find(uniform);
	if (iter != shader.uniform_locations.end())
	{
		bind_shader(shader);
		QUASAR_GL(glUniformMatrix3fv(iter->second + uniform_offset, 1, GL_FALSE, glm::value_ptr(value)));
		if (unbind)
			unbind_shader();
	}
	else
	{
		std::cerr << "Uniform \"" << uniform << "\" does not exist in shader (" << shader.rid << ")." << std::endl;
	}
}