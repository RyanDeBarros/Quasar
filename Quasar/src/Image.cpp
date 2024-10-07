#include "Image.h"

#include <stb/stb_image.h>
#include <memory>

#include "GLutility.h"

inline static GLenum bpp_format(Image::Dim bpp)
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

inline static GLint bpp_alignment(Image::BPP bpp)
{
	if (bpp == 4)
		return 4;
	else if (bpp == 3)
		return 1;
	else if (bpp == 2)
		return 2;
	else if (bpp == 1)
		return 1;
	else
		return 0;
}

inline static GLint bpp_internal_format(Image::BPP bpp)
{
	if (bpp == 4)
		return GL_RGBA8;
	else if (bpp == 3)
		return GL_RGB8;
	else if (bpp == 2)
		return GL_RG8;
	else if (bpp == 1)
		return GL_R8;
	else
		return 0;
}

inline static void delete_buffer(Image& image)
{
	stbi_image_free(image.pixels); // equivalent to delete[] image.pixels
}

inline static void delete_texture(Image& image)
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
	pixels = new Byte[area()];
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
		pixels = new Byte[area()];
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
		resend_texture();
		update_texture_params();
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
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, bpp_format(bpp), GL_UNSIGNED_BYTE, pixels);
	}
}

void Image::resend_texture() const
{
	if (tid)
	{
		bind_texture(tid);
		QUASAR_GL(glPixelStorei(GL_UNPACK_ALIGNMENT, bpp_alignment(bpp)));
		QUASAR_GL(glTexImage2D(GL_TEXTURE_2D, 0, bpp_internal_format(bpp), width, height, 0, bpp_format(bpp), GL_UNSIGNED_BYTE, pixels));
	}
}

void Image::send_subtexture(GLint x, GLint y, GLsizei w, GLsizei h) const
{
	bind_texture(tid);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, bpp_format(bpp), GL_UNSIGNED_BYTE, pixels);
}

void Image::_flip_vertically() const
{
	Dim stride = width * bpp;
	Byte* temp = new Byte[stride];
	Byte* bottom = pixels;
	Byte* top = pixels + (height - 1) * stride;
	for (Dim _ = 0; _ < height >> 1; ++_)
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
	Dim stride = width * bpp;
	Byte* temp = new Byte[bpp];
	Byte* row = pixels;
	Byte* left = nullptr;
	Byte* right = nullptr;
	for (Dim _ = 0; _ < height; ++_)
	{
		left = row;
		right = row + (width - 1) * bpp;
		for (Dim i = 0; i < width >> 1; ++i)
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

void Image::_rotate_180() const
{
	UprightRect full;
	full.x1 = width - 1;
	full.y1 = height / 2 - 1;
	Dim* temp = new Dim[bpp];
	iterate_path(full, [this, temp](PathIterator& pit) {
		Byte* p1 = pixels + bufoffset(pit.x, pit.y, width, bpp);
		Byte* p2 = pixels + bufoffset(width - 1 - pit.x, height - 1 - pit.y, width, bpp);
		memcpy(temp, p1, bpp);
		memcpy(p1, p2, bpp);
		memcpy(p2, temp, bpp);
		});
	delete[] temp;
}

void Image::iterate_path(const Path& path, const std::function<void(PathIterator&)>& func) const
{
	PathIterator pit = path.first();
	const PathIterator plast = path.last();
	while (true)
	{
		func(pit);
		if (pit == plast)
			break;
		else
			path.next(pit);
	}
}

void Image::riterate_path(const Path& path, const std::function<void(PathIterator&)>& func) const
{
	PathIterator pit = path.last();
	const PathIterator pfirst = path.first();
	while (true)
	{
		func(pit);
		if (pit == pfirst)
			break;
		else
			path.prev(pit);
	}
}

void Image::_set(Byte* pixel, const Path& path) const
{
	iterate_path(path, [this, pixel](PathIterator& pit) { memcpy(pixels + bufoffset(pit.x, pit.y, width, bpp), pixel, bpp); });
}
