#pragma once

#include <GL/glew.h>
#include "GPUTypes.h"


namespace GpuUtils {

    GLenum getGpuWrap(Wrap in);
    GLenum getGpuShaderType(ShaderType in);
    GLenum getGpuBufferType(BufferType in);
    GLenum getGpuUniformFrequency(UniformFrequency in);
    GLenum getGpuTestFunction(TestFunction in);
    GLenum getGpuStencilOp(StencilOp in);
    GLenum getGpuBlendEquation(BlendEquation in);
    GLenum getGpuBlendFunction(BlendFunction in);
    GLenum getGpuFace(Faces in);
    GLenum getGpuPoligonMode(PolygonMode in);
    GLenum getGpuTextureShape(TextureShape in);
    GLenum getGpuFilter(Filter in);
    GLenum getGpuPixelLayout(Layout in);
}