#pragma once

#include "Macros.h"

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
	MinFilter min_filter = MinFilter::Linear;
	MagFilter mag_filter = MagFilter::Linear;
	TextureWrap wrap_s = TextureWrap::ClampToEdge;
	TextureWrap wrap_t = TextureWrap::ClampToEdge;

	bool operator==(const TextureParams& other) const
	{
		return min_filter == other.min_filter && mag_filter == other.mag_filter && wrap_s == other.wrap_s && wrap_t == other.wrap_t;
	}
};

inline void bind_texture_params(const TextureParams& params)
{
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(params.min_filter)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(params.mag_filter)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(params.wrap_s)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(params.wrap_t)));
}

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

struct ImageReferencer
{
	bool own = false;
	unsigned short ref = 1;
	Image* image;
};

struct BufferReferencer
{
	bool own = false;
	unsigned char ilen_bytes = 0;
	unsigned short stride;
	unsigned short vlen_bytes = 0;
	unsigned short ref = 1;
	GLfloat* varr = nullptr;
	GLuint* iarr = nullptr;
};

struct TextureReferencer
{
	bool own = false;
	unsigned short ref = 0;
	GLuint texture = 0;
};

struct Sprite
{
	ImageReferencer* img = nullptr;
	BufferReferencer* buf = nullptr;
	TextureReferencer* tex = nullptr;
	
	Sprite() = default;
	Sprite(const Sprite&);
	Sprite(Sprite&&) noexcept;
	Sprite& operator=(const Sprite&);
	Sprite& operator=(Sprite&&) noexcept;
	~Sprite();

	void set_img_ref(ImageReferencer* img);
	void set_buf_ref(BufferReferencer* buf);
	void set_tex_ref(TextureReferencer* tex);
};

extern Sprite rect_sprite(Image* image, bool own_image, const TextureParams& texture_params = {}, TextureReferencer* heap_texture = nullptr);
