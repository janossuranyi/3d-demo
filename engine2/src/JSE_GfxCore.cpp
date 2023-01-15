#include "JSE.h"

JSE_Result JSE_GfxCore::Init()
{
    return Init_impl();
}

JSE_Result JSE_GfxCore::CreateSurface(const JSE_CreateSurfaceInfo& createSurfaceInfo)
{
    if (createSurfaceInfo.width == 0 && createSurfaceInfo.height == 0 && createSurfaceInfo.fullScreen == false) {
        return JSE_Result::INVALID_SURFACE_DIMENSION;
    }

    return CreateSurface_impl(createSurfaceInfo);
}

void JSE_GfxCore::Shutdown()
{
    Shutdown_impl();
}
