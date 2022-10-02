#pragma once

#include <GL/glew.h>
#include "GPUTypes.h"


namespace GpuUtils {

    GLenum getGpuWrap(Wrap in);
    GLenum getGpuShaderType(ShaderType in);
}