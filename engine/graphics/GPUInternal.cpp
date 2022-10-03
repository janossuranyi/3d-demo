#include "GPUInternal.h"

#define CASE_RETURN(c,r) case c: return r

namespace GpuUtils
{
    GLenum getGpuWrap(Wrap in)
    {
        switch(in) {
            CASE_RETURN(Wrap::CLAMP, GL_CLAMP_TO_EDGE);
            CASE_RETURN(Wrap::MIRROR, GL_MIRRORED_REPEAT);
            CASE_RETURN(Wrap::REPEAT, GL_REPEAT);
        }

        return GL_CLAMP_TO_EDGE;
    }

    GLenum getGpuShaderType(ShaderType in)
    {
        switch(in) {
            CASE_RETURN(ShaderType::COMPUTE, GL_COMPUTE_SHADER);
            CASE_RETURN(ShaderType::FRAGMENT, GL_FRAGMENT_SHADER);
            CASE_RETURN(ShaderType::TESSCONTROL, GL_TESS_CONTROL_SHADER);
            CASE_RETURN(ShaderType::TESSEVAL, GL_TESS_EVALUATION_SHADER);
            CASE_RETURN(ShaderType::VERTEX, GL_VERTEX_SHADER);
        }
        assert(false);
    }

    GLenum getGpuBufferType(BufferType in)
    {
        switch (in) {
            CASE_RETURN(BufferType::VERTEX, GL_ARRAY_BUFFER);
            CASE_RETURN(BufferType::INDEX, GL_ELEMENT_ARRAY_BUFFER);
            CASE_RETURN(BufferType::UNIFORM, GL_UNIFORM_BUFFER);
            CASE_RETURN(BufferType::STORAGE, GL_SHADER_STORAGE_BUFFER);
            CASE_RETURN(BufferType::TEXTURE, GL_TEXTURE_BUFFER);
        }
        assert(false);
    }

    GLenum getGpuTestFunction(TestFunction in)
    {
        switch (in) {
            CASE_RETURN(TestFunction::ALWAYS, GL_ALWAYS);
            CASE_RETURN(TestFunction::EQUAL, GL_EQUAL);
            CASE_RETURN(TestFunction::GEQUAL, GL_GEQUAL);
            CASE_RETURN(TestFunction::GREATER, GL_GREATER);
            CASE_RETURN(TestFunction::LEQUAL, GL_LEQUAL);
            CASE_RETURN(TestFunction::LESS, GL_LESS);
            CASE_RETURN(TestFunction::NEVER, GL_NEVER);
            CASE_RETURN(TestFunction::NOTEQUAL, GL_NOTEQUAL);
        }
        assert(false);
    }

    GLenum getGpuStencilOp(StencilOp in)
    {
        switch (in) {
            CASE_RETURN(StencilOp::DECR, GL_DECR);
            CASE_RETURN(StencilOp::DECRWRAP, GL_DECR_WRAP);
            CASE_RETURN(StencilOp::INCR, GL_INCR);
            CASE_RETURN(StencilOp::INCRWRAP, GL_INCR_WRAP);
            CASE_RETURN(StencilOp::INVERT, GL_INVERT);
            CASE_RETURN(StencilOp::KEEP, GL_KEEP);
            CASE_RETURN(StencilOp::REPLACE, GL_REPLACE);
            CASE_RETURN(StencilOp::ZERO, GL_ZERO);
        }
        assert(false);
    }

    GLenum getGpuBlendEquation(BlendEquation in)
    {
        switch (in) {
            CASE_RETURN(BlendEquation::ADD, GL_FUNC_ADD);
            CASE_RETURN(BlendEquation::MAX, GL_MAX);
            CASE_RETURN(BlendEquation::MIN, GL_MIN);
            CASE_RETURN(BlendEquation::SUBTRACT, GL_FUNC_SUBTRACT);
            CASE_RETURN(BlendEquation::REVERSE_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT);
        }
        assert(false);
    }

