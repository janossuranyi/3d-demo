#include "engine/engine.h"
#include "logger.h"
#include "effect_test_engine.h"

using namespace rc;

static float UNIT_BOX_POSITIONS[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

#define KERNEL_BLUR 0
#define KERNEL_BOTTOM_SOBEL 1
#define KERNEL_IDENTITY 2
#define KERNEL_EMBOSS 3
#define KERNEL_LEFT_SOBEL 4
#define KERNEL_OUTLINE 5
#define KERNEL_RIGHT_SOBEL 6
#define KERNEL_SHARPEN 7
#define KERNEL_TOP_SOBEL 8

EngineTestEffect::~EngineTestEffect()
{
	gfx::Renderer* hwr = ctx::Context::default()->hwr();
	hwr->deleteProgram(prgPoints);
	hwr->deleteProgram(prgPP);
	hwr->deleteProgram(prgSkybox);
	hwr->deleteProgram(prgComp);
	hwr->deleteProgram(prgDepth);
	hwr->deleteProgram(prgGauss);
	hwr->deleteProgram(prgViewTex);
	hwr->deleteFrameBuffer(fb);
	hwr->deleteBuffer(tmp);

	hwr->deleteBuffer(vb_points);
	hwr->deleteBuffer(vb_skybox);
	hwr->deleteTexture(skybox);
	hwr->deleteTexture(texDyn);
	hwr->deleteTexture(depth_attachment);
	hwr->deleteTexture(color_attachment);

	hwr->frame();

	ctx::Context::default()->vertexCache()->resetStaticBufferSet();

//	renderer.waitForFrameEnd();
}

#define CACHE_SIZE (128 * (1<<20))

bool EngineTestEffect::Init()
{
	gfx::TextureManager* tm = ctx::Context::default()->textureManager();
	gfx::Renderer* hwr = ctx::Context::default()->hwr();
	gfx::ShaderManager* sm = ctx::Context::default()->shaderManager();
	gfx::VertexCache& vtx_cache = *ctx::Context::default()->vertexCache();

	//sm->setVersionString("#version 450 core\n");

	glm::ivec2 win_size = hwr->getFramebufferSize();

	texDyn =
		hwr->createTexture2D(gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear,
			gfx::ImageSet::create(512, 512, gfx::TextureFormat::RGBA8));
	depth_attachment =
		hwr->createTexture2D(gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear,
			gfx::ImageSet::create(win_size.x, win_size.y, gfx::TextureFormat::D24S8));
	color_attachment =
		hwr->createTexture2D(gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear,
			gfx::ImageSet::create(win_size.x, win_size.y, gfx::TextureFormat::RGBA8));
    fb = 
		hwr->createFrameBuffer(std::vector<gfx::FramebufferTexture>{ {color_attachment, 0, 0}}, depth_attachment);


	tmp = hwr->createVertexBuffer(256, gfx::Memory(Vector<float>{0.5f, 0.0f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f}));

	Vector<String> const skybox_set{
		rc::ResourceManager::get_resource("textures/cubemaps/skybox/right.jpg"),
		rc::ResourceManager::get_resource("textures/cubemaps/skybox/left.jpg"),
		rc::ResourceManager::get_resource("textures/cubemaps/skybox/top.jpg"),
		rc::ResourceManager::get_resource("textures/cubemaps/skybox/bottom.jpg"),
		rc::ResourceManager::get_resource("textures/cubemaps/skybox/front.jpg"),
		rc::ResourceManager::get_resource("textures/cubemaps/skybox/back.jpg")
	};

	auto cube_images = gfx::ImageSet::fromFiles(skybox_set);

//	skybox = tm->createFromResource("textures/cubemaps/skybox.ktx2");

	skybox = hwr->createTextureCubemap(gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear, cube_images, true);
	

	{
		const size_t bufSize = sizeof(DrawVert) * NUMPOINTS;
		std::vector<DrawVert> buffer(NUMPOINTS);

		const int n = 1000, n2 = n / 2; // particles spread in the cube

		float r0[4];

		for (unsigned i = 0; i < NUMPOINTS; ++i)
		{
			r0[0] = static_cast<float>(rand() % n - n2);
			r0[1] = static_cast<float>(rand() % n - n2);
			r0[2] = static_cast<float>(rand() % n - n2);
			r0[3] = 1.0f;
			buffer[i].set_position(r0);
			r0[0] = (r0[0] / n) + 0.5f;
			r0[1] = (r0[1] / n) + 0.5f;
			r0[2] = (r0[2] / n) + 0.5f;
			r0[3] = 1.0f;
			buffer[i].set_color(r0);
		}


		vc_points = vtx_cache.allocStaticVertex(gfx::Memory(buffer));

//		vb_points = renderer.createVertexBuffer(bufSize, gfx::BufferUsage::Static, gfx::Memory(buffer));
	}

	{
		const size_t vert_count = (sizeof(UNIT_BOX_POSITIONS) / sizeof(float) / 3);
		std::vector<DrawVert> buffer(vert_count);

		float r[4]{ 0.f,0.f,0.f,1.f };

		for (int i = 0; i < vert_count; ++i)
		{
			r[0] = UNIT_BOX_POSITIONS[i * 3 + 0];
			r[1] = UNIT_BOX_POSITIONS[i * 3 + 1];
			r[2] = UNIT_BOX_POSITIONS[i * 3 + 2];
			r[3] = 1.0f;
			buffer[i].set_position(r);
		}

		vc_skybox = vtx_cache.allocStaticVertex(gfx::Memory(buffer));
		//vb_skybox = renderer.createVertexBuffer(bufSize, gfx::BufferUsage::Static, gfx::Memory(buffer));
	}

	prgComp = sm->createProgram(gfx::ComputeProgram{ "Compute01",
		"shaders/test_compute2.cs.glsl" , {{"LOCAL_SIZE_X", "8"},{"LOCAL_SIZE_Y","8"}} });
		prgViewTex	= sm->createProgram(gfx::RenderProgram{ "Rect",
			"shaders/fullscreen_rect.vs.glsl",
			"shaders/test_compute2.fs.glsl",{} });
		prgPoints	= sm->createProgram(gfx::RenderProgram{ "Draw_Points",
			"shaders/draw_point2.vs.glsl",
			"shaders/draw_point2.fs.glsl",{} });	
		prgSkybox	= sm->createProgram(gfx::RenderProgram{ "SkyBox",
			"shaders/skybox2.vs.glsl",
			"shaders/skybox2.fs.glsl" ,{} });
		prgPP		= sm->createProgram(gfx::RenderProgram{ "Kernel_Filter",
			"shaders/fullscreen_rect.vs.glsl",
			"shaders/kernel2.fs.glsl" ,{} });
		prgDepth	= sm->createProgram(gfx::RenderProgram{ "DepthViewer",
			"shaders/fullscreen_rect.vs.glsl",
			"shaders/view_depthbuf.fs.glsl" ,{} });
		prgGauss	= sm->createProgram(gfx::RenderProgram{ "GaussianBlur",
			"shaders/fullscreen_rect.vs.glsl",
			"shaders/gauss_filter.fs.glsl" ,{} });


	eyeZ = 1200.0f;
	glm::vec3 viewPos{ 0, 0, eyeZ };

	glm::mat4 V = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	P = glm::perspective(45.0f, (float)win_size.x / win_size.y, 1.0f, 1700.0f);

	VP = P * V;

	layout.begin()
		.add(gfx::AttributeName::Position,	gfx::AttributeType::Float,	3, false,	sizeof(DrawVert))
		.add(gfx::AttributeName::TexCoord0, gfx::AttributeType::Half,	2, false,	sizeof(DrawVert))
		.add(gfx::AttributeName::Normal,	gfx::AttributeType::UByte,	4, true,	sizeof(DrawVert))
		.add(gfx::AttributeName::Tangent,	gfx::AttributeType::UByte,	4, true,	sizeof(DrawVert))
		.add(gfx::AttributeName::Color0,	gfx::AttributeType::UByte,	4, true,	sizeof(DrawVert))
		.add(gfx::AttributeName::TexCoord1, gfx::AttributeType::Float,	1, false,	sizeof(DrawVert),	false)
		.switch_buffer()
		.add(gfx::AttributeName::TexCoord2, gfx::AttributeType::UInt,	1, false,	4,					true, 1, 1)
		.end();

	layout_handle = hwr->createVertexLayout(layout);

	Vector<vec4> tbData;
	for (uint i = 0; i < 256; ++i)
	{
		tbData.emplace_back((float)i / 256.0f);
	}
	texBuf = hwr->createTextureBuffer(256 * sizeof(vec4), gfx::BufferUsage::Dynamic, gfx::Memory(tbData));
	bufTex = hwr->createBufferTexture(texBuf, gfx::TextureFormat::RGBA32F);

    return true;
}

bool EngineTestEffect::Update(float time)
{
	angle += 0.1f * time;
	rotY += time * 0.01f;
	rotX += time * 0.02f;
	rotX = std::fmodf(rotX, 360.0f);
	rotY = std::fmodf(rotY, 360.0f);

	return true;
}

bool EngineTestEffect::HandleEvent(const SDL_Event* ev, float time)
{
	bool mustUpdate = false;

	if (ev->type == SDL_KEYDOWN)
	{
		switch (ev->key.keysym.sym)
		{
		case SDLK_w:
			eyeZ -= 5.0f;
			mustUpdate = true;
			break;
		case SDLK_s:
			eyeZ += 5.0f;
			mustUpdate = true;
			break;
		case SDLK_x:
			if (pp_offset >= 0.0005) pp_offset -= 0.0001;
			break;
		case SDLK_c:
			pp_offset += 0.0001;
			break;
		case SDLK_0:
			kernel = 0;
			break;
		case SDLK_1:
			kernel = 1;
			break;
		case SDLK_2:
			kernel = 2;
			break;
		case SDLK_3:
			kernel = 3;
			break;
		case SDLK_4:
			kernel = 4;
			break;
		case SDLK_5:
			kernel = 5;
			break;
		case SDLK_6:
			kernel = 6;
			break;
		case SDLK_7:
			kernel = 7;
			break;
		case SDLK_8:
			kernel = 8;
			break;
		case SDLK_SPACE:
			return false;
		}
	}

	if (mustUpdate)
	{
		glm::vec3 viewPos{ 0, 0, eyeZ };

		glm::mat4 V = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		VP = P * V;
	}

	//SDL_Log("ev.type: %d, mouseX: %d, mouseY: %d", ev->type, ev->motion.x, ev->motion.y);

	return true;
}

bool EngineTestEffect::Render(uint64_t frame)
{
	gfx::Renderer* hwr = ctx::Context::default()->hwr();
	gfx::VertexCache& vtx_cache = *ctx::Context::default()->vertexCache();

	uint16_t pass = 0;
	gfx::FenceHandle fence;
	
	vtx_cache.frame();
	const uint active_frame = hwr->getFrameNum() & 1;

	hwr->beginCompute();
	{
		effect1_vars["g_fAngle"] = angle;
		effect1_vars["g_fFa"] = 0.7f;
		//fence = renderer.createFence();
		hwr->setComputeJob(glm::ivec3(512 / 8, 512 / 8, 1), gfx::FenceHandle{});
		hwr->setImageTexture(0, texDyn, 0, false, 0, gfx::Access::Write, gfx::TextureFormat::RGBA8);
		hwr->setUniforms(effect1_vars);
		hwr->submit(pass, prgComp);
		hwr->endCompute();
	}

	glm::mat4 W(1.0f);
	W = glm::rotate(W, glm::radians(rotX), glm::vec3(1, 0, 0));
	W = glm::rotate(W, glm::radians(rotY), glm::vec3(0, 1, 0));

	glm::mat4 W2(1.0f);
	W2 = glm::rotate(W2, glm::radians(rotX+90.0f), glm::vec3(1, 0, 0));
	W2 = glm::rotate(W2, glm::radians(rotY+90.0f), glm::vec3(0, 1, 0));
	//const glm::mat4 WVP = VP * W;

	uint32_t count, offs;
	vb_points = vtx_cache.getVertexBuffer<DrawVert>(vc_points, offs, count);

	effect2_vars["g_mViewProjectionTransform"] = VP;
	effect2_vars["g_mWorldTransform"] = W;
	effect2_vars["g_mWorldTransform2"] = W2;

	hwr->setClearBits(pass, gfx::GLS_CLEAR_COLOR | gfx::GLS_CLEAR_DEPTH);
	hwr->setFrameBuffer(pass, fb);
	hwr->setPrimitiveType(gfx::PrimitiveType::Point);
	hwr->setRenderState(gfx::GLS_DEPTHFUNC_LESS);
	hwr->setUniforms(effect2_vars);
	hwr->setVertexDecl(layout);
	hwr->setInstanceCount(2);
	hwr->setVertexBuffer(vb_points);
	hwr->setVertexBuffer(tmp, 1, 0);

	hwr->submit(pass, prgPoints, count, offs, 0, gfx::IndexBufferType::U16);

	W = glm::rotate(glm::mat4(1), glm::radians(rotY), glm::vec3(0, 1, 0));
	const glm::mat4 sky_view = glm::mat4(glm::mat3(W));

	++pass;

	vb_skybox = vtx_cache.getVertexBuffer<DrawVert>(vc_skybox, offs, count);

	effect3_vars["m_P"] = P;
	effect3_vars["m_V"] = sky_view;
	effect3_vars["samp0"] = 0;
	effect3_vars["g_vData"] = 1;

	hwr->setClearBits(pass, 0);
	hwr->setFrameBuffer(pass, fb);
	hwr->setRenderState(gfx::GLS_DEPTHFUNC_LESS|gfx::GLS_DEPTHMASK);
	hwr->setPrimitiveType(gfx::PrimitiveType::Triangles);
	hwr->setVertexDecl(layout);
	hwr->setVertexAttribs(attrs);
	hwr->setVertexBuffer(vb_skybox);
	hwr->setUniforms(effect3_vars);
	hwr->setTexture(skybox, 0);
	hwr->setTexture(bufTex, 1);
	hwr->submit(pass, prgSkybox, count, offs, 0, gfx::IndexBufferType::U16);

	++pass;
	//renderer.setClearBits(pass, gfx::GLS_CLEAR_COLOR | gfx::GLS_CLEAR_DEPTH);
	effect4_vars["g_tInput"] = 0;
	effect4_vars["g_fOffset"] = pp_offset;
	effect4_vars["g_iKernel"] = kernel;
	hwr->setFrameBuffer(pass, gfx::FrameBufferHandle{0});
	hwr->setRenderState(gfx::GLS_DEPTHFUNC_ALWAYS|gfx::GLS_DEPTHMASK);
	hwr->setPrimitiveType(gfx::PrimitiveType::Triangles);
	hwr->setTexture(color_attachment);
	hwr->setVertexBuffer(gfx::BufferHandle{});
	hwr->setVertexDecl(layout);
	hwr->setUniforms(effect4_vars);
	hwr->submit(pass, prgPP, 6, 0, 0, gfx::IndexBufferType::U16);

	++pass;
	hwr->setClearBits(pass, 0);
	hwr->setFrameBuffer(pass, gfx::FrameBufferHandle{ 0 });
	hwr->setRenderState(gfx::GLS_DEPTHFUNC_ALWAYS | gfx::GLS_DEPTHMASK);
	hwr->setPrimitiveType(gfx::PrimitiveType::Triangles);
	hwr->setVertexDecl(layout);
	hwr->setTexture(depth_attachment);
	float scale = 0.2f;
	glm::mat4 _w = glm::translate(glm::mat4(1.0f), glm::vec3( (1.0f - scale), (1.0f - scale), 0.0f));
	_w = glm::scale(_w, glm::vec3(scale));
	effect5_vars["g_mWorldTransform"] = _w;
	effect5_vars["g_fFarPlane"] = 1700.0f;
	effect5_vars["samp0"] = 0;
	hwr->setUniforms(effect5_vars);
	hwr->submit(pass, prgDepth, 6, 0, 0, gfx::IndexBufferType::U16);


	++pass;
	hwr->setClearBits(pass, 0);
	hwr->setFrameBuffer(pass, gfx::FrameBufferHandle{ 0 });
	hwr->setRenderState(gfx::GLS_DEPTHFUNC_ALWAYS | gfx::GLS_DEPTHMASK);
	hwr->setPrimitiveType(gfx::PrimitiveType::Triangles);
	hwr->setTexture(texDyn);
	hwr->setVertexDecl(layout);
	hwr->setVertexBuffer(gfx::BufferHandle{});
	scale = 0.2f;
	_w = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f + scale, -1.0f + scale, 0.0f));
	_w = glm::scale(_w, glm::vec3(scale));
	effect6_vars["g_tInput"] = 0;
	effect6_vars["g_mWorldTransform"] = _w;
	hwr->setUniforms(effect6_vars);

	hwr->beginCompute();
	{
		hwr->MemoryBarrier(gfx::barrier::ShaderImageAccess);
		hwr->submit(pass);
		hwr->endCompute();
	}
	
	//renderer.deleteFence(fence);
	hwr->submit(pass, prgViewTex, 6, 0, 0, gfx::IndexBufferType::U16);

	return true;
}