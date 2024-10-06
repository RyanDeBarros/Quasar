#include "Sprite.h"

#include "Renderer.h"

static void increment_buffer_references(BufferReferencer* buf)
{
	if (buf && buf->ref)
		++(buf->ref);
}

Sprite::Sprite(ImageHandle image)
	: image(image)
{
	Image* img = ImageRegistry.get(image);
	img->gen_texture();
	buf = new BufferReferencer{};
	buf->stride = Sprite::RECT_STRIDE;
	buf->vlen_bytes = size_t(4) * buf->stride * sizeof(GLfloat);
	buf->varr = new GLfloat[4 * buf->stride]{
		0.0f, -0.5f * img->width, -0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f,  0.5f * img->width, -0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		2.0f,  0.5f * img->width,  0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		3.0f, -0.5f * img->width,  0.5f * img->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};
	buf->ilen_bytes = 6 * sizeof(GLuint);
	buf->iarr = new GLuint[6]{
		0, 1, 2,
		2, 3, 0
	};
}

Sprite::Sprite(const Sprite& other)
	: buf(other.buf), image(other.image), transform(other.transform)
{
	increment_buffer_references(buf);
}

Sprite::Sprite(Sprite&& other) noexcept
	: buf(other.buf), image(other.image), transform(other.transform)
{
	other.buf = nullptr;
}

static void decrement_buffer_references(BufferReferencer* buf)
{
	if (buf && buf->ref)
	{
		--(buf->ref);
		if (buf->ref == 0)
		{
			if (buf->own)
			{
				delete[] buf->varr;
				delete[] buf->iarr;
			}
			delete buf;
		}
	}
}

Sprite& Sprite::operator=(const Sprite& other)
{
	if (this != &other)
	{
		set_buf_ref(other.buf);
		image = other.image;
		transform = other.transform;
	}
	return *this;
}

Sprite& Sprite::operator=(Sprite&& other) noexcept
{
	if (this != &other)
	{
		if (buf != other.buf)
		{
			decrement_buffer_references(buf);
			buf = other.buf;
		}
		image = other.image;
		transform = other.transform;
		other.buf = nullptr;
	}
	return *this;
}

Sprite::~Sprite()
{
	decrement_buffer_references(buf);
}

void Sprite::set_buf_ref(BufferReferencer* new_buf)
{
	if (buf != new_buf)
	{
		decrement_buffer_references(buf);
		buf = new_buf;
		increment_buffer_references(buf);
	}
}

void Sprite::on_draw(Renderer* renderer) const
{
	renderer->prepare_for(*buf);
	Image* img = ImageRegistry.get(image);
	img->bind_texture(renderer->get_texture_slot(img->tid));
	renderer->pool_over_buffer(*buf);
}

void Sprite::sync_transform() const
{
	glm::vec2 pp = transform.packed_p();
	glm::vec4 prs = transform.packed_rs();
	GLfloat* row = buf->varr;
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + 4, &pp[0], 2 * sizeof(GLfloat));
		memcpy(row + 6, &prs[0], 4 * sizeof(GLfloat));
		row += buf->stride;
	}
}

void Sprite::sync_transform_p() const
{
	glm::vec2 pp = transform.packed_p();
	GLfloat* row = buf->varr;
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + 4, &pp[0], 2 * sizeof(GLfloat));
		row += buf->stride;
	}
}

void Sprite::sync_transform_rs() const
{
	glm::vec4 prs = transform.packed_rs();
	GLfloat* row = buf->varr;
	for (size_t i = 0; i < 4; ++i)
	{
		memcpy(row + 6, &prs[0], 4 * sizeof(GLfloat));
		row += buf->stride;
	}
}
