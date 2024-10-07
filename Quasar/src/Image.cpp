#include "Image.h"

#include <stb/stb_image.h>
#include <memory>

#include "GLutility.h"

Image::Image(const ImageConstructor& args)
{
	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load(args.filepath.c_str(), &width, &height, &bpp, 0);
	deletion_policy = ImageDeletionPolicy::FROM_STBI;
}

Image::Image(const Image& other)
	: width(other.width), height(other.height), bpp(other.bpp)
{
	deletion_policy = ImageDeletionPolicy::FROM_NEW;
	size_t size = static_cast<size_t>(width) * height * bpp;
	pixels = new unsigned char[size];
	memcpy(pixels, other.pixels, size);
}

Image::Image(Image&& other) noexcept
	: pixels(other.pixels), width(other.width), height(other.height), bpp(other.bpp), deletion_policy(other.deletion_policy)
{
	other.pixels = nullptr;
}

static void delete_image(const Image& image)
{
	if (image.deletion_policy == ImageDeletionPolicy::FROM_STBI) [[likely]]
		stbi_image_free(image.pixels);
	else if (image.deletion_policy == ImageDeletionPolicy::FROM_NEW)
		delete[] image.pixels;
}

Image& Image::operator=(const Image& other)
{
	if (this != &other)
	{
		if (pixels)
			delete_image(*this);
		deletion_policy = ImageDeletionPolicy::FROM_NEW;
		width = other.width;
		height = other.height;
		bpp = other.bpp;
		size_t size = static_cast<size_t>(width) * height * bpp;
		pixels = new unsigned char[size];
		memcpy(pixels, other.pixels, size);
	}
	return *this;
}

Image& Image::operator=(Image&& other) noexcept
{
	if (this != &other)
	{
		if (pixels)
			delete_image(*this);
		pixels = other.pixels;
		width = other.width;
		height = other.height;
		bpp = other.bpp;
		deletion_policy = other.deletion_policy;
		other.pixels = nullptr;
	}
	return *this;
}

Image::~Image()
{
	if (pixels)
		delete_image(*this);
}

void Image::gen_texture(const TextureParams& texture_params)
{
	if (tid == 0)
	{
		QUASAR_GL(glGenTextures(1, &tid));
		bind_texture(tid);
		GLint internal_format = 0;
		GLenum format = 0;
		GLint alignment = 0;
		if (bpp == 4)
		{
			internal_format = GL_RGBA8;
			format = GL_RGBA;
			alignment = 4;
		}
		else if (bpp == 3)
		{
			internal_format = GL_RGB8;
			format = GL_RGB;
			alignment = 1;
		}
		else if (bpp == 2)
		{
			internal_format = GL_RG8;
			format = GL_RG;
			alignment = 2;
		}
		else if (bpp == 1)
		{
			internal_format = GL_R8;
			format = GL_RED;
			alignment = 1;
		}
		QUASAR_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));
		QUASAR_GL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels));
		bind_texture_params(texture_params);
	}
}

void Image::update_texture(const TextureParams& texture_params) const
{
	if (tid != 0)
	{
		bind_texture(tid);
		bind_texture_params(texture_params);
	}
}

void Image::send_subtexture(GLint x, GLint y, GLsizei w, GLsizei h) const
{
	bind_texture(tid);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, bpp_format(), GL_UNSIGNED_BYTE, pixels);
}

GLenum Image::bpp_format() const
{
	if (bpp == 4)
		return GL_RGBA;
	else if (bpp == 3)
		return GL_RGB;
	else if (bpp == 2)
		return GL_RG;
	else if (bpp == 1)
		return GL_RED;
	else
		return 0;
}
