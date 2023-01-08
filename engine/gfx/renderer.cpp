#include "gfx/renderer.h"
#include "gfx/vertex.h"
#include "gfx/opengl/gl_context.h"

#include "logger.h"

#include <execution>

namespace gfx {

	RenderPass::RenderPass() :
		render_area{},
		clear_color{ 0.0f,0.0f,0.0f,1.0f },
		clear_bits{ GLS_CLEAR_COLOR | GLS_CLEAR_DEPTH },
		frame_buffer{ FrameBufferHandle::invalid } {
	}

	void RenderPass::clear() {
		clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		render_items.clear();
		compute_items.clear();
		frame_buffer = FrameBufferHandle::invalid;
	}

	bool RenderPass::isRenderAreaValid() const
	{
		return render_area.size.x > 0 && render_area.size.y > 0 && render_area.size.x < render_area.offset.x&& render_area.size.y < render_area.offset.y;
	}

	Renderer::Renderer() :
		framenum_{0},
		width_{0},
		height_{0},
		window_title_{"JS-Engine"},
		use_thread_{false},
		compute_active_{false},
		submit_{&frames_[0]},
		render_{ &frames_[1] }
	{
#ifdef _DEBUG
		Info(" sizeof(ComputeItem)=%d", sizeof(ComputeItem));
		Info(" sizeof(RenderItem)=%d", sizeof(RenderItem));
		Info(" sizeof(vbs)=%d", sizeof(RenderItem::vbs));
		Info(" sizeof(textures)=%d", sizeof(RenderItem::textures));
		Info("alignof(RenderItem)=%d", alignof(RenderItem));
#endif
	}

	Renderer::~Renderer()
	{
		if (use_thread_ && render_thread_.joinable())
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

		width_ = width;
		height_ = height;
		window_title_ = title;

		use_thread_ = use_thread;
		shared_render_context_.reset(new OpenGLRenderContext());

		if (!shared_render_context_->create_window(width, height, false, title))
		{
			return false;
		}


		if (use_thread_)
		{
			render_thread_ = std::thread(&Renderer::renderThread, this);
		}
		else
		{
			shared_render_context_->start_rendering();
		}

		defaultVertexDecl_.begin()
			.add(AttributeName::Position, AttributeType::Float, 3, false, 32, true)
			.add(AttributeName::TexCoord0, AttributeType::Float, 2, false, 32, true)
			.add(AttributeName::Normal, AttributeType::Float, 3, false, 32, true)
			.end();

		
		createVertexLayout(defaultVertexDecl_);

		return true;
	}

	int Renderer::getUniformOffsetAligment() const
	{
		return shared_render_context_->uniform_offset_aligment();
	}

	VertexDecl const* Renderer::defaultVertexDecl() const
	{
		return &defaultVertexDecl_;
	}

	QueryResult Renderer::getQueryResult(QueryHandle handle)
	{
		return shared_render_context_->get_query_result(handle);
	}

	QueryHandle Renderer::getMappedBufferAddress(int cbCount, const ConstantBufferHandle* handles)
	{
		QueryHandle res{};

		if (cbCount <= 0) return res;
		res = query_handle_.next();

		cmd::QueryMappedBufferAddresses cmd{};
		cmd.constantBufferHandles.assign(handles, handles + cbCount);
		cmd.handle = res;
		submitPostFrameCommand(cmd);

		return res;
	}

	glm::ivec2 Renderer::getFramebufferSize() const
	{
		return shared_render_context_->get_window_size();
	}

	VertexLayoutHandle Renderer::createVertexLayout(VertexDecl& decl)
	{
		const VertexLayoutHandle handle = vertex_layout_handle_.next();
		submitPreFrameCommand(cmd::CreateVertexLayout{ handle, decl });
		decl.setHandle(handle);

		return handle;
	}

	TextureBufferHandle Renderer::createTextureBuffer(uint size, BufferUsage usage, Memory data)
	{
		const TextureBufferHandle handle = texture_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateTextureBuffer{ handle,std::move(data),size,usage });

