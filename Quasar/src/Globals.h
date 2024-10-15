#pragma once

#include "variety/Registry.h"
#include "edit/Image.h"
#include "Shader.h"
#include "variety/History.h"

typedef Registry<Image, unsigned short, ImageConstructor>::Handle ImageHandle;
inline Registry<Image, unsigned short, ImageConstructor> ImageRegistry;
typedef Registry<Shader, unsigned short, ShaderConstructor>::Handle ShaderHandle;
inline Registry<Shader, unsigned short, ShaderConstructor> ShaderRegistry;

inline ActionHistory GlobalActionHistory;
