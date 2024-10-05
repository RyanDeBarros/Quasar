#include "Sprite.h"

#include "Renderer.h"

static void increment_image_references(ImageReferencer* img)
{
	if (img && img->ref)
		++(img->ref);
}

static void increment_buffer_references(BufferReferencer* buf)
{
	if (buf && buf->ref)
		++(buf->ref);
}

static void increment_texture_references(TextureReferencer* tex)
{
	if (tex && tex->ref)
		++(tex->ref);
}

Sprite::Sprite(const Sprite& other)
	: img(other.img), buf(other.buf), tex(other.tex)
{
	increment_image_references(img);
	increment_buffer_references(buf);
	increment_texture_references(tex);
}

Sprite::Sprite(Sprite&& other) noexcept
	: img(other.img), buf(other.buf), tex(other.tex)
{
	other.img = nullptr;
	other.buf = nullptr;
	other.tex = nullptr;
}

static void decrement_image_references(ImageReferencer* img)
{
	if (img && img->ref)
	{
		--(img->ref);
		if (img->ref == 0)
		{
			if (img->own)
				delete img->image;
			delete img;
		}
	}
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

static void decrement_texture_references(TextureReferencer* tex)
{
	if (tex && tex->ref)
	{
		--(tex->ref);
		if (tex->ref == 0)
		{
			if (tex->own)
			{
				QUASAR_GL(glDeleteTextures(1, &tex->texture));
			}
			delete tex;
		}
	}
}

Sprite& Sprite::operator=(const Sprite& other)
{
	if (this != &other)
	{
		set_img_ref(other.img);
		set_buf_ref(other.buf);
		set_tex_ref(other.tex);
	}
	return *this;
}

Sprite& Sprite::operator=(Sprite&& other) noexcept
{
	if (this != &other)
	{
		if (img != other.img)
		{
			decrement_image_references(img);
			img = other.img;
		}
		if (buf != other.buf)
		{
			decrement_buffer_references(buf);
			buf = other.buf;
		}
		if (tex != other.tex)
		{
			decrement_texture_references(tex);
			tex = other.tex;
		}
		other.img = nullptr;
		other.buf = nullptr;
		other.tex = nullptr;
	}
	return *this;
}

Sprite::~Sprite()
{
	decrement_image_references(img);
	decrement_buffer_references(buf);
	decrement_texture_references(tex);
}

void Sprite::set_img_ref(ImageReferencer* new_img)
{
	if (img != new_img)
	{
		decrement_image_references(img);
		img = new_img;
		increment_image_references(img);
	}
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

void Sprite::set_tex_ref(TextureReferencer* new_tex)
{
	if (tex != new_tex)
	{
		decrement_texture_references(tex);
		tex = new_tex;
		increment_texture_references(tex);
	}
}

void Sprite::on_draw(Renderer* renderer) const
{
	tex->bind(renderer->get_texture_slot(tex->texture));
	renderer->pool_over_buffer(*buf);
}

Sprite rect_sprite(Image* image, bool own_image, const TextureParams& texture_params, TextureReferencer* heap_texture)
{
	Sprite sprite;
	ImageReferencer* img_ref = new ImageReferencer{};
	img_ref->image = image;
	img_ref->own = own_image;
	sprite.img = img_ref;

	if (heap_texture)
		sprite.tex = heap_texture;
	else
	{
		sprite.tex = new TextureReferencer{ true };
		// TODO abstract to another function
		QUASAR_GL(glGenTextures(1, &sprite.tex->texture));
		QUASAR_GL(glBindTexture(GL_TEXTURE_2D, sprite.tex->texture));
		GLint internal_format;
		GLenum format;
		if (sprite.img->image->bpp == 4)
		{
			internal_format = GL_RGBA8;
			format = GL_RGBA;
		}
		else if (sprite.img->image->bpp == 3)
		{
			internal_format = GL_RGB8;
			format = GL_RGB;
		}
		else if (sprite.img->image->bpp == 2)
		{
			internal_format = GL_RG8;
			format = GL_RG;
		}
		else if (sprite.img->image->bpp == 1)
		{
			internal_format = GL_R8;
			format = GL_RED;
		}
		QUASAR_GL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, sprite.img->image->width, sprite.img->image->height,
			0, format, GL_UNSIGNED_BYTE, sprite.img->image->pixels));
		bind_texture_params(texture_params);
	}
	sprite.buf = new BufferReferencer{};
	sprite.buf->stride = Sprite::RECT_STRIDE;
	sprite.buf->vlen_bytes = size_t(4) * sprite.buf->stride * sizeof(GLfloat);
	sprite.buf->varr = new GLfloat[4 * sprite.buf->stride]{
		0.0f, -0.5f * sprite.img->image->width, -0.5f * sprite.img->image->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f,  0.5f * sprite.img->image->width, -0.5f * sprite.img->image->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		2.0f,  0.5f * sprite.img->image->width,  0.5f * sprite.img->image->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		3.0f, -0.5f * sprite.img->image->width,  0.5f * sprite.img->image->height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};
	sprite.buf->ilen_bytes = 6 * sizeof(GLuint);
	sprite.buf->iarr = new GLuint[6]{
		0, 1, 2,
		2, 3, 0
	};
	return sprite;
}
