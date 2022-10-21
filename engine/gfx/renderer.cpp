#include "renderer.h"
#include "logger.h"
#include "gl_context.h"

namespace gfx {

	void View::clear() {
		clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		render_items.clear();
		frame_buffer = FrameBufferHandle::invalid;
	}

	Renderer::Renderer() :
		width_{0},
		height_{0},
		window_title_{"JS-Engine"},
		use_thread_{false},
		render_thread_{},
		swapped_frames_{true},
		submit_{&frames_[0]},
		render_{ &frames_[1] }
	{}

	Renderer::~Renderer()
	{
	}

	bool Renderer::init(RendererType type, uint16_t width, uint16_t height, const std::string& title, bool use_thread) {
		if (type != RendererType::OpenGL) {
			Error("OpenGL only yet!!!");
			return false;
		}

		shared_render_context_.reset(new OpenGLRenderContext());
		shared_render_context_->create_window(width, height, false, title);

		return true;
	}
}
