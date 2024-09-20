#include <stb/stb_image.h>

#include "Sprite.h"

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

static void increment_image_references(ImageReferencer* img)
{
	if (img && img->ref)
		++(img->ref);
}

static void increment_buffer_references(BufferReferencer* buf)
{
	if (buf && buf->ref)
		++(buf->ref);
}

static void increment_texture_references(TextureReferencer* tex)
{
	if (tex && tex->ref)
		++(tex->ref);
}

Sprite::Sprite(const Sprite& other)
	: img(other.img), buf(other.buf), tex(other.tex)
{
	increment_image_references(img);
	increment_buffer_references(buf);
	increment_texture_references(tex);
}

Sprite::Sprite(Sprite&& other) noexcept
	: img(other.img), buf(other.buf), tex(other.tex)
{
	other.img = nullptr;
	other.buf = nullptr;
	other.tex = nullptr;
}

static void decrement_image_references(ImageReferencer* img)
{
	if (img && img->ref)
	{
		--(img->ref);
		if (!(img->ref))
			delete img;
	}
}

static void decrement_buffer_references(BufferReferencer* buf)
{
	if (buf && buf->ref)
	{
		--(buf->ref);
		if (!(buf->ref))
		{
			delete[] buf->varr;
			delete[] buf->iarr;
			delete buf;
		}
	}
}

static void decrement_texture_references(TextureReferencer* tex)
{
	if (tex && tex->ref)
	{
		--(tex->ref);
		if (!(tex->ref))
		{
			QUASAR_GL(glDeleteTextures(1, &tex->texture));
			delete tex;
		}
	}
}

Sprite& Sprite::operator=(const Sprite& other)
{
	if (this != &other)
	{
		if (img != other.img)
		{
			decrement_image_references(img);
			img = other.img;
			increment_image_references(img);
		}
		if (buf != other.buf)
		{
			decrement_buffer_references(buf);
			buf = other.buf;
			increment_buffer_references(buf);
		}
		if (tex != other.tex)
		{
			decrement_texture_references(tex);
			tex = other.tex;
			increment_texture_references(tex);
		}
	}
	return *this;
}

Sprite& Sprite::operator=(Sprite&& other) noexcept
{
	if (this != &other)
	{
		if (img != other.img)
		{
			decrement_image_references(img);
			img = other.img;
		}
		if (buf != other.buf)
		{
			decrement_buffer_references(buf);
			buf = other.buf;
		}
		if (tex != other.tex)
		{
			decrement_texture_references(tex);
			tex = other.tex;
		}
		other.img = nullptr;
		other.buf = nullptr;
		other.tex = nullptr;
	}
	return *this;
}

Sprite::~Sprite()
{
	decrement_image_references(img);
	decrement_buffer_references(buf);
	decrement_texture_references(tex);
}
