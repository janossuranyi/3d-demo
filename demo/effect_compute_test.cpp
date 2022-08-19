#include "effect_compute_test.h"
#include "filesystem.h"
#include "unit_rect.h"
#include "gpu_utils.h"

bool ComputeTestEffect::Init()
{

    while (glGetError() != GL_NO_ERROR) {}

    //GL_CHECK(glGenTextures(1, &tex0));
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, tex0);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    //GL_CHECK(glBindImageTexture(0, tex0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8));
    //GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex0));

    tex0_.selectUnit(0);
    tex0_.create(tex_w, tex_h, 0, eTextureFormat::RGBA, ePixelFormat::RGBA, eDataType::UNSIGNED_BYTE, nullptr);
    tex0_.withDefaultLinearClampEdge().updateParameters();
    tex0_.bindImage(0, 0, eImageAccess::WRITE_ONLY, eImageFormat::RGBA8);

    vbo_rect.create(sizeof(UNIT_RECT_WITH_ST), eGpuBufferUsage::STATIC, UNIT_RECT_WITH_ST);

    /* setup vertex layout */
    GL_CHECK(glCreateVertexArrays(1, &vao));
    GL_CHECK(glBindVertexArray(vao));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glEnableVertexAttribArray(1));
    vbo_rect.bind();
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertexLayout_t), (const void*)0));
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertexLayout_t), (const void*)8));
    GL_CHECK(glBindVertexArray(0));
    GL_CHECK(glDisableVertexAttribArray(0));
    GL_CHECK(glDisableVertexAttribArray(1));

    if (!prg_compute.loadComputeShader(g_fileSystem.resolve("assets/shaders/test_compute.cs.glsl")))
    {
        return false;
    }

    u_angle = prg_compute.getLocation("angle");

    if (!prg_view.loadShader(
        g_fileSystem.resolve("assets/shaders/test_compute.vs.glsl"),
        g_fileSystem.resolve("assets/shaders/test_compute.fs.glsl")))
    {
        return false;
    }

    glBindVertexArray(vao);

    return true;
}

bool ComputeTestEffect::Update(float time)
{
    angle += 0.1f * time;

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
    GL_CHECK(glUniform1f(u_angle, angle ));

    GL_CHECK(glDispatchCompute(tex_w, tex_h, 1));
    GL_CHECK(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
    GL_CHECK(syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    GL_CHECK(glWaitSync(syncObj, 0, GL_TIMEOUT_IGNORED));

    prg_view.use();
    glDrawArrays(GL_TRIANGLES, 0, 6);

}

ComputeTestEffect::~ComputeTestEffect() noexcept
{
    if (vao != 0xffff) glDeleteVertexArrays(1, &vao);
}
