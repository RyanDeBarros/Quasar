#pragma once

#include <string>
#include <functional>

#include "Macros.h"
#include "variety/FileSystem.h"
#include "PixelBuffer.h"

enum class MinFilter : GLint
{
	Nearest = GL_NEAREST,
	Linear = GL_LINEAR,
	NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
	LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
	NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
	LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

constexpr MinFilter MinFilterLookup[] = {
	MinFilter::Nearest,
	MinFilter::Linear,
	MinFilter::NearestMipmapNearest,
	MinFilter::LinearMipmapNearest,
	MinFilter::NearestMipmapLinear,
	MinFilter::LinearMipmapLinear
};

constexpr unsigned int MinFilterLookupLength = 6;

enum class MagFilter : GLint
{
	Nearest = GL_NEAREST,
	Linear = GL_LINEAR
};

constexpr MagFilter MagFilterLookup[] = {
	MagFilter::Nearest,
	MagFilter::Linear
};

constexpr unsigned int MagFilterLookupLength = 2;

enum class TextureWrap : GLint
{
	ClampToEdge = GL_CLAMP_TO_EDGE,
	ClampToBorder = GL_CLAMP_TO_BORDER,
	MirroredRepeat = GL_MIRRORED_REPEAT,
	Repeat = GL_REPEAT,
	MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
};

constexpr TextureWrap TextureWrapLookup[] = {
	TextureWrap::ClampToEdge,
	TextureWrap::ClampToBorder,
	TextureWrap::MirroredRepeat,
	TextureWrap::Repeat,
	TextureWrap::MirrorClampToEdge
};

constexpr unsigned int TextureWrapLookupLength = 5;

struct TextureParams
{
	MinFilter min_filter = MinFilter::Nearest;
	MagFilter mag_filter = MagFilter::Nearest;
	TextureWrap wrap_s = TextureWrap::ClampToEdge;
	TextureWrap wrap_t = TextureWrap::ClampToEdge;

	bool operator==(const TextureParams& other) const
	{
		return min_filter == other.min_filter && mag_filter == other.mag_filter && wrap_s == other.wrap_s && wrap_t == other.wrap_t;
	}

	static const TextureParams linear;
};

inline const TextureParams TextureParams::linear = { MinFilter::Linear, MagFilter::Linear };

inline void bind_texture_params(const TextureParams& params)
{
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(params.min_filter)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(params.mag_filter)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(params.wrap_s)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(params.wrap_t)));
}

enum class ImageFormat
{
	PNG,
	JPG,
	BMP,
	HDR,
	TGA
};

enum class JPGQuality
{
	HIGHEST = 100,
	VERY_HIGH = 90,
	HIGH = 75,
	MEDIUM = 50,
	LOW = 25,
	VERY_LOW = 10,
	LOWEST = 1
};

struct Image
{
	Buffer buf;
	GLuint tid = 0;

	Image() = default;
	Image(const FilePath& filepath, bool gen_texture = true);
	Image(const Image&);
	Image(Image&&) noexcept;
	Image& operator=(const Image&);
	Image& operator=(Image&&) noexcept;
	~Image();

	operator bool() const { return buf.pixels != nullptr; }

	// texture operations

	void gen_texture(const TextureParams& texture_params = {});
	void update_texture_params(const TextureParams& texture_params = {}) const;
	void update_texture() const;
	void update_subtexture(struct IntRect rect) const;
	void update_subtexture(int x, int y, int w, int h) const;
	void resend_texture();

	bool write_to_file(const FilePath& filepath, ImageFormat format, JPGQuality jpg_quality = JPGQuality::HIGHEST) const;

	// buffer operations
	void flip_horizontally() const;
	void flip_vertically() const;
	void rotate_90_del_old();
	void rotate_180() const;
	void rotate_270_del_old();

	// LATER change bpp
	// LATER crop width / height --> create new image and return it, so old image isn't lost?

	void _set(Byte* pixel, const Path& path_) const;
	void set(Byte* pixel, const Path& path_) const { _set(pixel, path_); update_texture(); }
};
