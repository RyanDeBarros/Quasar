#pragma once

#include "variety/Registry.h"
#include "edit/Image.h"
#include "Shader.h"

typedef Registry<Image, unsigned short, ImageConstructor> ImageRegistry;
typedef ImageRegistry::Handle ImageHandle;
typedef Registry<Shader, unsigned short, ShaderConstructor> ShaderRegistry;
typedef ShaderRegistry::Handle ShaderHandle;

inline ImageRegistry Images;
inline ShaderRegistry Shaders;