    GLenum getGpuBlendFunction(BlendFunction in)
    {
        switch (in) {
            CASE_RETURN(BlendFunction::DST_ALPHA, GL_DST_ALPHA);
            CASE_RETURN(BlendFunction::DST_COLOR, GL_DST_COLOR);
            CASE_RETURN(BlendFunction::ONE, GL_ONE);
            CASE_RETURN(BlendFunction::ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
            CASE_RETURN(BlendFunction::ONE_MINUS_DST_COLOR, GL_ONE_MINUS_DST_COLOR);
            CASE_RETURN(BlendFunction::ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            CASE_RETURN(BlendFunction::SRC_ALPHA, GL_SRC_ALPHA);
            CASE_RETURN(BlendFunction::SRC_COLOR, GL_SRC_COLOR);
            CASE_RETURN(BlendFunction::ZERO, GL_ZERO);
        }
        assert(false);
    }

    GLenum getGpuFace(Faces in)
    {
        switch (in) {
            CASE_RETURN(Faces::ALL, GL_FRONT_AND_BACK);
            CASE_RETURN(Faces::BACK, GL_BACK);
            CASE_RETURN(Faces::FRONT, GL_FRONT);
        }
        assert(false);
    }

    GLenum getGpuPoligonMode(PolygonMode in)
    {
        switch (in) {
            CASE_RETURN(PolygonMode::FILL, GL_FILL);
            CASE_RETURN(PolygonMode::LINE, GL_LINE);
            CASE_RETURN(PolygonMode::POINT, GL_POINT);
        }
        assert(false);
    }

    GLenum getGpuTextureShape(TextureShape in)
    {
        switch (in) {
            CASE_RETURN(TextureShape::D1, GL_TEXTURE_1D);
            CASE_RETURN(TextureShape::D2, GL_TEXTURE_2D);
            CASE_RETURN(TextureShape::D3, GL_TEXTURE_3D);
            CASE_RETURN(TextureShape::Cube, GL_TEXTURE_CUBE_MAP);
            CASE_RETURN(TextureShape::Array1D, GL_TEXTURE_1D_ARRAY);
            CASE_RETURN(TextureShape::Array2D, GL_TEXTURE_2D_ARRAY);
            CASE_RETURN(TextureShape::ArrayCube, GL_TEXTURE_CUBE_MAP_ARRAY);
        }
        assert(false);
    }

    GLenum getGpuFilter(Filter in)
    {
        switch (in) {
            CASE_RETURN(Filter::LINEAR, GL_LINEAR);
            CASE_RETURN(Filter::LINEAR_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
            CASE_RETURN(Filter::LINEAR_NEAREST, GL_LINEAR_MIPMAP_NEAREST);
            CASE_RETURN(Filter::NEAREST, GL_NEAREST);
            CASE_RETURN(Filter::NEAREST_LINEAR, GL_NEAREST_MIPMAP_LINEAR);
            CASE_RETURN(Filter::NEAREST_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
        }
        assert(false);
    }

    GLenum getGpuPixelLayout(Layout in)
    {
        switch (in)
        {
        case Layout::DEPTH24_STENCIL_8:
            return GL_DEPTH24_STENCIL8;
        case Layout::R:
            return GL_R;
        case Layout::R16:
            return GL_R16;
        case Layout::R16F:
            return GL_R16F;
        case Layout::RG:
            return GL_RG;
        case Layout::RG16:
            return GL_RG16;
        case Layout::RG16F:
            return GL_RG16F;
        case Layout::RGB:
            return GL_RGB;
        case Layout::RGBA:
            return GL_RGBA;
        case Layout::RGBA16F:
            return GL_RGBA16F;
        case Layout::RGB10A2:
            return GL_RGB10_A2;
        case Layout::RGBA32F:
            return GL_RGBA32F;
        case Layout::SRGB:
            return GL_SRGB8;
        case Layout::SRGB_A:
            return GL_SRGB8_ALPHA8;
        case Layout::R11F_G11F_B10F:
            return GL_R11F_G11F_B10F;
        case Layout::COMPRESSED_RGBA:
            return GL_COMPRESSED_RGBA;
        case Layout::COMPRESSED_SRGB:
            return GL_COMPRESSED_SRGB;
        case Layout::RGB565:
            return GL_RGB565;
        case Layout::RGB5_A1:
            return GL_RGB5_A1;
        }
        assert(false);
    }

    GLenum getGpuUniformFrequency(UniformFrequency in)
    {
        switch (in) {
            CASE_RETURN(UniformFrequency::STATIC, GL_STATIC_DRAW);
            CASE_RETURN(UniformFrequency::DYNAMIC, GL_DYNAMIC_DRAW);
            CASE_RETURN(UniformFrequency::FRAME, GL_DYNAMIC_DRAW);
            CASE_RETURN(UniformFrequency::VIEW, GL_DYNAMIC_DRAW);
        }
        assert(false);
    }


} // namespace GpuUtils
