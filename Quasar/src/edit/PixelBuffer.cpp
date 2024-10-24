#include "PixelBuffer.h"

#include <memory>

// TODO put somewhere else
static void advance_pit(PathIterator& pit, const Path& path, long long offset)
{
	if (offset > 0)
	{
		while (offset-- > 0)
			path.next(pit);
	}
	else if (offset < 0)
	{
		while (offset++ < 0)
			path.prev(pit);
	}
}

void subbuffer_copy(const Subbuffer& dest, const Subbuffer& src, long long dest_offset, long long src_offset, size_t length)
{
	assert_same_chpp(dest.buf, src.buf);
	PathIterator dest_pit = dest.path->first_iter();
	advance_pit(dest_pit, *dest.path, dest_offset);
	PathIterator src_pit = src.path->first_iter();
	advance_pit(src_pit, *src.path, src_offset);
	if (length == -1)
	{
		auto dest_last = dest.path->last_iter();
		auto src_last = src.path->last_iter();
		while (true)
		{
			memcpy(dest_pit.pos(dest.buf), src_pit.pos(src.buf), dest.buf.chpp);
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
			memcpy(dest_pit.pos(dest.buf), src_pit.pos(src.buf), dest.buf.chpp);
			dest.path->next(dest_pit);
			src.path->next(src_pit);
		}
	}
}

void subbuffer_copy(const Buffer& dest, const Subbuffer& src, long long dest_offset, long long src_offset, size_t length)
{
	assert_same_chpp(dest, src.buf);
	Byte* dest_pixels = dest.pixels + dest_offset;
	PathIterator src_pit = src.path->first_iter();
	advance_pit(src_pit, *src.path, src_offset);
	if (length == -1)
	{
		auto src_last = src.path->last_iter();
		while (true)
		{
			memcpy(dest_pixels, src_pit.pos(src.buf), dest.chpp);
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
			memcpy(dest_pixels, src_pit.pos(src.buf), dest.chpp);
			dest_pixels += dest.chpp;
			src.path->next(src_pit);
		}
	}
}

void subbuffer_copy(const Subbuffer& dest, const Buffer& src, long long dest_offset, long long src_offset, size_t length)
{
	assert_same_chpp(dest.buf, src);
	PathIterator dest_pit = dest.path->first_iter();
	advance_pit(dest_pit, *dest.path, dest_offset);
	Byte* src_pixels = src.pixels + src_offset;
	if (length == -1)
	{
		auto dest_last = dest.path->last_iter();
		while (true)
		{
			memcpy(dest_pit.pos(dest.buf), src_pixels, src.chpp);
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
			memcpy(dest_pit.pos(dest.buf), src_pixels, src.chpp);
			dest.path->next(dest_pit);
			src_pixels += src.chpp;
		}
	}
}

void subbuffer_copy(const Buffer& dest, const Buffer& src, long long dest_offset, long long src_offset, size_t length)
{
	assert_same_chpp(dest, src);
	if (length == -1)
		memcpy(dest.pixels + dest_offset * dest.chpp, src.pixels + src_offset * src.chpp, src.bytes());
	else
		memcpy(dest.pixels + dest_offset * dest.chpp, src.pixels + src_offset * src.chpp, length * dest.chpp);
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
