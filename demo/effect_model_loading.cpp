#include <string>
#include <glm/glm.hpp>
#include "gpu_buffer.h"
#include "gpu_program.h"
#include "pipeline.h"
#include "filesystem.h"
#include "demo.h"
#include "unit_rect.h"

#include "effect_model_loading.h"

bool LoadModelEffect::Init()
{

    if (!world.loadWorld(g_fileSystem.resolve(worldFile)))
    {
        Warning("World %s cannot load", worldFile.c_str());
        return false;
    }

    glm::vec3 clearColor = glm::pow(glm::vec3(.4f), glm::vec3(1.f / 2.2f));

    pipeline.setScreenRect(0, 0, videoConf.width, videoConf.height);
    pipeline.setState( GLS_DEPTHFUNC_LESS|GLS_CULL_FRONTSIDED/* | GLS_POLYMODE_LINE*/);
    pipeline.setWorldPosition(vec3(0, 0, 0));
    pipeline.setWorldScale(vec3(1, 1, 1));
    pipeline.setWorldEulerRotation(vec3(0, 0, 0));
    pipeline.setPerspectiveCamera(glm::radians(45.0f), 0.01f, 100.0f, 0.0f);
    pipeline.setClearColor(clearColor.r,clearColor.g,clearColor.b, 1.0f);

    posZ = 3.0f;

    if (!shader.loadShader(
        g_fileSystem.resolve("assets/shaders/model_test.vs.glsl"),
        g_fileSystem.resolve("assets/shaders/model_test_gloss.fs.glsl")))
    {
        return false;
    }

    if (!fxaa.loadShader(
        g_fileSystem.resolve("assets/shaders/fxaa.vs.glsl"),
        g_fileSystem.resolve("assets/shaders/fxaa3.fs.glsl")))
    {
        return false;
    }
    //shader.bindUniformBlock("cb_matrix", 1);

    pipeline.useProgram(shader);

    shader.set("samp0_albedo", 0);
    shader.set("samp1_normal", 1);
    shader.set("samp2_pbr", 2);
    shader.set("samp3_emissive", 3);
    shader.set("samp4_ao", 4);
    
    pipeline.useProgram(fxaa);
    //fxaa.set("screenTexture", 0);

    pipeline.bindConstantBuffers();
    //glEnable(GL_FRAMEBUFFER_SRGB);

//    shader.set(4, TEX_EMISSIVE);
//    shader.set(5, TEX_AO);


    fb_color = GpuTexture2D::createShared();
    fb_color->createRGB(videoConf.width, videoConf.height, 0);
    fb_color->withDefaultLinearClampEdge().updateParameters();
    fb_depth = GpuTexture2D::createShared();
    fb_depth->createDepthStencil(videoConf.width, videoConf.height);
    if (!fb.create()
        .addColorAttachment(0, fb_color)
        .setDepthStencilAttachment(fb_depth)
        .checkCompletness())
    {
        return false;
    }

    pipeline.setClearDepth(1.f);

    GpuFrameBuffer::bindDefault();

    rectBuffer.create(sizeof(UNIT_RECT_WITH_ST), eGpuBufferUsage::STATIC, 0, UNIT_RECT_WITH_ST);
    rectBuffer.bind();
    vertFormat.begin();
    vertFormat
        .with(0, 2, eDataType::FLOAT, false, 0, 16, &rectBuffer)
        .with(1, 2, eDataType::FLOAT, false, 8, 16, &rectBuffer)
        .end();


    return true;
}

bool LoadModelEffect::Update(float time)
{
    angleY += 0.01 * time;
    angleY = std::fmodf(angleY, 360.0f);
    return true;
}

bool LoadModelEffect::HandleEvent(const SDL_Event* ev, float time)
{
    if (ev->type == SDL_KEYDOWN)
    {
        if (ev->key.keysym.sym == SDLK_UP)
        {
            posZ -= 0.001 * time;
        }
        else if (ev->key.keysym.sym == SDLK_DOWN)
        {
            posZ += 0.001 * time;
        }
        else if (ev->key.keysym.sym == SDLK_LEFT)
        {
            posY -= 0.001 * time;
        }
        else if (ev->key.keysym.sym == SDLK_RIGHT)
        {
            posY += 0.001 * time;
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

    GpuFrameBuffer::bindDefault();

    pipeline.setLayout(vertFormat);
    pipeline.bindTexture(*fb_color, 0);
    pipeline.setState(GLS_DEPTHFUNC_ALWAYS | GLS_DEPTHMASK | GLS_CULL_TWOSIDED);
    pipeline.useProgram(fxaa);

    pipeline.drawArrays(eDrawMode::TRIANGLES, 0, 6);
    
}
