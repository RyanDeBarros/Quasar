#pragma once

#include "Registry.h"
#include "Image.h"
#include "Shader.h"

typedef Registry<Image, unsigned short, ImageConstructor>::Handle ImageHandle;
inline Registry<Image, unsigned short, ImageConstructor> ImageRegistry;
typedef Registry<Shader, unsigned short, ShaderConstructor>::Handle ShaderHandle;
inline Registry<Shader, unsigned short, ShaderConstructor> ShaderRegistry;