		return handle;
	}

	VertexBufferHandle Renderer::createVertexBuffer(uint32_t size, BufferUsage usage, Memory data)
	{
		const VertexBufferHandle handle = vertex_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateVertexBuffer{ handle, std::move(data), size, usage });

		return handle;
	}

	IndexBufferHandle Renderer::createIndexBuffer(uint32_t size, BufferUsage usage, Memory data)
	{
		const IndexBufferHandle handle = index_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateIndexBuffer{ handle, std::move(data), size, usage });

		return handle;
	}

	ShaderStorageBufferHandle gfx::Renderer::createShaderStorageBuffer(uint size, BufferUsage usage, Memory data)
	{
		const  ShaderStorageBufferHandle handle = shader_storage_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateShaderStorageBuffer{ handle, std::move(data), size, usage });

		return handle;
	}

	ConstantBufferHandle Renderer::createConstantBuffer(uint32_t size, BufferUsage usage, Memory data)
	{
		const ConstantBufferHandle handle = constant_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateConstantBuffer{ handle, std::move(data), size, usage });

		return handle;
	}

	TextureHandle Renderer::createTexture2D(TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, ImageSet& data, ushort max_iso)
	{
		const TextureHandle handle = texture_handle_.next();

		texture_data_[handle] = TextureData{ data[0].width,data[0].height,data.format()};
		submitPreFrameCommand(cmd::CreateTexture2D{ handle,wrap,minfilter,magfilter,std::move(data),false,max_iso });


		return handle;
	}

	TextureHandle Renderer::createTexture(TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, String const& apath, bool srgb, bool auto_mipmap, bool compress, ushort aQuality, ushort max_aniso)
	{
		const TextureHandle handle = texture_handle_.next();

		texture_data_[handle] = TextureData{ 0,0,TextureFormat::RGBA8 };
		const ushort flags = (ushort)((compress << 2) | (auto_mipmap << 1) | srgb);
		submitPreFrameCommand(cmd::CreateTexture{ handle,wrap,minfilter,magfilter,aQuality,apath,flags,max_aniso });

		return handle;
	}

	TextureHandle Renderer::createTextureCubemap(TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, Vector<ImageSet>& data, bool compress)
	{
		if (!data.size())
		{
			Warning("Cubemap must contains 1 texture image");
			return TextureHandle();
		}

		const TextureHandle handle = texture_handle_.next();
		const ImageSet& l0 = data.at(0);
		texture_data_[handle] = TextureData{ l0[0].width, l0[0].height, l0.format()};

		submitPreFrameCommand(cmd::CreateTextureCubeMap{ handle,wrap,minfilter,magfilter,std::move(data),compress });


		return handle;
	}

	TextureHandle Renderer::createBufferTexture(TextureBufferHandle hbuffer, TextureFormat format, uint offset, uint size)
	{
		TextureHandle handle = texture_handle_.next();
		submitPreFrameCommand(cmd::CreateBufferTexture{ hbuffer,handle,format,offset,size });

		return handle;
	}

	FrameBufferHandle Renderer::createFrameBuffer(uint16_t width, uint16_t height, TextureFormat format)
	{
		const FrameBufferHandle handle = frame_buffer_handle_.next();
		
		const TextureHandle color = createTexture2D(TextureWrap::ClampToEdge, TextureFilter::Linear, TextureFilter::Linear, gfx::ImageSet::create(width,height,format));
		submitPreFrameCommand(cmd::CreateFramebuffer{ handle,width,height,TextureHandle(), std::vector<FramebufferTexture>{ {color,0,0} } });
		frame_buffer_textures_.emplace(handle, std::vector<FramebufferTexture>{ {color, 0, 0} });

		return handle;
	}

	FrameBufferHandle Renderer::createFrameBuffer(std::vector<FramebufferTexture>& textures, TextureHandle depth_texture)
	{
		if (textures.empty())
		{
			Warning("Framebuffer required one color texture at least");
			return FrameBufferHandle();
		}

		uint16_t w = texture_data_.at(textures[0].handle).width, h = texture_data_.at(textures[0].handle).height;

		for (auto& t : textures)
		{
			auto& tdata = texture_data_.at(t.handle);
			if (tdata.width != w || tdata.height != h)
			{
				Warning("Framebuffer textures must be same size");
				return FrameBufferHandle();
			}
		}
		
		FrameBufferHandle handle = frame_buffer_handle_.next();
		submitPreFrameCommand(cmd::CreateFramebuffer{ handle,w,h,depth_texture,textures });
		if (depth_texture.isValid())
		{
			textures.push_back({ depth_texture,0,0 });
		}

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

	FenceHandle Renderer::createFence()
	{
		FenceHandle handle = fence_handle_.next();
		//submitPostFrameCommand(cmd::CreateFence{handle});

		return handle;
	}

	void Renderer::linkProgram(ProgramHandle handle, std::vector<ShaderHandle>& shaders)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPreFrameCommand(cmd::LinkProgram{ handle, shaders });
	}

	void Renderer::updateVertexBuffer(VertexBufferHandle handle, Memory data, uint32_t offset)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPreFrameCommand(cmd::UpdateVertexBuffer{ handle,std::move(data),offset,0 });
	}

	void Renderer::updateIndexBuffer(IndexBufferHandle handle, Memory data, uint32_t offset)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPreFrameCommand(cmd::UpdateIndexBuffer{ handle,std::move(data),offset });
	}

	void Renderer::updateConstantBuffer(ConstantBufferHandle handle, Memory data, uint offset)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPreFrameCommand(cmd::UpdateConstantBuffer{ handle,data,offset,0 });
	}

	void Renderer::updateShaderStorageBuffer(ShaderStorageBufferHandle handle, Memory data, uint offset)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPreFrameCommand(cmd::UpdateShaderStorageBuffer{ handle,data,offset });
	}

	void Renderer::updateTextureBuffer(TextureBufferHandle handle, Memory data, uint offset)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPreFrameCommand(cmd::UpdateTextureBuffer{ handle,std::move(data),offset,0 });
	}

	void Renderer::deleteVertexLayout(VertexLayoutHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteVertexLayout{ handle });
	}

	void Renderer::deleteVertexBuffer(VertexBufferHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteVertexBuffer{ handle });
	}

	void Renderer::deleteIndexBuffer(IndexBufferHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteIndexBuffer{ handle });
	}

	void Renderer::deleteConstantBuffer(ConstantBufferHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteConstantBuffer{ handle });
	}

	void Renderer::deleteShaderStorageBuffer(ShaderStorageBufferHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteShaderStorageBuffer{ handle });
	}

	void Renderer::deleteFrameBuffer(FrameBufferHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}

		submitPostFrameCommand(cmd::DeleteFramebuffer{ handle });

		for (auto& texture_handle : frame_buffer_textures_.at(handle))
		{
			submitPostFrameCommand(cmd::DeleteTexture{ texture_handle.handle });

			texture_data_.erase(texture_handle.handle);
			texture_handle_.release(texture_handle.handle);
		}
		frame_buffer_textures_.erase(handle);
	}

	void Renderer::deleteProgram(ProgramHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteProgram{ handle });
	}

	void Renderer::deleteShader(ShaderHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteShader{ handle });
	}

	void Renderer::deleteTexture(TextureHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteTexture{ handle });
		texture_data_.erase(handle);
	}

	void Renderer::deleteFence(FenceHandle handle)
	{
		if (!handle.isValid())
		{
			return;
		}
		submitPostFrameCommand(cmd::DeleteFence{ handle });
	}

	void Renderer::WaitSync(FenceHandle handle, bool client, uint64_t timeout)
	{
		submit_->active_compute.fence = handle;
		submit_->active_compute.wait_sync_client = client;
		submit_->active_compute.wait_timeout = timeout;
	}

	void Renderer::MemoryBarrier(uint32_t barrier_bits)
	{
		submit_->active_compute.barrier_bits = barrier_bits;
	}

	void Renderer::setComputeJob(glm::ivec3 num_groups, FenceHandle fence)
	{
		submit_->active_compute.num_groups_x = num_groups.x;
		submit_->active_compute.num_groups_y = num_groups.y;
		submit_->active_compute.num_groups_z = num_groups.z;
		submit_->active_compute.fence = fence;
	}

	void Renderer::setImageTexture(uint16_t unit, TextureHandle handle, uint16_t level, bool layered, uint16_t layer, Access access, TextureFormat format)
	{
		assert(unit < MAX_IMAGE_UNITS);
		submit_->active_compute.images[unit] = ImageBinding{ handle,level,layered,layer,access,format };
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

	void Renderer::setIndexBuffer(IndexBufferHandle handle)
	{
		submit_->active_item.ib = handle;
		submit_->active_item.ib_offset = 0;
	}

	void Renderer::setPrimitiveType(PrimitiveType type)
	{
		submit_->active_item.primitive_type = type;
	}

	void Renderer::setInstanceCount(uint count)
	{
		submit_->active_item.instance_count = count;
	}

	void Renderer::setTexture(TextureHandle handle, uint16_t unit)
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

	void Renderer::setConstBuffer(uint16_t index, ConstantBufferHandle handle, uint offset, uint size)
	{
		if (compute_active_)
		{

		}
		else
		{
			submit_->active_item.constant_buffers[index] = ConstantBufferBinding{ handle,offset,size };
		}
	}

	void Renderer::setViewport(unsigned pass, ushort x, ushort y, ushort width, ushort height)
	{
		submit_->renderPass(pass).render_area = Rect2D{ {x,y},{width,height} };
	}

	void Renderer::setClearColor(unsigned pass, const glm::vec4& value)
	{
		submit_->renderPass(pass).clear_color = value;
	}

	void Renderer::setVertexDecl(const VertexDecl& vertDecl)
	{
		submit_->active_item.vertexDecl = vertDecl;
	}
	void gfx::Renderer::setVertexAttribs(const VertexAttribMap& attribs)
	{
		submit_->active_item.vertexAttribs = attribs;
	}

	void Renderer::setUniforms(UniformMap& uniforms)
	{
		if (compute_active_) submit_->active_compute.uniforms = std::move(uniforms);
		else submit_->active_item.uniforms = std::move(uniforms);
	}

	void gfx::Renderer::beginCompute()
	{
		compute_active_ = true;
	}

	void gfx::Renderer::endCompute()
	{
		compute_active_ = false;
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
				render_cond_.wait(lck, [this] {return render_done_ || should_terminate_; });

				if (should_terminate_) 
				{ 
					return false; 
				}

				std::swap(render_, submit_);
				render_job_submitted_ = true;
				render_done_ = false;
			}
			render_cond_.notify_all();
		}

		++framenum_;

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

			// waiting for the next 
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
		render_cond_.notify_all();
		Info("Rendering thread stopped!");
	}
	
	Frame::Frame()
	{
		active_item = RenderItem();
		active_compute = ComputeItem();
		render_passes.assign(4, RenderPass());
	}

	RenderPass& Frame::renderPass(uint32_t index)
	{
		const size_t size = render_passes.size();
		if (size <= index) {
			render_passes.resize(2 * size > index ? 2 * size : index + 1);
		}

		return render_passes[index];
	}

	void Renderer::setVertexBuffer(VertexBufferHandle vb, ushort index, uint offset)
	{
		submit_->active_item.vbs[index] = VertexBinding{ vb,offset };
	}

	void gfx::Renderer::submit(uint32_t pass)
	{
		submit(pass, ProgramHandle());
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
		if (!compute_active_)
		{
			auto& item = submit_->active_item;
			item.program = program;
			item.vertex_count = vertex_count;
			item.ib_offset = ib_offset;
			item.vb_offset = vb_offset;
			submit_->renderPass(pass).render_items.emplace_back(std::move(item));
			item = RenderItem();
		}
		else
		{
			auto& item = submit_->active_compute;
			item.program = program;
			submit_->renderPass(pass).compute_items.emplace_back(std::move(item));
			item = ComputeItem();
		}

	}

	bool Renderer::renderFrame(Frame* frame)
	{
		shared_render_context_->process_command_list(frame->commands_pre);

		// sort render items
		for (auto& pass : frame->render_passes)
		{
			if (pass.render_items.size() > 2)
			{
				std::sort(std::execution::par, pass.render_items.begin(), pass.render_items.end());
			}
		}

		if (!shared_render_context_->frame(frame))
		{
			return false;
		}

		shared_render_context_->process_command_list(frame->commands_post);

		frame->active_item = RenderItem{};
		frame->active_compute = ComputeItem{};
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
