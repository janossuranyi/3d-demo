#include <string>
#include <glm/glm.hpp>
#include "gpu.h"
#include "gpu_buffer.h"
#include "gpu_program.h"
#include "gpu_types.h"
#include "pipeline.h"
#include "resource/filesystem.h"
#include "demo.h"
#include "unit_rect.h"

#include "effect_model_loading.h"

static const int ResDiv = 32;

bool LoadModelEffect::Init()
{
    world.init();

    if (!world.loadWorld(FileSystem::resolve(worldFile)))
    {
        Warning("World %s cannot load", worldFile.c_str());
        return false;
    }

    glm::vec3 clearColor = glm::pow(glm::vec3(0.05f, 0.05f, 0.2f), glm::vec3(1.f / 2.2f));

    pipeline.setScreenRect(0, 0, GPU::window().width, GPU::window().height);
    pipeline.setState(GLS_DEPTHFUNC_LESS | GLS_CULL_FRONTSIDED/* | GLS_POLYMODE_LINE*/);
    pipeline.setWorldPosition(vec3(0, 0, 0));
    pipeline.setWorldScale(vec3(1, 1, 1));
    pipeline.setWorldEulerRotation(vec3(0, 0, 0));
    pipeline.setPerspectiveCamera(glm::radians(45.0f), 0.01f, 100.0f, 0.0f);
    pipeline.setClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);

    posZ = 3.0f;

    if (!shader.loadShader(
        FileSystem::resolve("assets/shaders/model_test.vs.glsl"),
        FileSystem::resolve("assets/shaders/model_test_gloss.fs.glsl")))
    {
        return false;
    }

    if (!fxaa.loadShader(
        FileSystem::resolve("assets/shaders/fxaa.vs.glsl"),
        FileSystem::resolve("assets/shaders/bloom.fs.glsl")))
    {
        return false;
    }

    if (!gauss.loadShader(
        FileSystem::resolve("assets/shaders/fxaa.vs.glsl"),
        FileSystem::resolve("assets/shaders/gauss_filter.fs.glsl")))
    {
        return false;
    }

    //shader.bindUniformBlock("cb_matrix", 1);

    pipeline.useProgram(shader);
    pipeline.useProgram(fxaa);
    pipeline.bindConstantBuffers();
    //glEnable(GL_FRAMEBUFFER_SRGB);

    cb_color = GpuTexture2D::createShared();
    cb_color->createRGB16F(GPU::window().width, GPU::window().height, 0);
    cb_color->withDefaultLinearClampEdge().updateParameters();
    cb_color2 = GpuTexture2D::createShared();
    cb_color2->createRGB16F(GPU::window().width, GPU::window().height, 0);
    cb_color2->withDefaultLinearClampEdge().updateParameters();
    cb_depth = GpuTexture2D::createShared();
    cb_depth->createDepthStencil(GPU::window().width, GPU::window().height);
    if (!fb.create()
        .addColorAttachment(0, cb_color2)
        .addColorAttachment(1, cb_color)
        .setDepthStencilAttachment(cb_depth)
        .checkCompletness())
    {
        return false;
    }

    cb_copy = GpuTexture2D::createShared();
    cb_copy->createRGB8(GPU::window().width, GPU::window().height, 0);
    cb_copy->withDefaultLinearClampEdge().updateParameters();
    if (!fb_copy.create()
        .addColorAttachment(0, cb_copy)
        .checkCompletness()) {
        return false;
    }

    for (int i = 0; i < 2; ++i)
    {
        cb_blur[i] = GpuTexture2D::createShared();
        cb_blur[i]->createRGB8(GPU::window().width / ResDiv, GPU::window().height / ResDiv, 0);
        cb_blur[i]->withDefaultLinearClampEdge().updateParameters();

        if (!fb_blur[i].create()
            .addColorAttachment(0, cb_blur[i])
            .checkCompletness())
        {
            return false;
        }
    }

    pipeline.setClearDepth(1.f);

    GpuFrameBuffer::bindDefault();
    
    rectBuffer.reset(new GpuBuffer());
    rectBuffer->create(sizeof(UNIT_RECT_WITH_ST), BufferUsage::STATIC, 0, UNIT_RECT_WITH_ST);
    GPU::bind(*rectBuffer);

    vertFormat.begin();
    vertFormat
        .with(0, 2, ComponentType::FLOAT, false, 0, 16, rectBuffer.get())
        .with(1, 2, ComponentType::FLOAT, false, 8, 16, rectBuffer.get())
        .end();


    return true;
}

