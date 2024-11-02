#pragma once

#include "Shader.h"

namespace Uniforms
{
	extern void send_1(const Shader& shader, const char* uniform, float value, GLint uniform_offset = 0, bool unbind = false);
	extern void send_4(const Shader& shader, const char* uniform, glm::vec4 value, GLint uniform_offset = 0, bool unbind = false);
	extern void send_matrix3(const Shader& shader, const char* uniform, const glm::mat3& value, GLint uniform_offset = 0, bool unbind = false);
}
