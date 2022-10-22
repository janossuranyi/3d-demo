#include "renderer.h"
#include "logger.h"
#include "gl_context.h"

namespace gfx {

	void RenderPass::clear() {
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

	bool Renderer::renderFrame(Frame* frame)
	{
		shared_render_context_->process_command_list(frame->commands_pre);
		if (!shared_render_context_->frame(frame)) {
			return false;
		}
		shared_render_context_->process_command_list(frame->commands_post);

		frame->commands_post.clear();
		frame->commands_pre.clear();
		frame->active_item = RenderItem();
		for (auto& pass : frame->render_passes) {
			pass.clear();
		}

		frame->renderPass(0).frame_buffer = FrameBufferHandle{ 0 };
	}

	void Renderer::submitPreFrameCommand(RenderCommand command)
	{
		submit_->commands_pre.emplace_back(std::move(command));
	}

	void Renderer::submitPostFrameCommand(RenderCommand command)
	{
		submit_->commands_post.emplace_back(std::move(command));
	}
	
	Frame::Frame()
	{
		active_item = RenderItem();
		render_passes.assign(4, RenderPass());
	}

	RenderPass& Frame::renderPass(uint32_t index)
	{
		const size_t size = render_passes.size();
		if (size <= index) {
			render_passes.resize(size * 2);
		}

		return render_passes[index];
	}
}
