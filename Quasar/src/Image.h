#pragma once

enum class ImageDeletionPolicy : unsigned char
{
	FROM_STBI,
	FROM_NEW,
	FROM_EXTERNAL
};

struct Image
{
	unsigned char* pixels = nullptr;
	int width = 0, height = 0, bpp = 0;
	ImageDeletionPolicy deletion_policy = ImageDeletionPolicy::FROM_EXTERNAL;

	Image() = default;
	Image(const Image&);
	Image(Image&&) noexcept;
	Image& operator=(const Image&);
	Image& operator=(Image&&) noexcept;
	~Image();

	int stride() const { return width * bpp; }
};
