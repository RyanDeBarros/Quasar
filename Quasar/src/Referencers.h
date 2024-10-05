#pragma once

#include "Macros.h"
#include "GLutility.h"
#include "Image.h"

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

	void bind(GLuint slot) const { bind_texture(texture, slot); }
};
