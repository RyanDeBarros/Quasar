#pragma once

#include <stdexcept>
#include <string>
#include <functional>

typedef unsigned char Byte;
typedef int Dim;
typedef int CHPP;

struct Buffer
{
	static_assert(sizeof(Byte) == 1);
	Byte* pixels = nullptr;
	Dim width = 0;
	Dim height = 0;
	CHPP chpp = 0;

	bool operator==(const Buffer&) const = default;

	Dim stride() const { return width * chpp; }
	Dim bytes() const { return width * chpp * height; }
	Dim area() const { return width * height; }

	Dim index_offset(Dim x, Dim y) const { return x + y * width; }
	Dim byte_offset(Dim x, Dim y) const { return index_offset(x, y) * chpp; }
	Byte* pos(Dim x, Dim y) const { return pixels + byte_offset(x, y); }
	void coordinates_of(Dim i, Dim& x, Dim& y) const { x = i % width; y = i / height; }

	void pxnew() { pixels = new Byte[bytes()]; }
	bool same_dimensions_as(const Buffer& buf) const { return width == buf.width && height == buf.height && chpp == buf.chpp; }

	void flip_horizontally() const;
	void flip_vertically() const;
	Buffer rotate_90_ret_new() const;
	void rotate_180() const;
	Buffer rotate_270_ret_new() const;
};

struct CHPPMismatchError : public std::runtime_error
{
	CHPPMismatchError(CHPP a, CHPP b) : std::runtime_error("CHPP \"" + std::to_string(a) + "\" != \"" + std::to_string(b) + "\"") {}
};

struct PathIterator
{
	Dim x = 0, y = 0;
	bool operator==(const PathIterator& other) const = default;

	Dim offset(const Buffer& buf) const { return (x + y * buf.width) * buf.chpp; }
	Byte* pos(const Buffer& buf) const { return buf.pixels + offset(buf); }
};

struct Path
{
	virtual void first(PathIterator&) const = 0;
	virtual void last(PathIterator&) const = 0;
	virtual void prev(PathIterator&) const = 0;
	virtual void next(PathIterator&) const = 0;

	PathIterator first_iter() const { PathIterator pit; first(pit); return pit; }
	PathIterator last_iter() const { PathIterator pit; last(pit); return pit; }
	void move_iter(PathIterator& pit, long long offset) const;
};

struct ReversePath : public Path
{
	Path* forward;

	virtual void first(PathIterator& pit) const { forward->last(pit); }
	virtual void last(PathIterator& pit) const { forward->first(pit); }
	virtual void prev(PathIterator& pit) const { forward->next(pit); }
	virtual void next(PathIterator& pit) const { forward->prev(pit); }
};

struct Subbuffer
{
	Buffer buf;
	Path* path;
};

inline void assert_same_chpp(const Buffer& a, const Buffer& b) { if (a.chpp != b.chpp) throw CHPPMismatchError(a.chpp, b.chpp); }

extern void subbuffer_copy(const Subbuffer& dest, const Subbuffer& src, long long dest_offset = 0, long long src_offset = 0, size_t length = -1);
extern void subbuffer_copy(const Buffer& dest, const Subbuffer& src, long long dest_offset = 0, long long src_offset = 0, size_t length = -1);
extern void subbuffer_copy(const Subbuffer& dest, const Buffer& src, long long dest_offset = 0, long long src_offset = 0, size_t length = -1);
extern void subbuffer_copy(const Buffer& dest, const Buffer& src, long long dest_offset = 0, long long src_offset = 0, size_t length = -1);

extern void subbuffer_copy_unbalanced(const Buffer& dest, const Buffer& src, long long dest_offset = 0, long long src_offset = 0, size_t length = -1);

extern void iterate_path(const Path& path, const std::function<void(PathIterator&)>& func);
extern void reverse_iterate_path(const Path& path, const std::function<void(PathIterator&)>& func);
