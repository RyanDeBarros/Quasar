#include "Shader.h"

#include <sstream>

#include "variety/IO.h"

static GLuint compile_shader(GLenum type, const char* shader)
{
	QUASAR_GL(GLuint id = glCreateShader(type));
	QUASAR_GL(glShaderSource(id, 1, &shader, nullptr));
	QUASAR_GL(glCompileShader(id));

	GLint result;
	QUASAR_GL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		GLint length;
		QUASAR_GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		GLchar* message = new GLchar[length];
		QUASAR_GL(glGetShaderInfoLog(id, length, &length, message));
		std::cerr << "Shader compilation failed: " << message << std::endl;
		delete[] message;
		QUASAR_GL(glDeleteShader(id));
		return 0;
	}
	return id;
}

static GLuint load_program(const FilePath& vert, const FilePath& frag)
{
	std::string vertex_shader;
	if (!IO.read_file(vert, vertex_shader))
		QUASAR_ASSERT(false);
	std::string fragment_shader;
	if (!IO.read_file(frag, fragment_shader))
		QUASAR_ASSERT(false);

	QUASAR_GL(GLuint shader = glCreateProgram());
	GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader.c_str());
	GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader.c_str());
	if (vs && fs)
	{
		QUASAR_GL(glAttachShader(shader, vs));
		QUASAR_GL(glAttachShader(shader, fs));
		QUASAR_GL(glLinkProgram(shader));
		QUASAR_GL(glValidateProgram(shader));

		QUASAR_GL(glDeleteShader(vs));
		QUASAR_GL(glDeleteShader(fs));

		GLint result;
		QUASAR_GL(glGetProgramiv(shader, GL_LINK_STATUS, &result));
		if (result == GL_FALSE)
		{
			GLint length;
			QUASAR_GL(glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length));
			GLchar* message = new GLchar[length];
			QUASAR_GL(glGetProgramInfoLog(shader, length, &length, message));
			std::cerr << "Shader linking failed: " << message << std::endl;
			delete[] message;
			QUASAR_GL(glDeleteShader(shader));
			shader = 0;
		}
	}
	else
	{
		if (!vs)
		{
			QUASAR_GL(glDeleteShader(vs));
		}
		if (!fs)
		{
			QUASAR_GL(glDeleteShader(fs));
		}
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

Shader::Shader(const FilePath& vertex_shader, const FilePath& fragment_shader, std::vector<unsigned short>&& attribs, std::vector<std::string>&& uniforms)
	: attributes(std::move(attribs)) // TODO query attributes programatically so it isn't necessary to pass. same with uniforms.
{
	stride = stride_of(attributes);
	rid = load_program(vertex_shader, fragment_shader);
	for (auto&& uniform : uniforms)
		query_location(std::move(uniform));

	unsigned short offset = 0;
	for (unsigned short a : attributes)
	{
		attribute_offsets.push_back(offset);
		offset += a;
	}
}

Shader::Shader(Shader&& other) noexcept
	: rid(other.rid), stride(other.stride), uniform_locations(std::move(other.uniform_locations)),
	attributes(std::move(other.attributes)), attribute_offsets(std::move(other.attribute_offsets))
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
