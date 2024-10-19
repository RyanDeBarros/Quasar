#include "Shader.h"

#include <sstream>

#include "variety/IO.h"

static GLuint compile_shader(GLenum type, const char* shader)
{
	QUASAR_GL(GLuint id = glCreateShader(type));
	QUASAR_GL(glShaderSource(id, 1, &shader, nullptr));
	QUASAR_GL(glCompileShader(id));

	int result;
	QUASAR_GL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		QUASAR_GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		GLchar* message = new GLchar[length];
		QUASAR_GL(glGetShaderInfoLog(id, length, &length, message));
		delete[] message;
		QUASAR_GL(glDeleteShader(id));
		return 0;
	}
	return id;
}

static GLuint load_program(const char* vert, const char* frag)
{
	std::string vertex_shader;
	if (!IO::read_file(vert, vertex_shader))
		QUASAR_ASSERT(false);
	std::string fragment_shader;
	if (!IO::read_file(frag, fragment_shader))
		QUASAR_ASSERT(false);

	QUASAR_GL(GLuint shader = glCreateProgram());
	GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader.c_str());
	GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader.c_str());
	if (vs)
	{
		if (fs)
		{
			QUASAR_GL(glAttachShader(shader, vs));
			QUASAR_GL(glAttachShader(shader, fs));
			QUASAR_GL(glLinkProgram(shader));
			QUASAR_GL(glValidateProgram(shader));

			QUASAR_GL(glDeleteShader(vs));
			QUASAR_GL(glDeleteShader(fs));
		}
		else
		{
			QUASAR_GL(glDeleteProgram(shader));
			QUASAR_GL(glDeleteShader(vs));
			shader = 0;
		}
	}
	else
	{
		QUASAR_GL(glDeleteProgram(shader));
		shader = 0;
	}
	QUASAR_GL(bool valid = glIsProgram(shader));
	QUASAR_ASSERT(valid);
	return shader;
}

static unsigned short stride_of(const std::vector<unsigned short>& attributes)
{
	unsigned short sum = 0;
	for (auto iter = attributes.begin(); iter != attributes.end(); ++iter)
		sum += *iter;
	return sum;
}

Shader::Shader(const char* vertex_shader, const char* fragment_shader, std::vector<unsigned short>&& attribs, std::vector<std::string>&& uniforms)
	: attributes(std::move(attribs))
{
	stride = stride_of(attributes);
	rid = load_program(vertex_shader, fragment_shader);
	for (auto&& uniform : uniforms)
		query_location(std::move(uniform));
}

Shader::Shader(Shader&& other) noexcept
	: rid(other.rid), stride(other.stride), uniform_locations(std::move(other.uniform_locations)), attributes(std::move(other.attributes))
{
	other.rid = 0;
}

Shader::~Shader()
{
	QUASAR_GL(glDeleteProgram(rid));
}

void Shader::query_location(std::string&& uniform)
{
	QUASAR_GL(GLint location = glGetUniformLocation(rid, uniform.c_str()));
	if (!std::signbit(location))
		uniform_locations.emplace(std::move(uniform), location);
}
