#include <SOIL2.h>

#include "effect_compute_test.h"
#include "filesystem.h"
#include "unit_rect.h"
#include "gpu_utils.h"
#include "logger.h"

bool ComputeTestEffect::Init()
{

    while (glGetError() != GL_NO_ERROR) {}

    tex0_.selectUnit(0);
    //tex0_.create(tex_w, tex_h, 0, eTextureFormat::RGBA, ePixelFormat::RGBA, eDataType::UNSIGNED_BYTE, nullptr);
    tex0_.createRGB8(tex_w, tex_h, 0);
    tex0_.withDefaultLinearClampEdge().updateParameters();
    tex0_.bindImage(0, 0, eImageAccess::WRITE_ONLY, eImageFormat::RGBA8);

    vbo_rect.create(sizeof(UNIT_RECT_WITH_ST), eGpuBufferUsage::STATIC, 0, UNIT_RECT_WITH_ST);

    layout.begin()
        .with(0, 2, eDataType::FLOAT, false, 0, sizeof(vertexLayout_t), &vbo_rect)
        .with(1, 2, eDataType::FLOAT, false, 8, sizeof(vertexLayout_t), &vbo_rect)
        .end();

    layout.bind();

    if (!prg_compute.loadComputeShader(g_fileSystem.resolve("assets/shaders/test_compute.cs.glsl")))
    {
        return false;
    }

    GLint glint{};
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &glint);
    Info("GL_MAX_UNIFORM_BLOCK_SIZE = %d", glint);

    //u_angle = prg_compute.getLocation("angle");
    prg_compute.bindUniformBlock("cb_vars", 0);
    cbo.create(128, eGpuBufferUsage::DYNAMIC, BA_WRITE_PERSISTENT, nullptr);
    cbo.bindIndexed(0);
    cb_vars = reinterpret_cast<cbvars_t*>(cbo.mapPeristentWrite());

    angle = 0.0f;
    cb_vars->angle = 0.0f;

    if (!prg_view.loadShader(
        g_fileSystem.resolve("assets/shaders/test_compute.vs.glsl"),
        g_fileSystem.resolve("assets/shaders/test_compute.fs.glsl")))
    {
        return false;
    }

    return true;
}

bool ComputeTestEffect::Update(float time)
{
    angle += 0.1f * time;
    cb_vars->angle = angle;
    GL_CHECK(glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT));

    return true;
}

bool ComputeTestEffect::HandleEvent(const SDL_Event* ev)
{
    return true;
}

void ComputeTestEffect::Render()
{
    if (syncObj) GL_CHECK( glDeleteSync(syncObj) );

    prg_compute.use();
    //GL_CHECK(glUniform1f(u_angle, angle ));

    prg_compute.dispatchCompute(tex_w, tex_h, 1);

    GL_CHECK(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
    GL_CHECK(syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    GL_CHECK(glWaitSync(syncObj, 0, GL_TIMEOUT_IGNORED));

    prg_view.use();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

}

ComputeTestEffect::~ComputeTestEffect() noexcept
{
    cbo.unMap();
}
