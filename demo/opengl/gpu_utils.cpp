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

GLenum GL_castDataType(ComponentType type)
{
    switch (type) {
    case ComponentType::BYTE:                return GL_BYTE;
    case ComponentType::UNSIGNED_BYTE:       return GL_UNSIGNED_BYTE;
    case ComponentType::SHORT:               return GL_SHORT;
    case ComponentType::UNSIGNED_SHORT:      return GL_UNSIGNED_SHORT;
    case ComponentType::INT32:               return GL_INT;
    case ComponentType::UNSIGNED_INT32:      return GL_UNSIGNED_INT;
    case ComponentType::FLOAT:               return GL_FLOAT;
    case ComponentType::HALF_FLOAT:          return GL_HALF_FLOAT;
    case ComponentType::UNSIGNED_INT_24_8:   return GL_UNSIGNED_INT_24_8;
    }
}

GLenum GL_castPixelFormat(InputFormat pf)
{
    using PF = InputFormat;

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
    case PF::RGB16:
        return GL_RGB16;
    }
}

GLint GL_castTextureFormat(InternalFormat f)
{
    switch (f)
    {
    case InternalFormat::DEPTH24_STENCIL_8:
        return GL_DEPTH24_STENCIL8;
    case InternalFormat::R:
        return GL_R;
    case InternalFormat::R16:
        return GL_R16;
    case InternalFormat::R16F:
        return GL_R16F;
    case InternalFormat::RG:
        return GL_RG;
    case InternalFormat::RG16:
        return GL_RG16;
    case InternalFormat::RG16F:
        return GL_RG16F;
    case InternalFormat::RGB:
        return GL_RGB;
    case InternalFormat::RGBA:
        return GL_RGBA;
    case InternalFormat::RGBA16F:
        return GL_RGBA16F;
    case InternalFormat::RGB10A2:
        return GL_RGB10_A2;
    case InternalFormat::RGBA32F:
        return GL_RGBA32F;
    case InternalFormat::SRGB:
        return GL_SRGB8;
    case InternalFormat::SRGB_A:
        return GL_SRGB8_ALPHA8;
    case InternalFormat::R11F_G11F_B10F:
        return GL_R11F_G11F_B10F;
    case InternalFormat::COMPRESSED_RGBA:
        return GL_COMPRESSED_RGBA;
    case InternalFormat::COMPRESSED_SRGB:
        return GL_COMPRESSED_SRGB;
    case InternalFormat::RGB565:
        return GL_RGB565;
    case InternalFormat::RGB5_A1:
        return GL_RGB5_A1;
    }
}

GLenum GL_castShaderStage(ShaderType type)
{
    switch (type)
    {
    case ShaderType::COMPUTE:
        return GL_COMPUTE_SHADER;
    case ShaderType::VERTEX:
        return GL_VERTEX_SHADER;
    case ShaderType::GEOMETRY:
        return GL_GEOMETRY_SHADER;
    case ShaderType::FRAGMENT:
        return GL_FRAGMENT_SHADER;
    case ShaderType::TESS_CONTROL:
        return GL_TESS_CONTROL_SHADER;
    case ShaderType::TESS_EVALUATION:
        return GL_TESS_EVALUATION_SHADER;
    }
}

GLint GL_castTexWrap(Wrap p)
{
    switch (p)
    {
        case Wrap::CLAMP_TO_BORDER:     return GL_CLAMP_TO_BORDER;
        case Wrap::CLAMP_TO_EDGE:       return GL_CLAMP_TO_EDGE;
        case Wrap::MIRRORED_REPEAT:     return GL_MIRRORED_REPEAT;
        case Wrap::MIRROR_CLAMP_TO_EDGE:return GL_MIRROR_CLAMP_TO_EDGE;
        case Wrap::REPEAT:              return GL_REPEAT;
    }
}

GLenum GL_castImageAccess(Access p)
{
    switch (p)
    {
        case Access::READ_ONLY:   return GL_READ_ONLY;
        case Access::WRITE_ONLY:  return GL_WRITE_ONLY;
        case Access::READ_WRITE:  return GL_READ_WRITE;
    }
}

GLenum GL_castImageFormat(ImageFormat p)
{
    switch (p)
    {
        case ImageFormat::RGBA16F:     return GL_RGBA16F;
        case ImageFormat::RGBA32F:     return GL_RGBA32F;
        case ImageFormat::RGBA8:       return GL_RGBA8;
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

GLbitfield GL_castBufferAccessFlags(unsigned int pAccess)
{
    GLbitfield access{};

    if (pAccess & BA_MAP_READ)
    {
        access |= GL_MAP_READ_BIT;
    }
    if (pAccess & BA_MAP_WRITE)
    {
        access |= GL_MAP_WRITE_BIT;
    }
    if (pAccess & BA_MAP_COHERENT)
    {
        access |= GL_MAP_COHERENT_BIT;
    }
    if (pAccess & BA_MAP_PERSISTENT)
    {
        access |= GL_MAP_PERSISTENT_BIT;
    }
    if (pAccess & BA_DYNAMIC)
    {
        access |= GL_DYNAMIC_STORAGE_BIT;
    }
    if (!(pAccess & BA_MAP_PERSISTENT))
    {
        access &= ~GL_MAP_UNSYNCHRONIZED_BIT;
    }

    return access;
}

const char* GetShaderStageTitle(ShaderType type)
{
    switch (type)
    {
    case ShaderType::COMPUTE:
        return "Compute";
    case ShaderType::VERTEX:
        return "Vertex";
    case ShaderType::GEOMETRY:
        return "Geometry";
    case ShaderType::FRAGMENT:
        return "Fragment";;
    case ShaderType::TESS_CONTROL:
        return "Tesselation control";
    case ShaderType::TESS_EVALUATION:
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