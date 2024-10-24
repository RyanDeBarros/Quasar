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
	buf.pixels = new Byte[buf.bytes()];
	subbuffer_copy(buf, other.buf);
	if (other.tid != 0)
		gen_texture();
}

Image::Image(Image&& other) noexcept
	: buf(other.buf), tid(other.tid)
{
	other.buf.pixels = nullptr;
	other.tid = 0;
}

Image& Image::operator=(const Image& other)
{
	if (this != &other)
	{
		delete_buffer(*this);
		delete_texture(*this);
		buf = other.buf;
		buf.pixels = new Byte[buf.bytes()];
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
	QUASAR_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, chpp_format(buf.chpp), GL_UNSIGNED_BYTE, buf.pixels));
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

void Image::_rotate_90()
{
	if (buf.width == 0 || buf.height == 0)
		return;
	Buffer new_buffer;
	new_buffer.width = buf.height;
	new_buffer.height = buf.width;
	new_buffer.chpp = buf.chpp;
	new_buffer.pixels = new Byte[new_buffer.bytes()]; // TODO new_buffer.pxnew()

	Dim x0 = 0, x1 = buf.width - 1, y0 = 0, y1 = buf.height - 1;
	Subbuffer ring_subbuffer_old, ring_subbuffer_new;
	ring_subbuffer_old.buf = buf;
	ring_subbuffer_new.buf = new_buffer;

	Ring ring_old(x0, x1, y0, y1);
	Ring ring_new(y0, y1, x0, x1);
	bool old_valid = ring_old.valid();
	bool new_valid = ring_new.valid();
	while (old_valid && new_valid)
	{
		ring_subbuffer_old.path = &ring_old;
		ring_subbuffer_new.path = &ring_new;
		auto dest_offset = ring_new.width() - 1;
		auto length = ring_new.length();
		if (ring_new.height() == 1)
		{
			std::swap(ring_new.bottom.x0, ring_new.bottom.x1); // this is a hack. do not do this under normal circumstances.
			dest_offset = 0;
		}
		subbuffer_copy(ring_subbuffer_new, ring_subbuffer_old, dest_offset, 0, length);
		old_valid = ring_old.to_inner();
		new_valid = ring_new.to_inner();
	}
	delete_buffer(*this);
	buf = new_buffer;
}

void Image::_rotate_180() const
{
	UprightRect full;
	full.x1 = buf.width - 1;
	full.y1 = buf.height / 2 - 1;
	Byte* temp = new Byte[buf.chpp];
	iterate_path(full, [this, temp](PathIterator& pit) {
		Byte* p1 = pit.pos(buf);
		Byte* p2 = PathIterator{ buf.width - 1 - pit.x, buf.height - 1 - pit.y }.pos(buf);
		memcpy(temp, p1, buf.chpp);
		memcpy(p1, p2, buf.chpp);
		memcpy(p2, temp, buf.chpp);
		});
	if (buf.height % 2 == 1)
	{
		Byte* p1 = buf.pixels + buf.stride() * (buf.height / 2);
		Byte* p2 = buf.pixels + buf.stride() * (buf.height / 2 + 1) - buf.chpp;
		for (size_t i = 0; i < buf.width / 2; ++i)
		{
			memcpy(temp, p1, buf.chpp);
			memcpy(p1, p2, buf.chpp);
			memcpy(p2, temp, buf.chpp);
			p1 += buf.chpp;
			p2 -= buf.chpp;
		}
	}
	delete[] temp;

	// SLOWER, although still accurate:
	//UprightRect lower;
	//lower.x1 = buf.width - 1;
	//lower.y1 = buf.height / 2;
	//Subbuffer lower_sub{ buf, &lower };
	//
	//ReversePath reverse_lower;
	//reverse_lower.forward = &lower;
	//Subbuffer reverse_lower_sub{ buf, &reverse_lower };
	//
	//Buffer temp;
	//temp.width = buf.width;
	//temp.chpp = buf.chpp;
	//temp.height = buf.height / 2 + 1;
	//temp.pixels = new Byte[temp.bytes()];
	//
	//UprightRect upper;
	//upper.x1 = buf.width - 1;
	//upper.y0 = buf.height / 2 + 1;
	//upper.y1 = buf.height - 1;
	//Subbuffer upper_sub{ buf, &upper };
	//
	//ReversePath reverse_upper;
	//reverse_upper.forward = &upper;
	//Subbuffer reverse_upper_sub{ buf, &reverse_upper };
	//
	//subbuffer_copy(temp, reverse_lower_sub);
	//subbuffer_copy(buf, reverse_upper_sub);
	//subbuffer_copy(buf, temp, buf.width * (buf.height / 2 - 1));
	//
	//delete[] temp.pixels;
}

void Image::_rotate_270()
{
	if (buf.width == 0 || buf.height == 0)
		return;
	Buffer new_buffer;
	new_buffer.width = buf.height;
	new_buffer.height = buf.width;
	new_buffer.chpp = buf.chpp;
	new_buffer.pixels = new Byte[new_buffer.bytes()]; // TODO new_buffer.pxnew()

	Dim x0 = 0, x1 = buf.width - 1, y0 = 0, y1 = buf.height - 1;
	Subbuffer ring_subbuffer_old, ring_subbuffer_new;
	ring_subbuffer_old.buf = buf;
	ring_subbuffer_new.buf = new_buffer;
	
	Ring ring_old(x0, x1, y0, y1);
	Ring ring_new(y0, y1, x0, x1);
	bool old_valid = ring_old.valid();
	bool new_valid = ring_new.valid();
	while (old_valid && new_valid)
	{
		ring_subbuffer_old.path = &ring_old;
		ring_subbuffer_new.path = &ring_new;
		auto dest_offset = ring_new.width() - 1;
		auto length = ring_new.length();
		if (ring_new.height() == 1)
		{
			std::swap(ring_new.bottom.x0, ring_new.bottom.x1); // this is a hack. do not do this under normal circumstances.
			dest_offset = 0;
		}
		auto src_offset = ring_old.length() / 2;
		if (ring_old.shape == Ring::VERTI)
		{
			std::swap(ring_old.right.y0, ring_old.right.y1); // this is a hack. do not do this under normal circumstances.
			src_offset = 0;
		}
		else if (ring_old.shape == Ring::HORIZ)
		{
			std::swap(ring_old.bottom.x0, ring_old.bottom.x1); // this is a hack. do not do this under normal circumstances.
			src_offset = 0;
		}
		subbuffer_copy(ring_subbuffer_new, ring_subbuffer_old, dest_offset, src_offset, length);
		old_valid = ring_old.to_inner();
		new_valid = ring_new.to_inner();
	}
	delete_buffer(*this);
	buf = new_buffer;
}

void Image::_set(Byte* pixel, const Path& path) const
{
	iterate_path(path, [this, pixel](PathIterator& pit) { memcpy(pit.pos(buf), pixel, buf.chpp); });
}
