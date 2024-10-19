#pragma once

#include "variety/Registry.h"
#include "edit/Image.h"
#include "Shader.h"

typedef Registry<Image, unsigned short, ImageConstructor> ImageRegistry;
typedef ImageRegistry::Handle ImageHandle;

inline ImageRegistry Images;
