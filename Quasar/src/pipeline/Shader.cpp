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

static GLuint load_program(const std::string& vertex_shader, const std::string& fragment_shader)
{
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

Shader::Shader(const FilePath& vertex_shader, const FilePath& fragment_shader)
{
	std::string vert;
	if (!IO.read_file(vertex_shader, vert))
		QUASAR_ASSERT(false);
	std::string frag;
	if (!IO.read_file(fragment_shader, frag))
		QUASAR_ASSERT(false);
	rid = load_program(vert, frag);
	setup();
}

Shader::Shader(const FilePath& vertex_shader, const FilePath& fragment_shader, const std::unordered_map<std::string, std::string>& template_variables)
{
	std::string vert;
	if (!IO.read_template_file(vertex_shader, vert, template_variables))
		QUASAR_ASSERT(false);
	std::string frag;
	if (!IO.read_template_file(fragment_shader, frag, template_variables))
		QUASAR_ASSERT(false);
	rid = load_program(vert, frag);
	setup();
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

static unsigned short float_count(GLenum type)
{
	switch (type)
	{
	case GL_FLOAT: return 1;
	case GL_FLOAT_VEC2: return 2;
	case GL_FLOAT_VEC3: return 3;
	case GL_FLOAT_VEC4: return 4;
	case GL_INT: return 1;
	case GL_INT_VEC2: return 2;
	case GL_INT_VEC3: return 3;
	case GL_INT_VEC4: return 4;
	case GL_FLOAT_MAT2: return 4;
	case GL_FLOAT_MAT3: return 9;
	case GL_FLOAT_MAT4: return 16;
	case GL_FLOAT_MAT2x3: return 6;
	case GL_FLOAT_MAT2x4: return 8;
	case GL_FLOAT_MAT3x2: return 6;
	case GL_FLOAT_MAT3x4: return 12;
	case GL_FLOAT_MAT4x2: return 8;
	case GL_FLOAT_MAT4x3: return 12;
	default: { QUASAR_ASSERT(false); return 0; }
	}
}

void Shader::setup()
{
	load_vertex_attributes();
	setup_attribute_offsets();
	load_uniforms();
}

void Shader::load_vertex_attributes()
{
	GLint num_attributes;
	QUASAR_GL(glGetProgramiv(rid, GL_ACTIVE_ATTRIBUTES, &num_attributes));
	attributes.resize(num_attributes);
	for (GLint i = 0; i < num_attributes; ++i)
	{
		char name[256];
		GLint size;
		GLenum type;
		QUASAR_GL(glGetActiveAttrib(rid, i, sizeof(name), nullptr, &size, &type, name));
		QUASAR_GL(GLint location = glGetAttribLocation(rid, name));
		unsigned short attrib = float_count(type);
		attributes[location] = attrib;
		stride += attrib;
	}
}

void Shader::setup_attribute_offsets()
{
	unsigned short offset = 0;
	for (unsigned short a : attributes)
	{
		attribute_offsets.push_back(offset);
		offset += a;
	}
}

void Shader::load_uniforms()
{
	GLint num_uniforms;
	QUASAR_GL(glGetProgramiv(rid, GL_ACTIVE_UNIFORMS, &num_uniforms));
	for (GLint i = 0; i < num_uniforms; ++i)
	{
		char name[256];
		GLint size;
		GLenum type;
		QUASAR_GL(glGetActiveUniform(rid, i, sizeof(name), nullptr, &size, &type, name));
		QUASAR_GL(GLint location = glGetUniformLocation(rid, name));
		if (location >= 0)
			uniform_locations.emplace(name, location);
	}
}
