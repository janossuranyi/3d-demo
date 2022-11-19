
#include "context.h"

namespace ctx {

    std::unique_ptr<Context> Context::instance_;
    std::atomic_bool Context::ready_{false};


    Context* ctx::Context::default()
    {
        bool expected{ false };

        if (!ready_.load(std::memory_order_relaxed) && std::atomic_compare_exchange_strong(&ready_, &expected, true))
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

    gfx::VertexCache* Context::vertexCache()
    {
        return vc_.get();
    }

    Context::Context()
    {
        hwr_.reset(new gfx::Renderer());
        sm_.reset(new gfx::ShaderManager(hwr_.get()));
        vc_.reset(new gfx::VertexCache());
        vc_->start(hwr_.get(), STATIC_SIZE, FRAME_SIZE);
    }

}
