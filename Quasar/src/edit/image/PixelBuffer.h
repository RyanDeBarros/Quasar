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

	Dim stride() const { return width * chpp; }
	Dim bytes() const { return width * chpp * height; }
	Dim area() const { return width * height; }

	void pxnew() { pixels = new Byte[bytes()]; }
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

extern void iterate_path(const Path& path, const std::function<void(PathIterator&)>& func);
extern void reverse_iterate_path(const Path& path, const std::function<void(PathIterator&)>& func);
