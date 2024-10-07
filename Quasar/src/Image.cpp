#include "Image.h"

#include <stb/stb_image.h>
#include <memory>

#include "GLutility.h"

static void delete_buffer(Image& image)
{
	stbi_image_free(image.pixels); // equivalent to delete[] image.pixels
}

static void delete_texture(Image& image)
{
	QUASAR_GL(glDeleteTextures(1, &image.tid));
}

Image::Image(const ImageConstructor& args)
{
	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load(args.filepath.c_str(), &width, &height, &bpp, 0);
	if (args.gen_texture)
		gen_texture();
}

Image::Image(const Image& other)
	: width(other.width), height(other.height), bpp(other.bpp)
{
	pixels = new byte[area()];
	memcpy(pixels, other.pixels, area());
	if (other.tid != 0)
		gen_texture();
}

Image::Image(Image&& other) noexcept
	: width(other.width), height(other.height), bpp(other.bpp), pixels(other.pixels), tid(other.tid)
{
	other.pixels = nullptr;
	other.tid = 0;
}

Image& Image::operator=(const Image& other)
{
	if (this != &other)
	{
		delete_buffer(*this);
		delete_texture(*this);
		width = other.width;
		height = other.height;
		bpp = other.bpp;
		pixels = new byte[area()];
		memcpy(pixels, other.pixels, area());
		if (other.tid != 0)
			gen_texture();
	}
	return *this;
}

Image& Image::operator=(Image&& other) noexcept
{
	if (this != &other)
	{
		delete_buffer(*this);
		delete_texture(*this);
		width = other.width;
		height = other.height;
		bpp = other.bpp;
		pixels = other.pixels;
		tid = other.tid;
		other.pixels = nullptr;
		other.tid = 0;
	}
	return *this;
}

Image::~Image()
{
	delete_buffer(*this);
	delete_texture(*this);
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

void Image::update_texture_params(const TextureParams& texture_params) const
{
	if (tid != 0)
	{
		bind_texture(tid);
		bind_texture_params(texture_params);
	}
}

void Image::update_texture() const
{
	bind_texture(tid);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, bpp_format(), GL_UNSIGNED_BYTE, pixels);
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

void Image::flip_vertically() const
{
	dim stride = width * bpp;
	byte* temp = new byte[stride];
	byte* bottom = pixels;
	byte* top = pixels + (height - 1) * stride;
	for (dim _ = 0; _ < height >> 1; ++_)
	{
		memcpy(temp, bottom, stride);
		memcpy(bottom, top, stride);
		memcpy(top, temp, stride);
		bottom += stride;
		top -= stride;
	}
	delete[] temp;
}

void Image::flip_horizontally() const
{
	dim stride = width * bpp;
	byte* temp = new byte[bpp];
	byte* row = pixels;
	byte* left = nullptr;
	byte* right = nullptr;
	for (dim _ = 0; _ < height; ++_)
	{
		left = row;
		right = row + (width - 1) * bpp;
		for (dim i = 0; i < width >> 1; ++i)
		{
			memcpy(temp, left, bpp);
			memcpy(left, right, bpp);
			memcpy(right, temp, bpp);
			left += bpp;
			right -= bpp;
		}
		row += stride;
	}
	delete[] temp;
}

void Image::iterate_path(const Path& path, const std::function<void(byte*)>& func)
{
	PathIterator pit = path.first(*this);
	const PathIterator plast = path.last(*this);
	while (true)
	{
		byte* pixel = *pit;
		func(pixel);
		if (pit == plast)
			break;
		else
			path.next(pit);
	}
}

void Image::riterate_path(const Path& path, const std::function<void(byte*)>& func)
{
	PathIterator pit = path.last(*this);
	const PathIterator pfirst = path.first(*this);
	while (true)
	{
		byte* pixel = *pit;
		func(pixel);
		if (pit == pfirst)
			break;
		else
			path.prev(pit);
	}
}

void Image::set(byte* pixel, const Path& path)
{
	iterate_path(path, [pixel, this](byte* pos) { memcpy(pos, pixel, bpp); });
}
