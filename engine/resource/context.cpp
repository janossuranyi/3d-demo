#include "context.h"

namespace ctx {

    std::unique_ptr<Context> Context::instance_;

    Context* ctx::Context::default()
    {
        if (!instance_)
        {
            instance_.reset(new Context());
        }

        return instance_.get();
    }

    gfx::Renderer* Context::hwr()
    {
        return hwr_.get();
    }

    gfx::ShaderManager* Context::shaderManager()
    {
        return sm_.get();
    }

    Context::Context()
    {
        hwr_.reset(new gfx::Renderer());
        sm_.reset(new gfx::ShaderManager(hwr_.get()));
    }

}
