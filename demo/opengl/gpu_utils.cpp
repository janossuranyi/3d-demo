#include <GL/glew.h>
#include <SDL.h>
#include "logger.h"
#include "gpu_types.h"
#include "gpu_utils.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        Error("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

GLenum GL_castDataType(eDataType type)
{
    switch (type) {
    case eDataType::BYTE:                return GL_BYTE;
    case eDataType::UNSIGNED_BYTE:       return GL_UNSIGNED_BYTE;
    case eDataType::SHORT:               return GL_SHORT;
    case eDataType::UNSIGNED_SHORT:      return GL_UNSIGNED_SHORT;
    case eDataType::INT32:               return GL_INT;
    case eDataType::UNSIGNED_INT32:      return GL_UNSIGNED_INT;
    case eDataType::FLOAT:               return GL_FLOAT;
    case eDataType::HALF_FLOAT:          return GL_HALF_FLOAT;
    case eDataType::UNSIGNED_INT_24_8:   return GL_UNSIGNED_INT_24_8;
    }
}

GLenum GL_castPixelFormat(ePixelFormat pf)
{
    using PF = ePixelFormat;

    switch (pf)
    {
    case PF::RGB:
        return GL_RGB;
    case PF::RGBA:
        return GL_RGBA;
    case PF::RGB8:
        return GL_RGB8;
    case PF::RGBA8:
        return GL_RGBA8;
    case PF::RGBA16:
        return GL_RGBA16;
    }
}

GLint GL_castTextureFormat(eTextureFormat f)
{
    switch (f)
    {
    case eTextureFormat::DEPTH24_STENCIL_8:
        return GL_DEPTH24_STENCIL8;
    case eTextureFormat::R:
        return GL_R;
    case eTextureFormat::R16:
        return GL_R16;
    case eTextureFormat::R16F:
        return GL_R16F;
    case eTextureFormat::RG:
        return GL_RG;
    case eTextureFormat::RG16:
        return GL_RG16;
    case eTextureFormat::RG16F:
        return GL_RG16F;
    case eTextureFormat::RGB:
        return GL_RGB;
    case eTextureFormat::RGBA:
        return GL_RGBA;
    case eTextureFormat::RGBA16F:
        return GL_RGBA16F;
    case eTextureFormat::RGB10A2:
        return GL_RGB10_A2;
    case eTextureFormat::RGBA32F:
        return GL_RGBA32F;
    case eTextureFormat::SRGB:
        return GL_SRGB8;
    case eTextureFormat::SRGB_A:
        return GL_SRGB8_ALPHA8;
    case eTextureFormat::R11F_G11F_B10F:
        return GL_R11F_G11F_B10F;
    case eTextureFormat::COMPRESSED_RGBA:
        return GL_COMPRESSED_RGBA;
    case eTextureFormat::COMPRESSED_SRGB:
        return GL_COMPRESSED_SRGB;
    case eTextureFormat::RGB565:
        return GL_RGB565;
    case eTextureFormat::RGB5_A1:
        return GL_RGB5_A1;
    }
}

GLenum GL_castShaderStage(eShaderStage type)
{
    switch (type)
    {
    case eShaderStage::COMPUTE:
        return GL_COMPUTE_SHADER;
    case eShaderStage::VERTEX:
        return GL_VERTEX_SHADER;
    case eShaderStage::GEOMETRY:
        return GL_GEOMETRY_SHADER;
    case eShaderStage::FRAGMENT:
        return GL_FRAGMENT_SHADER;
    case eShaderStage::TESS_CONTROL:
        return GL_TESS_CONTROL_SHADER;
    case eShaderStage::TESS_EVALUATION:
        return GL_TESS_EVALUATION_SHADER;
    }
}

GLint GL_castTexWrap(eTexWrap p)
{
    switch (p)
    {
        case eTexWrap::CLAMP_TO_BORDER:     return GL_CLAMP_TO_BORDER;
        case eTexWrap::CLAMP_TO_EDGE:       return GL_CLAMP_TO_EDGE;
        case eTexWrap::MIRRORED_REPEAT:     return GL_MIRRORED_REPEAT;
        case eTexWrap::MIRROR_CLAMP_TO_EDGE:return GL_MIRROR_CLAMP_TO_EDGE;
        case eTexWrap::REPEAT:              return GL_REPEAT;
    }
}

