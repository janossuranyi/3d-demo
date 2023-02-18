#include <string>
#include <GL/glew.h>
#include "gpu.h"
#include "window.h"
#include "logger.h"

SDL_Window*     GPU::_windowHandle{ NULL };
SDL_GLContext   GPU::_glcontext{ 0 };
Window          GPU::_window{};
bool            GPU::_valid{ false };
int             GPU::_glVersion{ 0 };

bool GPU::setup(const Window& awindow, const std::string& name)
{
    int err;

    if ((err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)) {
        Error("ERROR: %s", SDL_GetError());
        return false;
    }

    Info("SD_Init done");

    /*
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, awindow.depthBits);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, awindow.stencilBits);
    //SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    //SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_TRUE);

    if (awindow.multisample > 0)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, awindow.multisample);
    }

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    if (awindow.fullscreen && awindow.width == -1 && awindow.height == -1)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    else if (awindow.fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    Info("SD_CreateWindow start");
    _windowHandle = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, awindow.width, awindow.height, flags);
    if (!_windowHandle)
    {
        Error("Cannot create window - %s", SDL_GetError());
        return false;
    }

    Info("SD_CreateWindow done");

    _glcontext = SDL_GL_CreateContext(_windowHandle);
    if (!_glcontext)
    {
        Error("Cannot create GL context - %s", SDL_GetError());
        return false;
    }

    Info("SDL_GL_CreateContext done");

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        Error("Cannot initialize GLEW");
        return false;
    }

    Info("glewInit done");

    SDL_GL_SetSwapInterval(1);

    std::string renderer = (char*)glGetString(GL_RENDERER);
    std::string version = (char*)glGetString(GL_VERSION);
    const float gl_version = float(atof(version.c_str()));
    _glVersion = int(gl_version * 100);

    if (_glVersion < 450)
    {
        Error("Sorry, I need at least OpenGL 4.5");
        return false;
    }

    SDL_version ver;

    SDL_GetVersion(&ver);

    Info("GL Renderer: %s", renderer.c_str());
    Info("GL Version: %s (%.2f)", version.c_str(), gl_version);
    Info("SDL version: %d.%d patch %d", (int)ver.major, (int)ver.minor, (int)ver.patch);

    int _w, _h;

    SDL_GetWindowSize(_windowHandle, &_w, &_h);
    glViewport(0, 0, _w, _h);
    glScissor(0, 0, _w, _h);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_FRAMEBUFFER_SRGB);

#ifdef _DEBUG
    if (GLEW_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

        glDebugMessageCallbackARB(&DebugMessageCallback, NULL);
        //glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
        //glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

    }
#endif
    if (GLEW_ARB_explicit_uniform_location)
    {
        //videoConf.explicitUnifromLocationEXT = true;
    }

    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &_window.redBits);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &_window.greenBits);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &_window.blueBits);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &_window.depthBits);
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &_window.alphaBits);
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &_window.stencilBits);


    _window.width = _w;
    _window.height = _h;

    _valid = true;

    return true;
}

void GPU::close()
{
    if (_glcontext)
    {
        SDL_GL_DeleteContext(_glcontext);
    }
    if (_windowHandle)
    {
        SDL_DestroyWindow(_windowHandle);
    }
}

const Window& GPU::window()
{
    return _window;
}

bool GPU::valid()
{
    return _valid;
}

void GPU::flipSwapChain()
{
    SDL_GL_SwapWindow(_windowHandle);
}

void GPU::bind(const GpuBuffer& abuf)
{
    if (abuf.mBuffer != INVALID_BUFFER)
    {
        const GLenum target = GL_CastBufferType(abuf.mTarget);
        GL_CHECK(glBindBuffer(target, abuf.mBuffer));
    }
}

void GPU::bind(const GpuTexture& atex)
{
    if (atex.mTexture != INVALID_TEXTURE)
    {
        glBindTexture(atex.getApiTarget(), atex.mTexture);
    }
}

void GPU::bind(const GpuTexture& atex, unsigned int aunit)
{
    activeTexture(aunit);
    bind(atex);
}

void GPU::activeTexture(unsigned int aunit)
{
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + aunit));
}

void GPU::bindImageUnit(const GpuTexture& atex, int unit, int level, Access access, ImageFormat format)
{
    const GLenum access_ = GL_castImageAccess(access);
    const GLenum format_ = GL_castImageFormat(format);

    GL_CHECK(glBindImageTexture(GLuint(unit), atex.mTexture, GLint(level), GL_FALSE, 0, access_, format_));
}

void GPU::createTexture2D(GpuTexture2D& atex, int w, int h, int level, InternalFormat internalFormat, InputFormat format, ComponentType type, const void* data)
{
    if (atex.mTexture == INVALID_TEXTURE)
    {
        GL_CHECK(glGenTextures(1, &atex.mTexture));
        bind(atex);
    }

    const GLenum dataType = GL_castDataType(type);
    const GLenum pixFormat = GL_castPixelFormat(format);
    const GLint texFormat = GL_castTextureFormat(internalFormat);

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, level, texFormat, w, h, 0, pixFormat, dataType, (data ? data : nullptr)));

}

void GPU::createTextureCubeface(GpuTexture& atex, int w, int h, int level, int face, InternalFormat internalFormat, InputFormat format, ComponentType type, const void* data)
{
    if (atex.mTexture == INVALID_TEXTURE)
    {
        GL_CHECK(glGenTextures(1, &atex.mTexture));
        bind(atex);
    }

    const GLenum dataType = GL_castDataType(type);
    const GLenum pixFormat = GL_castPixelFormat(format);
    const GLint texFormat = GL_castTextureFormat(internalFormat);

    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, texFormat, w, h, 0, pixFormat, dataType, (data ? data : nullptr)));

}

void GPU::generateMipmap(const GpuTexture& atex)
{
    GL_CHECK(glGenerateMipmap(atex.getApiTarget()));
}

/*void GPU::generateMipmapCubeface(const GpuTexture& atex, int face)
{
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face));
}*/

void GPU::resetTexture1D()
{
    glBindTexture(GL_TEXTURE_1D, 0);
}

void GPU::resetTexture2D()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GPU::resetTextureCube()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GPU::updateTextureParameters(GpuTexture& atex)
{
    for (auto& p : atex._intParams)
    {
        GL_CHECK(glTexParameteri(atex.getApiTarget(), p.first, p.second));
    }

    for (auto& p : atex._floatParams)
    {
        GL_CHECK(glTexParameterf(atex.getApiTarget(), p.first, p.second));
    }
}

void GPU::deleteTexture(GpuTexture& atex)
{
    if (atex.mTexture != INVALID_TEXTURE)
    {
        GL_CHECK(glDeleteTextures(1, &atex.mTexture));
    }
}
