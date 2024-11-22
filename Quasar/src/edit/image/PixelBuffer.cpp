#include "PixelBuffer.h"

#include <memory>

#include "PixelBufferPaths.h"

void Buffer::flip_horizontally() const
{
	Dim strd = stride();
	Byte* temp = new Byte[chpp];
	Byte* row = pixels;
	Byte* left = nullptr;
	Byte* right = nullptr;
	for (Dim _ = 0; _ < height; ++_)
	{
		left = row;
		right = row + (width - 1) * chpp;
		for (Dim i = 0; i < width >> 1; ++i)
		{
			memcpy(temp, left, chpp);
			memcpy(left, right, chpp);
			memcpy(right, temp, chpp);
			left += chpp;
			right -= chpp;
		}
		row += strd;
	}
	delete[] temp;
}

void Buffer::flip_vertically() const
{
	Dim strd = stride();
	Byte* temp = new Byte[strd];
	Byte* bottom = pixels;
	Byte* top = pixels + (height - 1) * strd;
	for (Dim _ = 0; _ < height >> 1; ++_)
	{
		memcpy(temp, bottom, strd);
		memcpy(bottom, top, strd);
		memcpy(top, temp, strd);
		bottom += strd;
		top -= strd;
	}
	delete[] temp;
}

Buffer Buffer::rotate_90_ret_new() const
{
	Buffer new_buffer = *this;
	std::swap(new_buffer.width, new_buffer.height);
	new_buffer.pxnew();

	Dim x0 = 0, x1 = width - 1, y0 = 0, y1 = height - 1;
	Subbuffer ring_subbuffer_old, ring_subbuffer_new;
	ring_subbuffer_old.buf = *this;
	ring_subbuffer_new.buf = new_buffer;

	Ring ring_old(x0, x1, y0, y1);
	Ring ring_new(y0, y1, x0, x1);
	bool old_valid = ring_old.valid();
	bool new_valid = ring_new.valid();
	while (old_valid && new_valid)
	{
		ring_subbuffer_old.path = &ring_old;
		ring_subbuffer_new.path = &ring_new;
		auto dest_offset = ring_new.width() - 1;
		auto length = ring_new.length();
		if (ring_new.height() == 1)
		{
			std::swap(ring_new.bottom.x0, ring_new.bottom.x1); // this is a hack. do not do this under normal circumstances.
			dest_offset = 0;
		}
		subbuffer_copy(ring_subbuffer_new, ring_subbuffer_old, dest_offset, 0, length);
		old_valid = ring_old.to_inner();
		new_valid = ring_new.to_inner();
	}
	return new_buffer;
}

void Buffer::rotate_180() const
{
	UprightRect full;
	full.x1 = width - 1;
	full.y1 = height / 2 - 1;
	Byte* temp = new Byte[chpp];
	iterate_path(full, [this, temp](PathIterator& pit) {
		Byte* p1 = pit.pos(*this);
		Byte* p2 = PathIterator{ width - 1 - pit.x, height - 1 - pit.y }.pos(*this);
		memcpy(temp, p1, chpp);
		memcpy(p1, p2, chpp);
		memcpy(p2, temp, chpp);
		});
	if (height % 2 == 1)
	{
		Byte* p1 = pixels + stride() * (height / 2);
		Byte* p2 = pixels + stride() * (height / 2 + 1) - chpp;
		for (size_t i = 0; i < width / 2; ++i)
		{
			memcpy(temp, p1, chpp);
			memcpy(p1, p2, chpp);
			memcpy(p2, temp, chpp);
			p1 += chpp;
			p2 -= chpp;
		}
	}
	delete[] temp;
}

Buffer Buffer::rotate_270_ret_new() const
{
	Buffer new_buffer = *this;
	std::swap(new_buffer.width, new_buffer.height);
	new_buffer.pxnew();

	Dim x0 = 0, x1 = width - 1, y0 = 0, y1 = height - 1;
	Subbuffer ring_subbuffer_old, ring_subbuffer_new;
	ring_subbuffer_old.buf = *this;
	ring_subbuffer_new.buf = new_buffer;

	Ring ring_old(x0, x1, y0, y1);
	Ring ring_new(y0, y1, x0, x1);
	bool old_valid = ring_old.valid();
	bool new_valid = ring_new.valid();
	while (old_valid && new_valid)
	{
		ring_subbuffer_old.path = &ring_old;
		ring_subbuffer_new.path = &ring_new;
		auto dest_offset = ring_new.width() - 1;
		auto length = ring_new.length();
		if (ring_new.height() == 1)
		{
			std::swap(ring_new.bottom.x0, ring_new.bottom.x1); // this is a hack. do not do this under normal circumstances.
			dest_offset = 0;
		}
		auto src_offset = ring_old.length() / 2;
		if (ring_old.shape == Ring::VERTI)
		{
			std::swap(ring_old.right.y0, ring_old.right.y1); // this is a hack. do not do this under normal circumstances.
			src_offset = 0;
		}
		else if (ring_old.shape == Ring::HORIZ)
		{
			std::swap(ring_old.bottom.x0, ring_old.bottom.x1); // this is a hack. do not do this under normal circumstances.
			src_offset = 0;
		}
		subbuffer_copy(ring_subbuffer_new, ring_subbuffer_old, dest_offset, src_offset, length);
		old_valid = ring_old.to_inner();
		new_valid = ring_new.to_inner();
	}
	return new_buffer;
}

void Path::move_iter(PathIterator& pit, long long offset) const
{
	if (offset > 0)
	{
		while (offset-- > 0)
			next(pit);
	}
	else if (offset < 0)
	{
		while (offset++ < 0)
			prev(pit);
	}
}

// TODO make sure that a buffer being copied doesn't surpass the destination's bounds

void subbuffer_copy(const Subbuffer& dest, const Subbuffer& src, long long dest_offset, long long src_offset, size_t length)
{
	assert_same_chpp(dest.buf, src.buf);
	PathIterator dest_pit = dest.path->first_iter();
	dest.path->move_iter(dest_pit, dest_offset);
	PathIterator src_pit = src.path->first_iter();
	src.path->move_iter(src_pit, src_offset);
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
	src.path->move_iter(src_pit, src_offset);
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
	dest.path->move_iter(dest_pit, dest_offset);
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

void subbuffer_copy_unbalanced(const Buffer& dest, const Buffer& src, long long dest_offset, long long src_offset, size_t length)
{
	if (dest.chpp == src.chpp)
		subbuffer_copy(dest, src, dest_offset, src_offset, length);
	else
	{
		CHPP chpp = std::min(dest.chpp, src.chpp);
		if (length == -1)
			length = src.area();
		Byte* dest_pixels = dest.pixels + dest_offset * dest.chpp;
		Byte* src_pixels = src.pixels + src_offset * src.chpp;
		for (size_t i = 0; i < length; ++i)
		{
			memcpy(dest_pixels, src_pixels, chpp);
			dest_pixels += dest.chpp;
			src_pixels += src.chpp;
		}
	}
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
