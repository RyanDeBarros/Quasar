#include "Shader.h"

#include <sstream>

#include "variety/IO.h"

static GLuint compile_shader(GLenum type, const char* shader, const char* filepath)
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
		if (length)
		{
			GLchar* message = new GLchar[length];
			QUASAR_GL(glGetShaderInfoLog(id, length, &length, message));
			LOG << LOG.error << LOG.start << "Shader compilation failed (\"" << filepath << "\"): " << message << LOG.endl;
			delete[] message;
		}
		else
			LOG << LOG.error << LOG.start << "Shader compilation failed (\"" << filepath << "\"): no program info log provided" << LOG.endl;
		QUASAR_GL(glDeleteShader(id));
		return 0;
	}
	return id;
}

static GLuint load_program(const std::string& vertex_shader, const char* vertex_filepath, const std::string& fragment_shader, const char* fragment_filepath)
{
	QUASAR_GL(GLuint shader = glCreateProgram());
	GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader.c_str(), vertex_filepath);
	GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader.c_str(), fragment_filepath);
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
			if (length)
			{
				GLchar* message = new GLchar[length];
				QUASAR_GL(glGetProgramInfoLog(shader, length, &length, message));
				LOG << LOG.error << LOG.start << "Shader linkage failed (vertex=\"" << vertex_filepath << "\", fragment=\"" << fragment_filepath << "\"): " << message << LOG.endl;
				delete[] message;
			}
			else
				LOG << LOG.error << LOG.start << "Shader linkage failed (vertex=\"" << vertex_filepath << "\", fragment=\"" << fragment_filepath << "\"): no program info log provided" << LOG.endl;
			QUASAR_GL(glDeleteProgram(shader));
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

static GLuint load_program(const std::string& vertex_shader, const char* vertex_filepath, const std::string& geometry_shader,
	const char* geometry_filepath, const std::string& fragment_shader, const char* fragment_filepath)
{
	QUASAR_GL(GLuint shader = glCreateProgram());
	GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader.c_str(), vertex_filepath);
	GLuint gs = compile_shader(GL_GEOMETRY_SHADER, geometry_shader.c_str(), geometry_filepath);
	GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader.c_str(), fragment_filepath);
	if (vs && gs && fs)
	{
		QUASAR_GL(glAttachShader(shader, vs));
		QUASAR_GL(glAttachShader(shader, gs));
		QUASAR_GL(glAttachShader(shader, fs));
		QUASAR_GL(glLinkProgram(shader));
		QUASAR_GL(glValidateProgram(shader));

		QUASAR_GL(glDeleteShader(vs));
		QUASAR_GL(glDeleteShader(gs));
		QUASAR_GL(glDeleteShader(fs));

		GLint result;
		QUASAR_GL(glGetProgramiv(shader, GL_LINK_STATUS, &result));
		if (result == GL_FALSE)
		{
			GLint length;
			QUASAR_GL(glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length));
			if (length)
			{
				GLchar* message = new GLchar[length];
				QUASAR_GL(glGetProgramInfoLog(shader, length, &length, message));
				LOG << LOG.error << LOG.start << "Shader linkage failed (vertex=\"" << vertex_filepath << "\", fragment=\"" << fragment_filepath << "\"): " << message << LOG.endl;
				delete[] message;
			}
			else
				LOG << LOG.error << LOG.start << "Shader linkage failed (vertex=\"" << vertex_filepath << "\", fragment=\"" << fragment_filepath << "\"): no program info log provided" << LOG.endl;
			QUASAR_GL(glDeleteProgram(shader));
			shader = 0;
		}
	}
	else
	{
		if (!vs)
		{
			QUASAR_GL(glDeleteShader(vs));
		}
		if (!gs)
		{
			QUASAR_GL(glDeleteShader(gs));
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
	rid = load_program(vert, vertex_shader.c_str(), frag, fragment_shader.c_str());
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
	rid = load_program(vert, vertex_shader.c_str(), frag, fragment_shader.c_str());
	setup();
}

Shader::Shader(const FilePath& vertex_shader, const FilePath& geometry_shader, const FilePath& fragment_shader)
{
	std::string vert;
	if (!IO.read_file(vertex_shader, vert))
		QUASAR_ASSERT(false);
	std::string geom;
	if (!IO.read_file(geometry_shader, geom))
		QUASAR_ASSERT(false);
	std::string frag;
	if (!IO.read_file(fragment_shader, frag))
		QUASAR_ASSERT(false);
	rid = load_program(vert, vertex_shader.c_str(), geom, geometry_shader.c_str(), frag, fragment_shader.c_str());
	setup();
}

Shader::Shader(const FilePath& vertex_shader, const FilePath& geometry_shader, const FilePath& fragment_shader, const std::unordered_map<std::string, std::string>& template_variables)
{
	std::string vert;
	if (!IO.read_template_file(vertex_shader, vert, template_variables))
		QUASAR_ASSERT(false);
	std::string geom;
	if (!IO.read_template_file(geometry_shader, geom, template_variables))
		QUASAR_ASSERT(false);
	std::string frag;
	if (!IO.read_template_file(fragment_shader, frag, template_variables))
		QUASAR_ASSERT(false);
	rid = load_program(vert, vertex_shader.c_str(), geom, geometry_shader.c_str(), frag, fragment_shader.c_str());
	setup();
}

Shader::Shader(const Shader& other)
	: stride(other.stride), uniform_locations(other.uniform_locations), attributes(other.attributes), attribute_offsets(other.attribute_offsets)
{
	if (!other)
		return;
	GLint num_shaders;
	QUASAR_GL(glGetProgramiv(other, GL_ATTACHED_SHADERS, &num_shaders));
	if (num_shaders == 0)
		return;
	GLuint* shaders = new GLuint[num_shaders];
	QUASAR_GL(glGetAttachedShaders(other, num_shaders, nullptr, shaders));
	rid = glCreateProgram();
	for (GLint i = 0; i < num_shaders; ++i)
	{
		QUASAR_GL(glAttachShader(rid, shaders[i]));
	}
	QUASAR_GL(glLinkProgram(rid));
	QUASAR_GL(glValidateProgram(rid));
	GLint result;
	QUASAR_GL(glGetProgramiv(rid, GL_LINK_STATUS, &result));
	if (result == GL_FALSE)
	{
		GLint length;
		QUASAR_GL(glGetProgramiv(rid, GL_INFO_LOG_LENGTH, &length));
		if (length)
		{
			GLchar* message = new GLchar[length];
			QUASAR_GL(glGetProgramInfoLog(rid, length, &length, message));
			LOG << LOG.error << LOG.start << "Shader copy linkage failed: " << message << LOG.endl;
			delete[] message;
		}
		else
			LOG << LOG.error << LOG.start << "Shader copy linkage failed: no program info log provided" << LOG.endl;
		QUASAR_GL(glDeleteProgram(rid));
		rid = 0;
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
	default:
	{
		LOG << LOG.error << LOG.start << "Cannot parse number of floats in type (" << type << ')' << LOG.nl;
		QUASAR_ASSERT(false);
		return 0;
	}
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
