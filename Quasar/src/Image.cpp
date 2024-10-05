#include "Image.h"

#include <stb/stb_image.h>
#include <memory>

Image::Image(const char* filepath)
{
	stbi_set_flip_vertically_on_load(true);
	pixels = stbi_load(filepath, &width, &height, &bpp, 0);
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