bool LoadModelEffect::Update(float time)
{
    angleY += 0.01f * time;
    angleY = std::fmodf(angleY, 360.0f);
    return true;
}

bool LoadModelEffect::HandleEvent(const SDL_Event* ev, float time)
{
    if (ev->type == SDL_KEYDOWN)
    {
        if (ev->key.keysym.sym == SDLK_UP)
        {
            posZ -= 0.001f * time;
        }
        else if (ev->key.keysym.sym == SDLK_DOWN)
        {
            posZ += 0.001f * time;
        }
        else if (ev->key.keysym.sym == SDLK_LEFT)
        {
            posY -= 0.001f * time;
        }
        else if (ev->key.keysym.sym == SDLK_RIGHT)
        {
            posY += 0.001f * time;
        }
    }
    return true;
}

void LoadModelEffect::Render()
{
    fb.bind();


    pipeline.setWorldEulerRotation(vec3(glm::radians(-15.0f), glm::radians(angleY), 0));
    pipeline.setView(vec3(0, posY, posZ), vec3(0, posY-1.0f, posZ-3.0f));
    pipeline.setState(GLS_DEPTHFUNC_LESS | GLS_CULL_FRONTSIDED /* | GLS_POLYMODE_LINE */);
    pipeline.useProgram(shader);
    pipeline.clear(true, true, false);

    world.renderWorld(pipeline);

    pipeline.setState(GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS | GLS_CULL_TWOSIDED);
    int w = GPU::window().width, h = GPU::window().height;
    int active_fb = 0;

    fb.bindToRead();
    fb_copy.bindToWrite();
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    fb_copy.bindToRead();
    fb_blur[1-active_fb].bindToWrite();
    glBlitFramebuffer(0, 0, w, h, 0, 0, w / ResDiv, h / ResDiv, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glm::vec2 u_direction(1, 0);
    pipeline.useProgram(gauss);
    pipeline.setLayout(vertFormat);
    pipeline.setScreenRect(0, 0, w / ResDiv, h / ResDiv);

    for (int i = 0; i < 2; ++i)
    {
        fb_blur[active_fb].bind();
        pipeline.bindTexture(*cb_blur[1 - active_fb], 0);
        gauss.set("u_direction", u_direction);
        pipeline.drawArrays(eDrawMode::TRIANGLES, 0, 6);

        fb_blur[1-active_fb].bind();
        pipeline.bindTexture(*cb_blur[active_fb], 0);
        gauss.set("u_direction", vec2(1.0)-u_direction);
        pipeline.drawArrays(eDrawMode::TRIANGLES, 0, 6);

    }
    
    pipeline.bindDefaultFramebuffer();

    pipeline.setScreenRect(0, 0, w, h );
    pipeline.setLayout(vertFormat);
//    pipeline.bindTexture(*cb_color, 0);
    pipeline.bindTexture(*cb_color, 0);
    pipeline.bindTexture(*cb_blur[1-active_fb], 1);
    pipeline.setState(GLS_DEPTHFUNC_ALWAYS | GLS_DEPTHMASK | GLS_CULL_TWOSIDED);
    pipeline.useProgram(fxaa);
    fxaa.set("exposure", 1.2f);

    pipeline.drawArrays(eDrawMode::TRIANGLES, 0, 6);
    
}
