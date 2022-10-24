#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <chrono>
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
		submit_{&frames_[0]},
		render_{ &frames_[1] }
	{
	}

	Renderer::~Renderer()
	{
		if (use_thread_)
		{
			{
				std::lock_guard<std::mutex> lck(render_mtx_);
				should_terminate_ = true;
			}
			render_cond_.notify_all();
			render_thread_.join();
		}
	}

	bool Renderer::init(RendererType type, uint16_t width, uint16_t height, const std::string& title, bool use_thread) {
		if (type != RendererType::OpenGL) {
			Error("OpenGL only yet!!!");
			return false;
		}

		use_thread_ = use_thread;
		shared_render_context_.reset(new OpenGLRenderContext());
		shared_render_context_->create_window(width, height, false, title);

		if (use_thread_)
		{
			render_thread_ = std::thread(&Renderer::renderThread, this);
		}
		else
		{
			shared_render_context_->start_rendering();
		}

		return true;
	}

	glm::ivec2 Renderer::getFramebufferSize() const
	{
		return shared_render_context_->get_window_size();
	}

	VertexBufferHandle Renderer::createVertexBuffer(uint32_t size, BufferUsage usage, Memory data)
	{
		VertexBufferHandle handle = vertex_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateVertexBuffer{ handle, std::move(data), size, usage });

		return handle;
	}

	IndexBufferHandle Renderer::createIndexBuffer(uint32_t size, BufferUsage usage, Memory data)
	{
		IndexBufferHandle handle = index_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateIndexBuffer{ handle, std::move(data), size, usage });

		return handle;
	}

	ConstantBufferHandle Renderer::createConstantBuffer(uint32_t size, BufferUsage usage, Memory data)
	{
		ConstantBufferHandle handle = constant_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateConstantBuffer{ handle, std::move(data), size, usage });

		return handle;
	}

	TextureHandle Renderer::createTexture2D(uint16_t width, uint16_t height, TextureFormat format, TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, bool srgb, Memory data)
	{
		TextureHandle handle = texture_handle_.next();
		submitPreFrameCommand(cmd::CreateTexture2D{ handle,width,height,format,wrap,minfilter,magfilter,srgb,std::move(data) });

		texture_data_[handle] = TextureData{ width,height,format };

		return handle;
	}

	TextureHandle Renderer::createTextureCubemap(uint16_t width, uint16_t height, TextureFormat format, TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, bool srgb, std::vector<Memory>& data)
	{
		if (data.size() != 6)
		{
			Warning("Cubemap must contains 6 texture image");
			return TextureHandle();
		}

		TextureHandle handle = texture_handle_.next();
		submitPreFrameCommand(cmd::CreateTextureCubeMap{ handle,width,height,format,wrap,minfilter,magfilter,srgb,std::move(data)});

		texture_data_[handle] = TextureData{ width,height,format };

		return handle;
	}

	FrameBufferHandle Renderer::createFrameBuffer(uint16_t width, uint16_t height, TextureFormat format)
	{
		FrameBufferHandle handle = frame_buffer_handle_.next();
		TextureHandle color = createTexture2D(width, height, format, TextureWrap::ClampToEdge, TextureFilter::Linear, TextureFilter::Linear, false, Memory());
		auto textures = std::vector<TextureHandle>{ color };
		submitPreFrameCommand(cmd::CreateFramebuffer{ handle,width,height, textures});
		frame_buffer_textures_.emplace(handle, textures);

		return handle;
	}

	FrameBufferHandle Renderer::createFrameBuffer(std::vector<TextureHandle>& textures)
	{
		if (textures.empty())
		{
			Warning("Framebuffer required one color texture at least");
			return FrameBufferHandle();
		}

		uint16_t w = texture_data_.at(textures[0]).width, h = texture_data_.at(textures[0]).height;

		for (auto& t : textures)
		{
			auto& tdata = texture_data_.at(t);
			if (tdata.width != w || tdata.height != h)
			{
				Warning("Framebuffer textures must be same size");
				return FrameBufferHandle();
			}
		}
		
		FrameBufferHandle handle = frame_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateFramebuffer{ handle,w,h,textures});
		frame_buffer_textures_.emplace(handle, std::move(textures));

		return handle;
	}

	ProgramHandle Renderer::createProgram()
	{
		ProgramHandle handle = program_handle_.next();
		submitPreFrameCommand(cmd::CreateProgram{handle});

		return handle;
	}

	ShaderHandle Renderer::createShader(ShaderStage stage, const std::string& source)
	{
		if (source.empty())
		{
			Warning("Shader source is empty!!!");
			return ShaderHandle();
		}

		ShaderHandle handle = shader_handle_.next();
		submitPreFrameCommand(cmd::CreateShader{handle,stage,source});

		return handle;
	}

	void Renderer::linkProgram(ProgramHandle handle, std::vector<ShaderHandle>& shaders)
	{
		submitPreFrameCommand(cmd::LinkProgram{ handle, shaders });
	}

	void Renderer::updateVertexBuffer(VertexBufferHandle handle, Memory data, uint32_t offset)
	{
		submitPreFrameCommand(cmd::UpdateVertexBuffer{ handle,std::move(data),offset,0 });
	}

	void Renderer::updateIndexBuffer(IndexBufferHandle handle, Memory data, uint32_t offset)
	{
		submitPreFrameCommand(cmd::UpdateIndexBuffer{ handle,std::move(data),offset });
	}

	void Renderer::updateConstantBuffer(ConstantBufferHandle handle, Memory data, uint32_t offset)
	{
		submitPreFrameCommand(cmd::UpdateConstantBuffer{ handle,std::move(data),offset,0 });
	}

	void Renderer::deleteVertexBuffer(VertexBufferHandle handle)
	{
		submitPostFrameCommand(cmd::DeleteVertexBuffer{ handle });

		vertex_buffer_handle_.release(handle);
	}

	void Renderer::deleteIndexBuffer(IndexBufferHandle handle)
	{
		submitPostFrameCommand(cmd::DeleteIndexBuffer{ handle });

		index_buffer_handle_.release(handle);
	}

	void Renderer::deleteConstantBuffer(ConstantBufferHandle handle)
	{
		submitPostFrameCommand(cmd::DeleteConstantBuffer{ handle });

		constant_buffer_handle_.release(handle);
	}

	void Renderer::deleteFrameBuffer(FrameBufferHandle handle)
	{
		submitPostFrameCommand(cmd::DeleteFramebuffer{ handle });
		frame_buffer_handle_.release(handle);

		for (auto& texture_handle : frame_buffer_textures_.at(handle))
		{
			submitPostFrameCommand(cmd::DeleteTexture{ texture_handle });

			texture_data_.erase(texture_handle);
			texture_handle_.release(texture_handle);
		}
		frame_buffer_textures_.erase(handle);
	}

	void Renderer::deleteProgram(ProgramHandle handle)
	{
		submitPostFrameCommand(cmd::DeleteProgram{ handle });

		program_handle_.release(handle);
	}

	void Renderer::deleteShader(ShaderHandle handle)
	{
		submitPostFrameCommand(cmd::DeleteShader{ handle });

		shader_handle_.release(handle);
	}

	void Renderer::deleteTexture(TextureHandle handle)
	{
		submitPostFrameCommand(cmd::DeleteTexture{ handle });

		texture_data_.erase(handle);
		texture_handle_.release(handle);
	}

	void Renderer::setRenderState(StateBits bits)
	{
		submit_->active_item.state_bits = bits;
	}

	void Renderer::setScissorEnable(bool enabled)
	{
		submit_->active_item.scissor = enabled;
	}

	void Renderer::setScissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
	{
		submit_->active_item.scissor_x = x;
		submit_->active_item.scissor_y = y;
		submit_->active_item.scissor_w = w;
		submit_->active_item.scissor_h = h;
	}

	void Renderer::setVertexBuffer(VertexBufferHandle handle)
	{
		submit_->active_item.vb = handle;
		submit_->active_item.vb_offset = 0;
	}

	void Renderer::setIndexBuffer(IndexBufferHandle handle)
	{
		submit_->active_item.ib = handle;
		submit_->active_item.ib_offset = 0;
	}

	void Renderer::setPrimitiveType(PrimitiveType type)
	{
		submit_->active_item.primitive_type = type;
	}

	void Renderer::setTexure(uint16_t unit, TextureHandle handle)
	{
		submit_->active_item.textures[unit].handle = handle;
	}

	void Renderer::setClearBits(unsigned pass, uint16_t value)
	{
		submit_->renderPass(pass).clear_bits = value;
	}

	void Renderer::setFrameBuffer(unsigned pass, FrameBufferHandle handle)
	{
		submit_->renderPass(pass).frame_buffer = handle;
	}

	void Renderer::setConstBuffer(unsigned pass, uint16_t index, ConstantBufferHandle handle)
	{
		submit_->renderPass(pass).constant_buffers[index] = handle;
	}

	void Renderer::setClearColor(unsigned pass, const glm::vec4& value)
	{
		submit_->renderPass(pass).clear_color = value;
	}

	void Renderer::setProgramVar(const std::string& name, int value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, float value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, const glm::vec2& value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, const glm::vec3& value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, const glm::vec4& value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, const glm::mat3& value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, const glm::mat4& value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, const std::vector<float>& value)
	{
		setProgramVar(name, UniformData{ value });
	}

	void Renderer::setProgramVar(const std::string& name, const std::vector<glm::vec4>& value)
	{
		setProgramVar(name, UniformData{ value });
	}


	bool Renderer::frame()
	{

		if (!use_thread_)
		{
			if (!renderFrame(submit_)) {
				Error("render failed!");
				return false;
			}
		}
		else
		{
			// wait previous render to finnish
			{
				std::unique_lock<std::mutex> lck(render_mtx_);
				render_cond_.wait(lck, [this] {return render_done_; });
				std::swap(render_, submit_);
				render_job_submitted_ = true;
				render_done_ = false;
			}
			render_cond_.notify_all();
		}

		return true;
	}

	void Renderer::submitPreFrameCommand(RenderCommand command)
	{
		submit_->commands_pre.emplace_back(std::move(command));
	}

	void Renderer::submitPostFrameCommand(RenderCommand command)
	{
		submit_->commands_post.emplace_back(std::move(command));
	}

	void Renderer::renderThread()
	{
		Info("Rendering thread start... %d", std::this_thread::get_id());

		shared_render_context_->start_rendering();
		while (!should_terminate_)
		{
			std::unique_lock<std::mutex> lck(render_mtx_);

			render_done_ = true;
			render_cond_.notify_all();

			render_cond_.wait(lck, [this] {return should_terminate_ || render_job_submitted_; });

			render_job_submitted_ = false;
			render_done_ = false;

			lck.unlock();

			if (!renderFrame(render_))
			{
				should_terminate_ = true;
			}
		}
		shared_render_context_->stop_rendering();
		Info("Rendering thread stopped!");
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
	void Renderer::setProgramVar(const std::string& name, UniformData data)
	{
		submit_->active_item.uniforms[name] = data;
	}
	void Renderer::submit(uint32_t pass, ProgramHandle program)
	{
		submit(pass, program, 0);
	}
	void Renderer::submit(uint32_t pass, ProgramHandle program, uint32_t vertex_count)
	{
		submit(pass, program, vertex_count, 0, 0);
	}
	void Renderer::submit(uint32_t pass, ProgramHandle program, uint32_t vertex_count, uint32_t vb_offset, uint32_t ib_offset)
	{
		auto& item = submit_->active_item;
		item.program = program;
		item.vertex_count = vertex_count;
		item.ib_offset = ib_offset;
		item.vb_offset = vb_offset;

		submit_->renderPass(pass).render_items.emplace_back(std::move(item));
		item = RenderItem();
	}

	bool Renderer::renderFrame(Frame* frame)
	{
		shared_render_context_->process_command_list(frame->commands_pre);

		if (!shared_render_context_->frame(frame))
		{
			return false;
		}

		shared_render_context_->process_command_list(frame->commands_post);

		frame->active_item = RenderItem();
		for (auto& pass : frame->render_passes)
		{
			pass.clear();
		}

		frame->renderPass(0).frame_buffer = FrameBufferHandle{ 0 };
		frame->commands_pre.clear();
		frame->commands_post.clear();

		return true;
	}
	void Renderer::waitForFrameEnd()
	{
		if (use_thread_)
		{
			std::unique_lock<std::mutex> lck(render_mtx_);
			render_cond_.wait(lck, [this] {return render_done_; });
		}
	}
}
