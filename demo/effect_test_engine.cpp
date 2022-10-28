#include <string>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "logger.h"
#include "effect_test_engine.h"
#include "engine/gfx/gfx.h"


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

static const float UNIT_RECT_WITH_ST[6 * 4] = {
	-1.0f, 1.0,/**/0.0f, 1.0f,
	-1.0f,-1.0f,/**/0.0f,0.0f,
	1.0f,-1.0f,/**/1.0f,0.0f,
	1.0f,1.0f,/**/1.0f,1.0f,
	-1.0f,1.0f,/**/0.0f,1.0f,
	1.0f,-1.0f,/**/1.0f,0.0f
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
	renderer.deleteProgram(prgPoints);
	renderer.deleteProgram(prgPP);
	renderer.deleteProgram(prgSkybox);
	renderer.deleteFrameBuffer(fb);
	
	renderer.deleteVertexBuffer(vb_points);
	renderer.deleteVertexBuffer(vb_pp);
	renderer.deleteVertexBuffer(vb_skybox);
	renderer.frame();
//	renderer.waitForFrameEnd();
}

#define CACHE_SIZE (128 * (1<<20))

bool EngineTestEffect::Init()
{
    renderer.init(gfx::RendererType::OpenGL, 1440, 900, "test", true);
	vtx_cache.init(&renderer, CACHE_SIZE, CACHE_SIZE/4);


	glm::ivec2 win_size = renderer.getFramebufferSize();
	
	texDyn =
		renderer.createTexture2D(512, 512, gfx::TextureFormat::RGBA8, gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear, false, false, gfx::Memory());
	depth_attachment =
		renderer.createTexture2D(win_size.x, win_size.y, gfx::TextureFormat::D24S8, gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear, false, false, gfx::Memory());
    color_attachment = 
        renderer.createTexture2D(win_size.x, win_size.y, gfx::TextureFormat::RGBA8, gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear, false, false, gfx::Memory());
    fb = 
        renderer.createFrameBuffer(std::vector<gfx::TextureHandle>{color_attachment}, depth_attachment);


	const std::vector<std::string> textures_faces = {
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"
	};

	{
		std::vector<gfx::Memory> sky_images;
		int x, y, n, img_x = 0, img_y = 0;
		stbi_set_flip_vertically_on_load(false);
		for (auto& side : textures_faces)
		{
			auto fname = ResourceManager::get_resource(side);
			uint8_t* image = stbi_load(fname.c_str(), &x, &y, &n, 3);
			if (img_x == 0)
			{
				img_x = x;
				img_y = y;
			}
			else if (img_x != x || img_y != y)
			{
				Warning("Cube faces size not equal");
			}
			sky_images.emplace_back(image, x * y * 3);
			stbi_image_free(image);
		}

		skybox =
			renderer.createTextureCubemap(img_x, img_y, gfx::TextureFormat::RGB8_COMPRESSED, gfx::TextureWrap::ClampToEdge, gfx::TextureFilter::Linear, gfx::TextureFilter::Linear, true, sky_images);
	}

	{
		const size_t bufSize = sizeof(gfx::DrawVert) * NUMPOINTS;
		std::vector<gfx::DrawVert> buffer(NUMPOINTS);

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
		const size_t bufSize = sizeof(gfx::DrawVert) * 6;
		std::vector<gfx::DrawVert> buffer(6);
		float r[4]{0.f,0.f,0.f,1.f};

		for (int i = 0; i < 6; ++i)
		{
			r[0] = UNIT_RECT_WITH_ST[i * 4 + 0];
			r[1] = UNIT_RECT_WITH_ST[i * 4 + 1];
			buffer[i].set_position(r);
			r[0] = UNIT_RECT_WITH_ST[i * 4 + 2];
			r[1] = UNIT_RECT_WITH_ST[i * 4 + 3];
			buffer[i].set_texcoord(r);
		}

		vc_pp = vtx_cache.allocStaticVertex(gfx::Memory(buffer));
//		vb_pp = renderer.createVertexBuffer(bufSize, gfx::BufferUsage::Static, gfx::Memory(buffer));
	}

	{
		const int vert_count = (sizeof(UNIT_BOX_POSITIONS) / sizeof(float) / 3);
		const size_t bufSize = sizeof(gfx::DrawVert) * vert_count;

		std::vector<gfx::DrawVert> buffer(vert_count);

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
	
	{
		std::vector<gfx::ShaderHandle> shaders(1);
		std::string cs = ResourceManager::get_text_resource("test_compute2.cs.glsl");
		prgComp = renderer.createProgram();
		shaders[0] = renderer.createShader(gfx::ShaderStage::Compute, cs);
		renderer.linkProgram(prgComp, shaders);
		renderer.deleteShader(shaders[0]);
	}

	{
		std::vector<gfx::ShaderHandle> shaders(2);
		std::string fs = ResourceManager::get_text_resource("test_compute2.fs.glsl");
		std::string vs = ResourceManager::get_text_resource("test_compute2.vs.glsl");
		prgViewTex = renderer.createProgram();
		shaders[0] = renderer.createShader(gfx::ShaderStage::Vertex, vs);
		shaders[1] = renderer.createShader(gfx::ShaderStage::Fragment, fs);
		renderer.linkProgram(prgViewTex, shaders);
		renderer.deleteShader(shaders[0]);
		renderer.deleteShader(shaders[1]);
	}

	{
		std::vector<gfx::ShaderHandle> shaders(2);
		std::string fs = ResourceManager::get_text_resource("draw_point2.fs.glsl");
		std::string vs = ResourceManager::get_text_resource("draw_point2.vs.glsl");
		prgPoints = renderer.createProgram();
		shaders[0] = renderer.createShader(gfx::ShaderStage::Vertex, vs);
		shaders[1] = renderer.createShader(gfx::ShaderStage::Fragment, fs);
		renderer.linkProgram(prgPoints, shaders);
		renderer.deleteShader(shaders[0]);
		renderer.deleteShader(shaders[1]);
	}
	{
		std::vector<gfx::ShaderHandle> shaders(2);
		std::string fs = ResourceManager::get_text_resource("skybox2.fs.glsl");
		std::string vs = ResourceManager::get_text_resource("skybox2.vs.glsl");
		prgSkybox = renderer.createProgram();
		shaders[0] = renderer.createShader(gfx::ShaderStage::Vertex, vs);
		shaders[1] = renderer.createShader(gfx::ShaderStage::Fragment, fs);
		renderer.linkProgram(prgSkybox, shaders);
		renderer.deleteShader(shaders[0]);
		renderer.deleteShader(shaders[1]);
	}
	{
		std::vector<gfx::ShaderHandle> shaders(2);
		std::string fs = ResourceManager::get_text_resource("kernel2.fs.glsl");
		std::string vs = ResourceManager::get_text_resource("kernel2.vs.glsl");
		prgPP = renderer.createProgram();
		shaders[0] = renderer.createShader(gfx::ShaderStage::Vertex, vs);
		shaders[1] = renderer.createShader(gfx::ShaderStage::Fragment, fs);
		renderer.linkProgram(prgPP, shaders);
		renderer.deleteShader(shaders[0]);
		renderer.deleteShader(shaders[1]);
	}
	{
		std::vector<gfx::ShaderHandle> shaders(2);
		std::string fs = ResourceManager::get_text_resource("view_depthbuf.fs.glsl");
		std::string vs = ResourceManager::get_text_resource("view_depthbuf.vs.glsl");
		prgDepth = renderer.createProgram();
		shaders[0] = renderer.createShader(gfx::ShaderStage::Vertex, vs);
		shaders[1] = renderer.createShader(gfx::ShaderStage::Fragment, fs);
		renderer.linkProgram(prgDepth, shaders);
		renderer.deleteShader(shaders[0]);
		renderer.deleteShader(shaders[1]);
	}

	eyeZ = 1200.0f;
	glm::vec3 viewPos{ 0, 0, eyeZ };

	glm::mat4 V = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	P = glm::perspective(45.0f, (float)win_size.x / win_size.y, 1.0f, 1700.0f);

	VP = P * V;

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

void EngineTestEffect::Render()
{
	uint16_t pass = 0;
	gfx::FenceHandle fence;

	{
		std::vector<gfx::DrawVert> buffer(6);
		float r[4]{ 0.f,0.f,0.f,1.f };

		for (int i = 0; i < 6; ++i)
		{
			r[0] = UNIT_RECT_WITH_ST[i * 4 + 0];
			r[1] = UNIT_RECT_WITH_ST[i * 4 + 1];
			buffer[i].set_position(r);
			r[0] = UNIT_RECT_WITH_ST[i * 4 + 2];
			r[1] = UNIT_RECT_WITH_ST[i * 4 + 3];
			buffer[i].set_texcoord(r);
		}

		vc_pp = vtx_cache.allocVertex(std::move(gfx::Memory(buffer)));
		//		vb_pp = renderer.createVertexBuffer(bufSize, gfx::BufferUsage::Static, gfx::Memory(buffer));
	}

	vtx_cache.frame();

	renderer.beginCompute();
	{
		fence = renderer.createFence();
		renderer.setComputeJob(glm::ivec3(512/8, 512/8, 1), fence);
		renderer.setImageTexture(0, texDyn, 0, false, 0, gfx::Access::Write, gfx::TextureFormat::RGBA8);
		renderer.setProgramVar("angle", angle);
		renderer.setProgramVar("fa", 0.7f);
		renderer.submit(pass, prgComp);
		renderer.endCompute();
	}

	glm::mat4 W(1.0f);
	W = glm::rotate(W, glm::radians(rotX), glm::vec3(1, 0, 0));
	W = glm::rotate(W, glm::radians(rotY), glm::vec3(0, 1, 0));

	const glm::mat4 WVP = VP * W;

	uint32_t count, offs;
	vb_points = vtx_cache.getVertexBuffer<gfx::DrawVert>(vc_points, offs, count);

	renderer.setClearBits(pass, gfx::GLS_CLEAR_COLOR | gfx::GLS_CLEAR_DEPTH);
	renderer.setFrameBuffer(pass, fb);
	renderer.setVertexBuffer(vb_points);
	renderer.setPrimitiveType(gfx::PrimitiveType::Point);
	renderer.setRenderState(gfx::GLS_DEPTHFUNC_LESS);
	renderer.setProgramVar("m_WVP", WVP);
	renderer.submit(pass, prgPoints, count, offs, 0);

	W = glm::rotate(glm::mat4(1), glm::radians(rotY), glm::vec3(0, 1, 0));
	const glm::mat4 sky_view = glm::mat4(glm::mat3(W));

	++pass;

	vb_skybox = vtx_cache.getVertexBuffer<gfx::DrawVert>(vc_skybox, offs, count);

	renderer.setClearBits(pass, 0);
	renderer.setFrameBuffer(pass, fb);
	renderer.setRenderState(gfx::GLS_DEPTHFUNC_LESS|gfx::GLS_DEPTHMASK);
	renderer.setPrimitiveType(gfx::PrimitiveType::Triangles);
	renderer.setTexure(0, skybox);
	renderer.setVertexBuffer(vb_skybox);
	renderer.setProgramVar("m_P", P);
	renderer.setProgramVar("m_V", sky_view);
	renderer.setProgramVar("samp0", 0);
	renderer.submit(pass, prgSkybox, count, offs, 0);

	++pass;
	vb_pp = vtx_cache.getVertexBuffer<gfx::DrawVert>(vc_pp, offs, count);
	//renderer.setClearBits(pass, gfx::GLS_CLEAR_COLOR | gfx::GLS_CLEAR_DEPTH);
	renderer.setFrameBuffer(pass, gfx::FrameBufferHandle{0});
	renderer.setRenderState(gfx::GLS_DEPTHFUNC_ALWAYS|gfx::GLS_DEPTHMASK);
	renderer.setPrimitiveType(gfx::PrimitiveType::Triangles);
	renderer.setTexure(0, color_attachment);
	renderer.setVertexBuffer(vb_pp);
	renderer.setProgramVar("samp0", 0);
	renderer.setProgramVar("g_offset", pp_offset);
	renderer.setProgramVar("g_kernel", kernel);
	renderer.submit(pass, prgPP, count, offs, 0);

	++pass;
	renderer.setClearBits(pass, 0);
	renderer.setFrameBuffer(pass, gfx::FrameBufferHandle{ 0 });
	renderer.setRenderState(gfx::GLS_DEPTHFUNC_ALWAYS | gfx::GLS_DEPTHMASK);
	renderer.setPrimitiveType(gfx::PrimitiveType::Triangles);
	renderer.setTexure(0, depth_attachment);
	renderer.setVertexBuffer(vb_pp);
	renderer.setProgramVar("samp0", 0);
	float scale = 0.2f;
	glm::mat4 _w = glm::translate(glm::mat4(1.0f), glm::vec3( (1.0f - scale), (1.0f - scale), 0.0f));
	_w = glm::scale(_w, glm::vec3(scale));
	renderer.setProgramVar("m_W", _w);
	renderer.setProgramVar("g_far", 1700.0f);
	renderer.submit(pass, prgDepth, count, offs, 0);


	++pass;
	renderer.setClearBits(pass, 0);
	renderer.setFrameBuffer(pass, gfx::FrameBufferHandle{ 0 });
	renderer.setRenderState(gfx::GLS_DEPTHFUNC_ALWAYS | gfx::GLS_DEPTHMASK);
	renderer.setPrimitiveType(gfx::PrimitiveType::Triangles);
	renderer.setTexure(0, texDyn);
	renderer.setVertexBuffer(vb_pp);
	renderer.setProgramVar("samp0", 0);
	scale = 0.2f;
	_w = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f + scale, -1.0f + scale, 0.0f));
	_w = glm::scale(_w, glm::vec3(scale));
	renderer.setProgramVar("m_W", _w);
	
	renderer.beginCompute();
	{
		renderer.WaitSync(fence, false, 0);
		renderer.submit(pass);
		renderer.endCompute();
	}
	
	renderer.deleteFence(fence);
	renderer.submit(pass, prgViewTex, count, offs, 0);

	renderer.frame();
}
