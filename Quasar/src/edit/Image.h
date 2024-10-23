#pragma once

#include <string>
#include <functional>

#include "Macros.h"
#include "variety/FileSystem.h"

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

struct ImageConstructor
{
	FilePath filepath;
	bool gen_texture = true;

	bool operator==(const ImageConstructor&) const = default;
};

template<>
struct std::hash<ImageConstructor>
{
	size_t operator()(const ImageConstructor& ic) const { return std::hash<FilePath>{}(ic.filepath); }
};

struct PathIterator;
struct Path;

struct Image
{
	typedef unsigned char Byte;
	typedef int Dim;
	typedef int CHPP;

	Byte* pixels = nullptr;
	GLuint tid = 0;
	Dim width = 0, height = 0;
	CHPP chpp = 0;

	Image() = default;
	Image(const ImageConstructor& args);
	Image(const Image&);
	Image(Image&&) noexcept;
	Image& operator=(const Image&);
	Image& operator=(Image&&) noexcept;
	~Image();

	operator bool() const { return pixels != nullptr; }

	// texture operations

	void gen_texture(const TextureParams& texture_params = {});
	void update_texture_params(const TextureParams& texture_params = {}) const;
	void update_texture() const;
	void resend_texture() const;
	void send_subtexture(GLint x, GLint y, GLsizei w, GLsizei h) const;

	// buffer operations

	static Dim bufoffset(Dim x, Dim y, Dim width, Dim chpp) { return (y * width + x) * chpp; }
	Dim stride() const { return width * chpp; }
	Dim area() const { return width * height * chpp; }

	void _flip_vertically() const;
	void _flip_horizontally() const;
	void flip_vertically() const { _flip_vertically(); update_texture(); }
	void flip_horizontally() const { _flip_horizontally(); update_texture(); }

	//void _rotate_90();
	void _rotate_180() const;
	void rotate_180() const { _rotate_180(); update_texture(); }
	//void _rotate_270();
	// change bpp
	// crop width / height --> create new image and return it, so old image isn't lost?
	// This reminds me, implement a undo/redo action history stack.

	void iterate_path(const Path& path_, const std::function<void(PathIterator&)>& func) const;
	void riterate_path(const Path& path_, const std::function<void(PathIterator&)>& func) const;
	void _set(Byte* pixel, const Path& path_) const;
	void set(Byte* pixel, const Path& path_) const { _set(pixel, path_); update_texture(); }
};

struct PathIterator
{
	Image::Dim x = 0;
	Image::Dim y = 0;

	bool operator==(const PathIterator& other) const = default;
};

struct Path
{
	virtual void first(PathIterator&) const = 0;
	virtual void last(PathIterator&) const = 0;
	virtual void prev(PathIterator&) const = 0;
	virtual void next(PathIterator&) const = 0;

	PathIterator first() const { PathIterator pit; first(pit); return pit; }
	PathIterator last() const { PathIterator pit; last(pit); return pit; }
};

struct HorizontalLine : public Path
{
	Image::Dim x0 = 0, x1 = 0, y = 0;
	void first(PathIterator& pit) const override { pit.x = x0; pit.y = y; }
	void last(PathIterator& pit) const override { pit.x = x1; pit.y = y; }
	void prev(PathIterator& pit) const override { --pit.x; }
	void next(PathIterator& pit) const override { ++pit.x; }
};

struct VerticalLine : public Path
{
	Image::Dim x = 0, y0 = 0, y1 = 0;
	void first(PathIterator& pit) const override { pit.x = x; pit.y = y0; }
	void last(PathIterator& pit) const override { pit.x = x; pit.y = y1; }
	void prev(PathIterator& pit) const override { --pit.y; }
	void next(PathIterator& pit) const override { ++pit.y; }
};

struct UprightRect : public Path
{
	Image::Dim x0 = 0, x1 = 0, y0 = 0, y1 = 0;

	UprightRect() = default;
	UprightRect(const Image& im) : x0(0), x1(im.width - 1), y0(0), y1(im.height - 1) {}

	void first(PathIterator& pit) const override { pit.x = x0; pit.y = y0; }
	void last(PathIterator& pit) const override { pit.x = x1; pit.y = y1; }
	void prev(PathIterator& pit) const override { if (pit.x == x0) { pit.x = x1; --pit.y; } else { --pit.x; } }
	void next(PathIterator& pit) const override { if (pit.x == x1) { pit.x = x0; ++pit.y; } else { ++pit.x; } }
};
