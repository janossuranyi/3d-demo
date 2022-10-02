#include "GPUInternal.h"

#define CASE_RETURN(c,r) case c: return r;

namespace GpuUtils
{
    GLenum getGpuWrap(Wrap in)
    {
        switch(in) {
            CASE_RETURN(Wrap::CLAMP,    GL_CLAMP_TO_EDGE)
            CASE_RETURN(Wrap::MIRROR,   GL_MIRRORED_REPEAT)
            CASE_RETURN(Wrap::REPEAT,   GL_REPEAT)
        }

        return GL_CLAMP_TO_EDGE;
    }

    GLenum getGpuShaderType(ShaderType in)
    {
        switch(in) {
            CASE_RETURN(ShaderType::COMPUTE,        GL_COMPUTE_SHADER)
            CASE_RETURN(ShaderType::FRAGMENT,       GL_FRAGMENT_SHADER)
            CASE_RETURN(ShaderType::TESSCONTROL,    GL_TESS_CONTROL_SHADER)
            CASE_RETURN(ShaderType::TESSEVAL,       GL_TESS_EVALUATION_SHADER)
            CASE_RETURN(ShaderType::VERTEX,         GL_VERTEX_SHADER);
        }
        assert(false);
    }

    
} // namespace GpuUtils
