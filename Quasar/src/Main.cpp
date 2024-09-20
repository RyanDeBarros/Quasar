#include <stb/stb_image.h>

#include "Macros.h"
#include "IO.h"
#include "Sprite.h"

constexpr unsigned short QUASAR_MAX_VERTICES = 1024;
constexpr unsigned short QUASAR_MAX_INDEXES = 2048;

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "[GLFW ERROR " << error << "]: " << description << std::endl;
	QUASAR_ASSERT(false);
}

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

int main()
{
	if (glfwInit() != GLFW_TRUE)
		return -1;
	glfwSetErrorCallback(glfw_error_callback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	int window_width = 1440;
	int window_height = 1080;
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Quasor", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}
	glfwSwapInterval(1);
	QUASAR_GL(glClearColor(0.5f, 0.5f, 0.5f, 0.5f));
	QUASAR_GL(std::cout << "Welcome to Quasar - GL_VERSION: " << glGetString(GL_VERSION) << std::endl);

	GLfloat* vertex_pool = new GLfloat[QUASAR_MAX_VERTICES];
	GLuint* index_pool = new GLuint[QUASAR_MAX_INDEXES];
	GLuint vao, vb, ib;

	QUASAR_GL(glGenVertexArrays(1, &vao));
	QUASAR_GL(glBindVertexArray(vao));
	
	QUASAR_GL(glGenBuffers(1, &vb));
	QUASAR_GL(glBindBuffer(GL_ARRAY_BUFFER, vb));
	QUASAR_GL(glBufferData(GL_ARRAY_BUFFER, QUASAR_MAX_VERTICES * sizeof(GLfloat), vertex_pool, GL_DYNAMIC_DRAW));
	QUASAR_GL(glGenBuffers(1, &ib));
	QUASAR_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib));
	QUASAR_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, QUASAR_MAX_INDEXES * sizeof(GLuint), index_pool, GL_DYNAMIC_DRAW));

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

	QUASAR_GL(glUseProgram(shader));

	stbi_set_flip_vertically_on_load(true);
	Image img;
	img.pixels = stbi_load("ex/tux.png", &img.width, &img.height, &img.bpp, 0);
	img.deletion_policy = ImageDeletionPolicy::FROM_STBI;
	GLint internal_format;
	GLenum format;
	if (img.bpp == 4)
	{
		internal_format = GL_RGBA8;
		format = GL_RGBA;
	}
	else if (img.bpp == 3)
	{
		internal_format = GL_RGB8;
		format = GL_RGB;
	}
	else if (img.bpp == 2)
	{
		internal_format = GL_RG8;
		format = GL_RG;
	}
	else if (img.bpp == 1)
	{
		internal_format = GL_R8;
		format = GL_RED;
	}
	
	Sprite sprite;
	sprite.img = new ImageReferencer{1};
	sprite.img->image = std::move(img);
	sprite.buf = new BufferReferencer{1};
	sprite.tex = new TextureReferencer{1};
	QUASAR_GL(glGenTextures(1, &sprite.tex->texture));

	QUASAR_GL(glActiveTexture(GL_TEXTURE0 + 0));
	QUASAR_GL(glBindTexture(GL_TEXTURE_2D, sprite.tex->texture));
	QUASAR_GL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, sprite.img->image.width, sprite.img->image.height,
		0, format, GL_UNSIGNED_BYTE, sprite.img->image.pixels));
	TextureParams params;
	bind_texture_params(params);

	sprite.buf->stride = 14;
	sprite.buf->vlen_bytes = 4 * sprite.buf->stride * sizeof(GLfloat);
	sprite.buf->varr = new GLfloat[4 * sprite.buf->stride]{
		0.0f, -0.5f * img.width, -0.5f * img.height, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.3f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f,  0.5f * img.width, -0.5f * img.height, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.3f, 1.0f, 1.0f, 0.0f, 1.0f,
		2.0f,  0.5f * img.width,  0.5f * img.height, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.3f, 0.0f, 1.0f, 1.0f, 1.0f,
		3.0f, -0.5f * img.width,  0.5f * img.height, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.3f, 1.0f, 0.0f, 0.0f, 1.0f
	};
	sprite.buf->ilen_bytes = 6 * sizeof(GLuint);
	sprite.buf->iarr = new GLuint[6]{
		0, 1, 2,
		2, 3, 0
	};


	int offset = 0;
	int i = 0;
	int len = 1;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, sprite.buf->stride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 2;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, sprite.buf->stride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 1;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, sprite.buf->stride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 2;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, sprite.buf->stride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 4;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, sprite.buf->stride * sizeof(GL_FLOAT), (const GLvoid*)offset));
	offset += len * sizeof(GLfloat);
	len = 4;
	++i;
	QUASAR_GL(glEnableVertexAttribArray(i));
	QUASAR_GL(glVertexAttribPointer(i, len, GL_FLOAT, GL_FALSE, sprite.buf->stride * sizeof(GL_FLOAT), (const GLvoid*)offset));

	memcpy(vertex_pool, sprite.buf->varr, sprite.buf->vlen_bytes);
	memcpy(index_pool, sprite.buf->iarr, sprite.buf->ilen_bytes);

	QUASAR_GL(glBufferSubData(GL_ARRAY_BUFFER, 0, sprite.buf->vlen_bytes, vertex_pool));
	QUASAR_GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sprite.buf->ilen_bytes, index_pool));

	QUASAR_GL(glEnable(GL_BLEND));
	QUASAR_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	glm::mat3 proj = glm::ortho<float>(0.0f, static_cast<float>(window_width), 0.0f, static_cast<float>(window_height));
	glm::mat3 vp = proj;
	QUASAR_GL(GLint vp_loc = glGetUniformLocation(shader, "u_VP"));
	QUASAR_GL(glUniformMatrix3fv(vp_loc, 1, GL_FALSE, &vp[0][0]));

	for (glfwPollEvents(); !glfwWindowShouldClose(window); glfwPollEvents())
	{

		QUASAR_GL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));




		glfwSwapBuffers(window);
		QUASAR_GL(glClear(GL_COLOR_BUFFER_BIT));
	}

	QUASAR_GL(glDeleteProgram(shader));
	QUASAR_GL(glDeleteBuffers(1, &vao));
	QUASAR_GL(glDeleteBuffers(1, &vb));
	QUASAR_GL(glDeleteBuffers(1, &ib));
	delete[] vertex_pool;

	return 0;
}
