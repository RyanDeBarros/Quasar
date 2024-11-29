#include "Image.h"

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <memory>

#include "variety/GLutility.h"
#include "variety/Geometry.h"
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

Image::Image(const FilePath& filepath, bool _gen_texture)
{
	// LATER handle gif file from memory
	buf.pixels = stbi_load(filepath.c_str(), &buf.width, &buf.height, &buf.chpp, 0);
	if (buf.pixels && _gen_texture)
		gen_texture();
}

Image::Image(const Image& other)
	: buf(other.buf)
{
	buf.pxnew();
	subbuffer_copy(buf, other.buf);
	if (other.tid != 0)
		gen_texture();
}

Image::Image(Image&& other) noexcept
	: buf(other.buf), tid(other.tid)
{
	other.buf.pixels = nullptr;
	other.buf.width = 0;
	other.buf.height = 0;
	other.buf.chpp = 0;
	other.tid = 0;
}

Image& Image::operator=(const Image& other)
{
	if (this != &other)
	{
		delete_buffer(*this);
		delete_texture(*this);
		buf = other.buf;
		buf.pxnew();
		subbuffer_copy(buf, other.buf);
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
		buf = other.buf;
		other.buf.pixels = nullptr;
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
		QUASAR_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buf.width, buf.height, chpp_format(buf.chpp), GL_UNSIGNED_BYTE, buf.pixels));
	}
}

void Image::update_subtexture(IntRect rect) const
{
	update_subtexture(rect.x, rect.y, rect.w, rect.h);
}

void Image::update_subtexture(int x, int y, int w, int h) const
{
	if (tid && on_interval(x, 0, buf.width - 1) && on_interval(y, 0, buf.height - 1) && w >= 0 && h >= 0)
	{
		if (x + w >= buf.width)
			w = buf.width - x;
		if (y + h >= buf.height)
			h = buf.height - y;
		bind_texture(tid);
		for (Dim r = y; r < y + h; ++r)
		{
			QUASAR_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, x, r, w, 1, chpp_format(buf.chpp), GL_UNSIGNED_BYTE, buf.pos(x, r)));
		}
	}
}

void Image::resend_texture()
{
	if (!tid)
	{
		QUASAR_GL(glGenTextures(1, &tid));
	}
	bind_texture(tid);
	QUASAR_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, chpp_alignment(buf.chpp)));
	QUASAR_GL(glTexImage2D(GL_TEXTURE_2D, 0, chpp_internal_format(buf.chpp), buf.width, buf.height, 0, chpp_format(buf.chpp), GL_UNSIGNED_BYTE, buf.pixels));
}

bool Image::write_to_file(const FilePath& filepath, ImageFormat format, JPGQuality jpg_quality) const
{
	switch (format)
	{
	case ImageFormat::PNG:
		return stbi_write_png(filepath.c_str(), buf.width, buf.height, buf.chpp, buf.pixels, buf.stride() * sizeof(Byte));
	case ImageFormat::JPG:
		return stbi_write_jpg(filepath.c_str(), buf.width, buf.height, buf.chpp, buf.pixels, int(jpg_quality));
	case ImageFormat::BMP:
		return stbi_write_bmp(filepath.c_str(), buf.width, buf.height, buf.chpp, buf.pixels);
	case ImageFormat::HDR:
	{
#pragma warning(push)
#pragma warning(disable : 6386)
		float* hdr_buf = new float[buf.bytes()];
		for (size_t i = 0; i < buf.bytes(); ++i)
			hdr_buf[i] = buf.pixels[i] / 255.0f;
#pragma warning(pop)
		bool success = stbi_write_hdr(filepath.c_str(), buf.width, buf.height, buf.chpp, hdr_buf); // NOTE HDR cannot be internally stored in Image.
		delete[] hdr_buf;
		return success;
	}
	case ImageFormat::TGA:
		return stbi_write_tga(filepath.c_str(), buf.width, buf.height, buf.chpp, buf.pixels);
	}
	return false;
}

void Image::flip_horizontally() const
{
	buf.flip_horizontally();
	update_texture();
}

void Image::flip_vertically() const
{
	buf.flip_vertically();
	update_texture();
}

void Image::rotate_90_del_old()
{
	Buffer new_buffer = buf.rotate_90_ret_new();
	delete_buffer(*this);
	buf = new_buffer;
	resend_texture();
}

void Image::rotate_180() const
{
	buf.rotate_180();
	update_texture();
}

void Image::rotate_270_del_old()
{
	Buffer new_buf = buf.rotate_270_ret_new();
	delete_buffer(*this);
	buf = new_buf;
	resend_texture();
}

void Image::_set(Byte* pixel, const Path& path) const
{
	iterate_path(path, [this, pixel](PathIterator& pit) { memcpy(pit.pos(buf), pixel, buf.chpp); });
}
