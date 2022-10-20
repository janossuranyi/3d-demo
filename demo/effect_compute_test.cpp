#include "effect_compute_test.h"
#include "resource/filesystem.h"
#include "unit_rect.h"
#include "gpu.h"
#include "gpu_utils.h"
#include "gpu_types.h"
#include "logger.h"

bool ComputeTestEffect::Init()
{
    while (glGetError() != GL_NO_ERROR) {}

    tex0_.selectUnit(0);
    //tex0_.create(tex_w, tex_h, 0, eTextureFormat::RGBA, ePixelFormat::RGBA, eDataType::UNSIGNED_BYTE, nullptr);
    tex0_.createRGB8(tex_w, tex_h, 0);
    tex0_.withDefaultLinearClampEdge().updateParameters();
    GPU::bindImageUnit(tex0_, 0, 0, Access::WRITE_ONLY, ImageFormat::RGBA8);

    vbo_rect.reset(new GpuBuffer(BufferTarget::VERTEX));
    vbo_rect->create(sizeof(UNIT_RECT_WITH_ST), BufferUsage::STATIC, 0, UNIT_RECT_WITH_ST);

    layout.begin()
        .with(0, 2, ComponentType::FLOAT, false, 0, sizeof(vertexLayout_t), vbo_rect.get())
        .with(1, 2, ComponentType::FLOAT, false, 8, sizeof(vertexLayout_t), vbo_rect.get())
        .end();

    layout.bind();

    if (!prg_compute.loadComputeShader(FileSystem::resolve("assets/shaders/test_compute.cs.glsl")))
    {
        return false;
    }

    GLint glint{};
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &glint);
    Info("GL_MAX_UNIFORM_BLOCK_SIZE = %d", glint);

    //u_angle = prg_compute.getLocation("angle");
    if (!prg_compute.mapLocationToIndex("fa", 0))
    {
        return false;
    }

    fa = 0.7f;
    prg_compute.use();
    prg_compute.set(0, fa);
    prg_compute.bindUniformBlock("cb_vars", 0);
    cbo.reset(new GpuBuffer(BufferTarget::UNIFORM));
    cbo->create(128, BufferUsage::DYNAMIC, BA_WRITE_PERSISTENT, nullptr);
    cbo->bindIndexed(0);
    cb_vars = reinterpret_cast<cbvars_t*>(cbo->mapPeristentWrite());

    angle = 0.0f;
    cb_vars->angle = 0.0f;

    if (!prg_view.loadShader(
        FileSystem::resolve("assets/shaders/test_compute.vs.glsl"),
        FileSystem::resolve("assets/shaders/test_compute.fs.glsl")))
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

bool ComputeTestEffect::HandleEvent(const SDL_Event* ev, float time)
{
    return true;
}

void ComputeTestEffect::Render()
{
    if (syncObj) GL_CHECK( glDeleteSync(syncObj) );

    prg_compute.use();
    prg_compute.dispatchCompute(tex_w, tex_h, 1);

    GL_CHECK(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
    GL_CHECK(syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    GL_CHECK(glWaitSync(syncObj, 0, GL_TIMEOUT_IGNORED));

    prg_view.use();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

}

ComputeTestEffect::~ComputeTestEffect() noexcept
{
    //cbo->unMap();
}
