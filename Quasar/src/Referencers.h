#pragma once

#include "Macros.h"
#include "GLutility.h"
#include "Image.h"

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
