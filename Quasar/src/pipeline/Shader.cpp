#include "Shader.h"

#include <sstream>

#include "variety/IO.h"

static GLuint compile_shader(GLenum type, const char* shader, const char* filepath)
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
		std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: \"" << filepath << "\"\n" << message << std::endl;
		delete[] message;
		QUASAR_GL(glDeleteShader(id));
		return 0;
	}
	return id;
}

static GLuint load_program(const char* vert, const char* frag)
{
	std::string vertex_shader;
	if (!IO::read_file("res/standard.vert", vertex_shader))
		QUASAR_ASSERT(false);
	std::string fragment_shader;
	if (!IO::read_file("res/standard.frag", fragment_shader))
		QUASAR_ASSERT(false);

	QUASAR_GL(GLuint shader = glCreateProgram());
	GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader.c_str(), "res/standard.vert");
	GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader.c_str(), "res/standard.frag");
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
	return shader;
}

static unsigned short stride_of(const std::vector<unsigned short>& attributes)
{
	unsigned short sum = 0;
	for (auto iter = attributes.begin(); iter != attributes.end(); ++iter)
		sum += *iter;
	return sum;
}

Shader::Shader(const ShaderConstructor& args)
{
	std::string vert, frag;
	auto values = IO::load_asset(args.filepath.c_str(), "shader");
	auto iter = values.find("VERT");
	if (iter != values.end())
		vert = std::move(iter->second.moving_parse());
	iter = values.find("FRAG");
	if (iter != values.end())
		frag = std::move(iter->second.moving_parse());
	iter = values.find("ATTRIBS");
	if (iter != values.end())
		attributes = std::move(iter->second.parse<std::vector<unsigned short>>());

	stride = stride_of(attributes);
	rid = load_program(vert.c_str() + 2, frag.c_str() + 2);
}

Shader::~Shader()
{
	QUASAR_GL(glDeleteProgram(rid));
}

void Shader::query_location(std::string&& attribute)
{
	QUASAR_GL(GLint location = glGetUniformLocation(rid, attribute.c_str()));
	if (!std::signbit(location))
		locations.emplace(std::move(attribute), location);
}
