#include "PixelBuffer.h"
#include "PixelBufferPaths.h"

#include <memory>

void subbuffer_copy(const Subbuffer& dest, const Subbuffer& src, size_t dest_offset, size_t src_offset, size_t length)
{
	assert_same_chpp(dest.buf, src.buf);
	PathIterator dest_pit = dest.path->first_iter();
	while (dest_offset-- > 0)
		dest.path->next(dest_pit);
	PathIterator src_pit = src.path->first_iter();
	while (src_offset-- > 0)
		src.path->next(src_pit);
	if (length == -1)
	{
		auto dest_last = dest.path->last_iter();
		auto src_last = src.path->last_iter();
		while (true)
		{
			memcpy(dest_pit.pos(dest.buf), src_pit.pos(src.buf), dest.buf.chpp * sizeof(Byte));
			if (dest_pit == dest_last || src_pit == src_last)
				break;
			dest.path->next(dest_pit);
			src.path->next(src_pit);
		}
	}
	else
	{
		while (length-- > 0)
		{
			memcpy(dest_pit.pos(dest.buf), src_pit.pos(src.buf), dest.buf.chpp * sizeof(Byte));
			dest.path->next(dest_pit);
			src.path->next(src_pit);
		}
	}
}

void subbuffer_copy(const Buffer& dest, const Subbuffer& src, size_t dest_offset, size_t src_offset, size_t length)
{
	assert_same_chpp(dest, src.buf);
	Byte* dest_pixels = dest.pixels + dest_offset;
	PathIterator src_pit = src.path->first_iter();
	while (src_offset-- > 0)
		src.path->next(src_pit);
	if (length == -1)
	{
		auto src_last = src.path->last_iter();
		while (true)
		{
			memcpy(dest_pixels, src_pit.pos(src.buf), dest.chpp * sizeof(Byte));
			if (src_pit == src_last)
				break;
			dest_pixels += dest.chpp;
			src.path->next(src_pit);
		}
	}
	else
	{
		while (length-- > 0)
		{
			memcpy(dest_pixels, src_pit.pos(src.buf), dest.chpp * sizeof(Byte));
			dest_pixels += dest.chpp;
			src.path->next(src_pit);
		}
	}
}

void subbuffer_copy(const Subbuffer& dest, const Buffer& src, size_t dest_offset, size_t src_offset, size_t length)
{
	assert_same_chpp(dest.buf, src);
	PathIterator dest_pit = dest.path->first_iter();
	while (dest_offset-- > 0)
		dest.path->next(dest_pit);
	Byte* src_pixels = src.pixels + src_offset;
	if (length == -1)
	{
		auto dest_last = dest.path->last_iter();
		while (true)
		{
			memcpy(dest_pit.pos(dest.buf), src_pixels, src.chpp * sizeof(Byte));
			if (dest_pit == dest_last)
				break;
			dest.path->next(dest_pit);
			src_pixels += src.chpp;
		}
	}
	else
	{
		while (length-- > 0)
		{
			memcpy(dest_pit.pos(dest.buf), src_pixels, src.chpp * sizeof(Byte));
			dest.path->next(dest_pit);
			src_pixels += src.chpp;
		}
	}
}

void subbuffer_copy(const Buffer& dest, const Buffer& src, size_t dest_offset, size_t src_offset, size_t length)
{
	assert_same_chpp(dest, src);
	memcpy(dest.pixels + dest_offset, src.pixels + src_offset, length * dest.chpp * sizeof(Byte));
}

void iterate_path(const Path& path, const std::function<void(PathIterator&)>& func)
{
	PathIterator pit = path.first_iter();
	const PathIterator plast = path.last_iter();
	while (true)
	{
		func(pit);
		if (pit == plast)
			break;
		else
			path.next(pit);
	}
}

void reverse_iterate_path(const Path& path, const std::function<void(PathIterator&)>& func)
{
	PathIterator pit = path.last_iter();
	const PathIterator pfirst = path.first_iter();
	while (true)
	{
		func(pit);
		if (pit == pfirst)
			break;
		else
			path.prev(pit);
	}
}
