#include "Image.h"

#include <stb/stb_image.h>
#include <memory>

#include "variety/GLutility.h"
#include "PixelBufferPaths.h"

inline static GLenum chpp_format(CHPP chpp)
{
	if (chpp == 4)
		return GL_RGBA;
	else if (chpp == 3)
		return GL_RGB;
	else if (chpp == 2)
		return GL_RG;
	else if (chpp == 1)
		return GL_RED;
	else
		return 0;
}

inline static GLint chpp_alignment(CHPP chpp)
{
	if (chpp == 4)
		return 4;
	else if (chpp == 3)
		return 1;
	else if (chpp == 2)
		return 2;
	else if (chpp == 1)
		return 1;
	else
		return 0;
}

inline static GLint chpp_internal_format(CHPP chpp)
{
	if (chpp == 4)
		return GL_RGBA8;
	else if (chpp == 3)
		return GL_RGB8;
	else if (chpp == 2)
		return GL_RG8;
	else if (chpp == 1)
		return GL_R8;
	else
		return 0;
}

inline static void delete_buffer(Image& image)
{
	stbi_image_free(image.buf.pixels); // equivalent to delete[] image.pixels
}

inline static void delete_texture(Image& image)
{
	QUASAR_GL(glDeleteTextures(1, &image.tid));
}

Image::Image(const ImageConstructor& args)
{
	stbi_set_flip_vertically_on_load(true);
	// LATER handle gif file from memory
	buf.pixels = stbi_load(args.filepath.c_str(), &buf.width, &buf.height, &buf.chpp, 0);
	if (buf.pixels && args.gen_texture)
		gen_texture();
}

Image::Image(const Image& other)
	: buf(other.buf)
{
	if (other.tid != 0)
		gen_texture();
}

Image::Image(Image&& other) noexcept
	: buf(std::move(other.buf)), tid(other.tid)
{
	other.tid = 0;
}

Image& Image::operator=(const Image& other)
{
	if (this != &other)
	{
		delete_buffer(*this);
		delete_texture(*this);
		buf = other.buf;
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
		buf = std::move(other.buf);
		tid = other.tid;
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
		resend_texture();
		update_texture_params(texture_params);
	}
}

void Image::update_texture_params(const TextureParams& texture_params) const
{
	if (tid)
	{
		bind_texture(tid);
		bind_texture_params(texture_params);
	}
}

void Image::update_texture() const
{
	if (tid)
	{
		bind_texture(tid);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buf.width, buf.height, chpp_format(buf.chpp), GL_UNSIGNED_BYTE, buf.pixels);
	}
}

void Image::resend_texture() const
{
	if (tid)
	{
		bind_texture(tid);
		QUASAR_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, chpp_alignment(buf.chpp)));
		QUASAR_GL(glTexImage2D(GL_TEXTURE_2D, 0, chpp_internal_format(buf.chpp), buf.width, buf.height, 0, chpp_format(buf.chpp), GL_UNSIGNED_BYTE, buf.pixels));
	}
}

void Image::send_subtexture(GLint x, GLint y, GLsizei w, GLsizei h) const
{
	bind_texture(tid);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, chpp_format(buf.chpp), GL_UNSIGNED_BYTE, buf.pixels);
}

void Image::_flip_vertically() const
{
	Dim stride = buf.stride();
	Byte* temp = new Byte[stride];
	Byte* bottom = buf.pixels;
	Byte* top = buf.pixels + (buf.height - 1) * stride;
	for (Dim _ = 0; _ < buf.height >> 1; ++_)
	{
		memcpy(temp, bottom, stride);
		memcpy(bottom, top, stride);
		memcpy(top, temp, stride);
		bottom += stride;
		top -= stride;
	}
	delete[] temp;
}

void Image::_flip_horizontally() const
{
	Dim stride = buf.stride();
	Byte* temp = new Byte[buf.chpp];
	Byte* row = buf.pixels;
	Byte* left = nullptr;
	Byte* right = nullptr;
	for (Dim _ = 0; _ < buf.height; ++_)
	{
		left = row;
		right = row + (buf.width - 1) * buf.chpp;
		for (Dim i = 0; i < buf.width >> 1; ++i)
		{
			memcpy(temp, left, buf.chpp);
			memcpy(left, right, buf.chpp);
			memcpy(right, temp, buf.chpp);
			left += buf.chpp;
			right -= buf.chpp;
		}
		row += stride;
	}
	delete[] temp;
}

void Image::_rotate_180() const
{
	UprightRect full;
	full.x1 = buf.width - 1;
	full.y1 = buf.height / 2 - 1;
	Dim* temp = new Dim[buf.chpp];
	iterate_path(full, [this, temp](PathIterator& pit) {
		Byte* p1 = pit.pos(buf);
		Byte* p2 = PathIterator{ buf.width - 1 - pit.x, buf.height - 1 - pit.y }.pos(buf);
		memcpy(temp, p1, buf.chpp);
		memcpy(p1, p2, buf.chpp);
		memcpy(p2, temp, buf.chpp);
		});
	delete[] temp;
}

void Image::_set(Byte* pixel, const Path& path) const
{
	iterate_path(path, [this, pixel](PathIterator& pit) { memcpy(pit.pos(buf), pixel, buf.chpp); });
}
