#pragma once

#include "Referencers.h"

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
};

inline void bind_texture_params(const TextureParams& params)
{
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(params.min_filter)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(params.mag_filter)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(params.wrap_s)));
	QUASAR_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(params.wrap_t)));
}

struct Sprite
{
	static constexpr unsigned short RECT_STRIDE = 14;

	// TODO just transition to registries and handles?
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

	void on_draw(class Renderer*) const;
};

extern Sprite rect_sprite(Image* image, bool own_image, const TextureParams& texture_params = {}, TextureReferencer* heap_texture = nullptr);