GLenum GL_castImageAccess(eImageAccess p)
{
    switch (p)
    {
        case eImageAccess::READ_ONLY:   return GL_READ_ONLY;
        case eImageAccess::WRITE_ONLY:  return GL_WRITE_ONLY;
        case eImageAccess::READ_WRITE:  return GL_READ_WRITE;
    }
}

GLenum GL_castImageFormat(eImageFormat p)
{
    switch (p)
    {
        case eImageFormat::RGBA16F:     return GL_RGBA16F;
        case eImageFormat::RGBA32F:     return GL_RGBA32F;
        case eImageFormat::RGBA8:       return GL_RGBA8;
    }
}

GLenum GL_castDrawMode(eDrawMode p)
{
    switch (p)
    {
    case eDrawMode::LINES:
        return GL_LINES;
    case eDrawMode::LINE_LOOP:
        return GL_LINE_LOOP;
    case eDrawMode::LINE_STRIP:
        return GL_LINE_STRIP;
    case eDrawMode::POINTS:
        return GL_POINTS;
    case eDrawMode::TRIANGLES:
        return GL_TRIANGLES;
    case eDrawMode::TRIANGLE_FAN:
        return GL_TRIANGLE_FAN;
    case eDrawMode::TRIANGLE_STRIP:
        return GL_TRIANGLE_STRIP;
    }
}

const char* GetShaderStageTitle(eShaderStage type)
{
    switch (type)
    {
    case eShaderStage::COMPUTE:
        return "Compute";
    case eShaderStage::VERTEX:
        return "Vertex";
    case eShaderStage::GEOMETRY:
        return "Geometry";
    case eShaderStage::FRAGMENT:
        return "Fragment";;
    case eShaderStage::TESS_CONTROL:
        return "Tesselation control";
    case eShaderStage::TESS_EVALUATION:
        return "Tesselation evaluation";
    }
}

void GLAPIENTRY DebugMessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const GLvoid* userParam)
{
    // Convert enums into a humen readable text
    // See: https://www.opengl.org/registry/specs/ARB/debug_output.txt

//    if (severity > GL_DEBUG_SEVERITY_MEDIUM) return;

    const char* sourceText = "Unknown";
    switch (source)
    {
    case GL_DEBUG_SOURCE_API_ARB:
        // The GL
        sourceText = "API";
        break;
        // The GLSL shader compiler or compilers for other extension - provided languages
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
        sourceText = "Shader compiler";
        break;
        // The window system, such as WGL or GLX
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
        sourceText = "Window system";
        break;
        // External debuggers or third-party middleware libraries
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
        sourceText = "Third party";
        break;
        // The application
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
        sourceText = "Application";
        break;
        // Sources that do not fit to any of the ones listed above
    case GL_DEBUG_SOURCE_OTHER_ARB:
        sourceText = "Other";
        break;
    }

    const char* typeText = "Unknown";
    switch (type)
    {
        // Events that generated an error
    case GL_DEBUG_TYPE_ERROR_ARB:
        typeText = "Error";
        break;
        // Behavior that has been marked for deprecation
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
        typeText = "Deprecated behavior";
        break;
        // Behavior that is undefined according to the specification
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
        typeText = "Undefined behavior";
        break;
        // Implementation-dependent performance warnings
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
        typeText = "Performance";
        break;
        // Use of extensions or shaders in a way that is highly vendor - specific
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
        typeText = "Portability";
        break;
        // Types of events that do not fit any of the ones listed above
    case GL_DEBUG_TYPE_OTHER_ARB:
        typeText = "Other";
        break;
    }

    const char* severityText = "Unknown";
    switch (severity)
    {
        // Any GL error; dangerous undefined behavior; any GLSL or ARB shader compiler and linker errors;
    case GL_DEBUG_SEVERITY_HIGH_ARB:
        severityText = "High";
        break;
        // Severe performance warnings; GLSL or other shader compiler and linker warnings; use of currently deprecated behavior
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
        severityText = "Medium";
        break;
        // Performance warnings from redundant state changes; trivial undefined behavior
    case GL_DEBUG_SEVERITY_LOW_ARB:
        severityText = "Low";
        break;
    }

    // Unused params
    (void)id;
    (void)length;
    (void)userParam;

    // Replace LogDebug with your logging function
    Info("[OpenGL:source='%s', type='%s', severity='%s'] %s",
        sourceText,
        typeText,
        severityText,
        message);
}