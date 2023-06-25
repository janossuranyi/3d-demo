#include <random>

#include <imgui.h>
#include <tiny_gltf.h>
#include "./System.h"
#include "./Heap.h"
#include "./Engine.h"
#include "./RenderBackend.h"
#include "./RenderSystem.h"
#include "./FrameBuffer.h"
#include "./Logger.h"
#include "./Resources.h"
#include "./Math.h"

#define ON_FLIGHT_FRAMES 2

#define CACHE_LINE_ALIGN(bytes) (((bytes) + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1))
#define ALIGN(a) (((a) + 15) & ~15)

namespace jsr {

	surface_t* R_MakeFullScreenRect();
	surface_t* R_MakeZeroOneCube();
	surface_t* R_MakeZeroOneSphere();
	surface_t* R_MakeZeroOneCone();
	void R_CreateSurfFromTris(drawSurf_t& surf, surface_t& tris);

	const size_t DEFAULT_FRAME_MEM_SIZE = 16 * 1024 * 1024;

	renderGlobals_t			renderGlobals{};
	frameData_t				frames[ON_FLIGHT_FRAMES];
	frameData_t*			frameData;
	frameData_t*			renderData;
	std::atomic_int			maxFrameMemUsage;
	int						activeFrame;
	int						renderFrame;

	RenderSystem::RenderSystem()
	{
		view			= nullptr;
		initialized		= false;
		viewCount		= 0;
		backend			= new RenderBackend();
		vertexCache		= new VertexCache();
		programManager	= new ProgramManager();
		imageManager	= new ImageManager();
		modelManager	= new ModelManager();
		materialManager = new MaterialManager();
		defaultMaterial = {};

		unitCubeSurface_	= {};
		unitCubeTris		= {};
		unitRectSurface_	= {};
		unitRectTris		= {};
		unitSphereSurface_	= {};
		unitSphereTris		= {};
		unitConeSurface_	= {};
		unitConeTris		= {};
	}

	RenderSystem::~RenderSystem()
	{
		if (IsInitialized())
		{
			Shutdown();

			Framebuffer::Shutdown();

			delete modelManager;
			delete materialManager;
			delete imageManager;
			delete programManager;
			delete vertexCache;
			delete backend;

			MemFree( unitRectTris );
			MemFree( unitCubeTris );
			MemFree( unitSphereTris );

			ImGui::DestroyContext();

			R_ShutdownCommandBuffers();

			initialized = false;
			Info("Max frame memory used: %d", maxFrameMemUsage.load());
		}
	}

	bool RenderSystem::Init()
	{
		ImGui::CreateContext();

		if (!backend->Init())
		{
			return false;
		}

		if (!vertexCache->Init())		Error("[RenderSystem]: vertexCache init failed !");
		if (!programManager->Init())	Error("[RenderSystem]: programManager init failed !");
		if (!imageManager->Init())		Error("[RenderSystem]: imageManager init failed !");

		Framebuffer::Init();
		R_InitCommandBuffers();

		unitRectTris = R_MakeFullScreenRect();
		unitCubeTris = R_MakeZeroOneCube();
		unitSphereTris = R_MakeZeroOneSphere();
		unitConeTris = R_MakeZeroOneCone();

		defaultMaterial = materialManager->CreateMaterial("_defaultMaterial");
		stage_t& s = defaultMaterial->GetStage(STAGE_DEBUG);
		s.alphaCutoff = 0.5f;
		s.coverage = COVERAGE_SOLID;
		s.cullMode = CULL_NONE;
		s.diffuseScale = glm::vec4(0.1f,0.00f,0.1f,1.0f);
		s.emissiveScale = glm::vec4(0.0f);
		s.roughnessScale = 0.4f;
		s.metallicScale = 0.0f;
		s.enabled = true;
		s.SetImage(IMU_DIFFUSE, globalImages.whiteImage);
		s.SetImage(IMU_AORM, globalImages.whiteImage);
		s.SetImage(IMU_NORMAL, globalImages.flatNormal);
		s.SetImage(IMU_EMMISIVE, globalImages.whiteImage);
		s.shader = PRG_TEXTURED;

		int bloomdivisor = 1 << renderGlobals.bloomDownsampleLevel;
		programManager->g_commonData.renderTargetRes = glm::vec4{
			engineConfig.r_resX,
			engineConfig.r_resY,
			1.0f / engineConfig.r_resX,
			1.0f / engineConfig.r_resY 
		};
		programManager->g_commonData.bloomRes = glm::vec4{ 
			engineConfig.r_resX / bloomdivisor,
			engineConfig.r_resY / bloomdivisor,
			1.0f / (engineConfig.r_resX / bloomdivisor),
			1.0f / (engineConfig.r_resY / bloomdivisor) 
		};
		programManager->g_commonData.shadowRes.x = renderGlobals.shadowResolution;
		programManager->g_commonData.shadowRes.y = renderGlobals.shadowResolution;
		programManager->g_commonData.shadowRes.z = 1.0f / renderGlobals.shadowResolution;
		programManager->g_commonData.shadowRes.w = 1.0f / renderGlobals.shadowResolution;

		const int kernelSize = sizeof(programManager->g_commonData.ssaoKernel) / sizeof(programManager->g_commonData.ssaoKernel[0]);
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		for (unsigned int i = 0; i < kernelSize; ++i)
		{
			glm::vec4 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator),
				0.0f
			);
			sample = glm::normalize(sample);
			float scale = (float)i / float(kernelSize);
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			//sample *= randomFloats(generator);
			programManager->g_commonData.ssaoKernel[i] = sample;
		}

		programManager->UpdateCommonUniform();

		initialized = true;

		return true;
	}
	void RenderSystem::Shutdown()
	{
	}

	bool RenderSystem::IsInitialized() const
	{
		return initialized;
	}

	void RenderSystem::RenderFrame(const emptyCommand_t* cmds)
	{
		backend->RenderCommandBuffer(cmds);
		++viewCount;
	}

	emptyCommand_t const* RenderSystem::SwapCommandBuffer_BeginNewFrame(bool smpMode)
	{
		emptyCommand_t* cmds = R_SwapCommandBuffers(smpMode);
		
		vertexCache->Frame();

		backend->unitRectSurface = unitRectSurface_;
		backend->unitCubeSurface = unitCubeSurface_;
		backend->unitSphereSurface = unitSphereSurface_;
		backend->unitConeSurface = unitConeSurface_;
		R_CreateSurfFromTris(unitRectSurface_, *unitRectTris);
		R_CreateSurfFromTris(unitCubeSurface_, *unitCubeTris);
		R_CreateSurfFromTris(unitSphereSurface_, *unitSphereTris);
		R_CreateSurfFromTris(unitConeSurface_, *unitConeTris);

		return cmds;
	}

	glm::vec2 RenderSystem::GetScreenSize() const
	{
		int w, h;
		backend->GetScreenSize(w, h);
		return glm::vec2(float(w), float(h));
	}

	uint8_t* R_FrameAlloc(uint32_t bytes)
	{
		bytes = CACHE_LINE_ALIGN(bytes);

		uint8_t* ret{};

		const int next = frameData->used.fetch_add(bytes, std::memory_order_relaxed);
		const int end = next + bytes;

		if (end > DEFAULT_FRAME_MEM_SIZE) {
			Error("Out of frame memory");
		}

		ret = frameData->frameMemory + next;

		int x = maxFrameMemUsage.load(std::memory_order_relaxed);
		if (end > x) {
			maxFrameMemUsage.compare_exchange_weak(x, end);
		}

		for (uint32_t offset = 0; offset < bytes; offset += CACHE_LINE_SIZE) {
			//std::memset(ret + offset, 0, CACHE_LINE_SIZE);
			ZeroCacheLine(ret, offset);
		}

		return ret;
	}

	emptyCommand_t* R_GetCommandBuffer(uint32_t size)
	{
		emptyCommand_t* cmd;
		cmd = (emptyCommand_t*)R_FrameAlloc(size);
		cmd->next = nullptr;
		frameData->cmdTail->next = &cmd->command;
		frameData->cmdTail = cmd;

		return cmd;
	}

	emptyCommand_t* R_SwapCommandBuffers(bool swap)
	{
		if (swap)
		{
			renderFrame = activeFrame;
			activeFrame = (activeFrame + 1) % ON_FLIGHT_FRAMES;
			frameData = &frames[activeFrame];
			renderData = &frames[renderFrame];
		}

		R_ResetCommandBuffer();

		return frames[swap ? renderFrame : activeFrame].cmdHead;
	}

	void R_ResetCommandBuffer()
	{
		uintptr_t bytesNeededForAlignment = 0;
		const uintptr_t aligmentError = ((uintptr_t)frameData->frameMemory & (CACHE_LINE_SIZE - 1));
		if (aligmentError)
		{
			bytesNeededForAlignment = CACHE_LINE_SIZE - aligmentError;
		}

		int size = bytesNeededForAlignment + CACHE_LINE_ALIGN(sizeof(emptyCommand_t));

		frameData->used.store(size, std::memory_order_relaxed);
		emptyCommand_t* cmd = (emptyCommand_t*)frameData->frameMemory + bytesNeededForAlignment;
		std::memset(cmd, 0, sizeof(*cmd));
		cmd->command = RC_NOP;
		cmd->next = nullptr;
		frameData->cmdTail = cmd;
		frameData->cmdHead = cmd;
	}

	void R_InitCommandBuffers()
	{
		assert(CACHE_LINE_SIZE == GetCPUCacheLineSize());

		for (int i = 0; i < ON_FLIGHT_FRAMES; ++i) {
			frameData = &frames[i];
			frameData->frameMemory = (uint8_t*)MemAlloc16(DEFAULT_FRAME_MEM_SIZE);
			R_ResetCommandBuffer();
		}
		activeFrame = 0;
		renderFrame = 1;
		frameData = &frames[activeFrame];
		renderData = &frames[renderFrame];

	}

	void R_ShutdownCommandBuffers()
	{
		for (int i = 0; i < ON_FLIGHT_FRAMES; ++i)
		{
			MemFree16(frames[i].frameMemory);
		}
	}

	static surface_t* R_MakeFullScreenRect()
	{
		const glm::vec3 coords[] = {
			{ 1.0f, 1.0f, 0.0f},
			{-1.0f, 1.0f, 0.0f},
			{-1.0f,-1.0f, 0.0f},
			{ 1.0f,-1.0f, 0.0f}
		};
		const glm::vec2 uv[] = {
			{ 1.0f, 1.0f},
			{ 0.0f, 1.0f},
			{ 0.0f, 0.0f},
			{ 1.0f, 0.0f}
		};
		const float white[] = { 1.0f,1.0f,1.0f,1.0f };

//		surface_t* rect = (surface_t*)MemAlloc(sizeof(*rect));
		surface_t* rect = new (MemAlloc(sizeof(*rect))) surface_t();

		rect->numVerts = 4;
		rect->numIndexes = 6;
		int vertsBytes = ALIGN(sizeof(drawVert_t) * 4);
		rect->verts = (drawVert_t*)MemAlloc(vertsBytes);
		int indexBytes = ALIGN(sizeof(elementIndex_t) * 6);
		rect->indexes = (elementIndex_t*)MemAlloc(indexBytes);
		rect->topology = TP_TRIANGLES;

		for (int i = 0; i < 4; ++i)
		{
			rect->verts[i].SetPos(coords[i]);
			rect->verts[i].SetUV(uv[i]);
			rect->verts[i].SetColor(white);
			rect->verts[i].SetNormal({ 0.0f,0.0f,1.0f });
			rect->verts[i].SetTangent({ 1.0f,0.0f,0.0f,1.0f });
		}

		const elementIndex_t idx[] = {1,2,0,0,2,3};
		for (int i = 0; i < 6; ++i)
		{
			rect->indexes[i] = idx[i];
		}

		rect->bounds.Extend({ -1.0f, -1.0f, 0.0f });
		rect->bounds.Extend({ 1.0f, 1.0f, 0.0f });

		return rect;
	}

	/*
=============
R_MakeZeroOneCubeTris
=============
*/
	static surface_t* R_MakeZeroOneCube()
	{
		using namespace glm;
		surface_t* tri = new (MemAlloc(sizeof(*tri))) surface_t();
		memset(tri, 0, sizeof(*tri));

		tri->numVerts = 8;
		tri->numIndexes = 36;

		const int indexSize = tri->numIndexes * sizeof(tri->indexes[0]);
		const int allocatedIndexBytes = ALIGN(indexSize);
		tri->indexes = (elementIndex_t*)MemAlloc16(allocatedIndexBytes);
		memset(tri->indexes, 0, allocatedIndexBytes);

		const int vertexSize = tri->numVerts * sizeof(tri->verts[0]);
		const int allocatedVertexBytes = ALIGN(vertexSize);
		tri->verts = (drawVert_t*)MemAlloc16(allocatedVertexBytes);
		memset(tri->verts, 0, allocatedVertexBytes);

		tri->topology = TP_TRIANGLES;

		drawVert_t* verts = tri->verts;

		const float low = 0.0f;
		const float high = 1.0f;

		vec3 center(0.0f);
		vec3 mx(low, 0.0f, 0.0f);
		vec3 px(high, 0.0f, 0.0f);
		vec3 my(0.0f, low, 0.0f);
		vec3 py(0.0f, high, 0.0f);
		vec3 mz(0.0f, 0.0f, low);
		vec3 pz(0.0f, 0.0f, high);

		verts[0].xyz = center + mx + my + mz;
		verts[1].xyz = center + px + my + mz;
		verts[2].xyz = center + px + py + mz;
		verts[3].xyz = center + mx + py + mz;
		verts[4].xyz = center + mx + my + pz;
		verts[5].xyz = center + px + my + pz;
		verts[6].xyz = center + px + py + pz;
		verts[7].xyz = center + mx + py + pz;

		// bottom
		tri->indexes[0 * 3 + 0] = 2;
		tri->indexes[0 * 3 + 1] = 3;
		tri->indexes[0 * 3 + 2] = 0;
		tri->indexes[1 * 3 + 0] = 1;
		tri->indexes[1 * 3 + 1] = 2;
		tri->indexes[1 * 3 + 2] = 0;
		// back
		tri->indexes[2 * 3 + 0] = 5;
		tri->indexes[2 * 3 + 1] = 1;
		tri->indexes[2 * 3 + 2] = 0;
		tri->indexes[3 * 3 + 0] = 4;
		tri->indexes[3 * 3 + 1] = 5;
		tri->indexes[3 * 3 + 2] = 0;
		// left
		tri->indexes[4 * 3 + 0] = 7;
		tri->indexes[4 * 3 + 1] = 4;
		tri->indexes[4 * 3 + 2] = 0;
		tri->indexes[5 * 3 + 0] = 3;
		tri->indexes[5 * 3 + 1] = 7;
		tri->indexes[5 * 3 + 2] = 0;
		// right
		tri->indexes[6 * 3 + 0] = 1;
		tri->indexes[6 * 3 + 1] = 5;
		tri->indexes[6 * 3 + 2] = 6;
		tri->indexes[7 * 3 + 0] = 2;
		tri->indexes[7 * 3 + 1] = 1;
		tri->indexes[7 * 3 + 2] = 6;
		// front
		tri->indexes[8 * 3 + 0] = 3;
		tri->indexes[8 * 3 + 1] = 2;
		tri->indexes[8 * 3 + 2] = 6;
		tri->indexes[9 * 3 + 0] = 7;
		tri->indexes[9 * 3 + 1] = 3;
		tri->indexes[9 * 3 + 2] = 6;
		// top
		tri->indexes[10 * 3 + 0] = 4;
		tri->indexes[10 * 3 + 1] = 7;
		tri->indexes[10 * 3 + 2] = 6;
		tri->indexes[11 * 3 + 0] = 5;
		tri->indexes[11 * 3 + 1] = 4;
		tri->indexes[11 * 3 + 2] = 6;

		for (int i = 0; i < 8; i++)
		{
			verts[i].SetColor(vec4(1.0));
		}

		return tri;
	}

	static surface_t* R_MakeZeroOneSphere()
	{
		using namespace glm;
		surface_t* tri = new (MemAlloc(sizeof(*tri))) surface_t();
		memset(tri, 0, sizeof(*tri));

		const float range = 1.0f;
		const int rings = 10.0f; // 20.0f;
		const int sectors = 10.0f; // 20.0f;

		tri->numVerts = (rings * sectors);
		tri->numIndexes = ((rings - 1) * sectors) * 6;

		const int indexSize = tri->numIndexes * sizeof(tri->indexes[0]);
		const int allocatedIndexBytes = ALIGN(indexSize);
		tri->indexes = (elementIndex_t*)MemAlloc16(allocatedIndexBytes);

		const int vertexSize = tri->numVerts * sizeof(tri->verts[0]);
		const int allocatedVertexBytes = ALIGN(vertexSize);
		tri->verts = (drawVert_t*)MemAlloc16(allocatedVertexBytes);

		tri->topology = TP_TRIANGLES;

		drawVert_t* verts = tri->verts;

		float const R = 1.0f / (float)(rings - 1);
		float const S = 1.0f / (float)(sectors - 1);

		int numTris = 0;
		int numVerts = 0;
		constexpr float HALF_PI = glm::pi<float>() / 2.0f;
		constexpr float PI = glm::pi<float>();

		for (int r = 0; r < rings; ++r)
		{
			for (int s = 0; s < sectors; ++s)
			{
				const float y = sin(-HALF_PI + PI * r * R);
				const float x = cos(2 * PI * s * S) * sin(PI * r * R);
				const float z = sin(2 * PI * s * S) * sin(PI * r * R);

				verts[numVerts].SetUV(s * S, r * R);
				verts[numVerts].xyz = vec3(x, y, z) * range;
				verts[numVerts].SetNormal(x, y, z);
				verts[numVerts].SetColor(vec4(0.0f, 0.0f, 1.0f, 1.0f));
				numVerts++;
				tri->bounds.Extend(vec3(x, y, z) * range);

				if (r < (rings - 1))
				{
					int curRow = r * sectors;
					int nextRow = (r + 1) * sectors;
					int nextS = (s + 1) % sectors;

					tri->indexes[(numTris * 3) + 0] = (curRow + s);
					tri->indexes[(numTris * 3) + 1] = (nextRow + s);
					tri->indexes[(numTris * 3) + 2] = (nextRow + nextS);

					numTris += 1;

					tri->indexes[(numTris * 3) + 0] = (curRow + s);
					tri->indexes[(numTris * 3) + 1] = (nextRow + nextS);
					tri->indexes[(numTris * 3) + 2] = (curRow + nextS);

					numTris += 1;
				}
			}
		}

		return tri;
	}
	surface_t* R_MakeZeroOneCone()
	{
		using namespace tinygltf;
		using namespace glm;

		std::string warn, err;
		TinyGLTF loader;
		Model model;
		const std::string filename = ResourceManager::instance.GetResource("models/zeroOneCone2.glb");
		if (loader.LoadBinaryFromFile(&model, &err,&warn,filename))
		{
			surface_t* tri = new (MemAlloc16(sizeof(*tri))) surface_t();
			memset(tri, 0, sizeof(*tri));
			tri->topology = TP_TRIANGLES;

			int posIdx = model.meshes[0].primitives[0].attributes.at("POSITION");
			int norIdx = model.meshes[0].primitives[0].attributes.at("NORMAL");
			int tanIdx = model.meshes[0].primitives[0].attributes.at("TANGENT");
			int texIdx = model.meshes[0].primitives[0].attributes.at("TEXCOORD_0");

			const auto& posAccess = model.accessors[posIdx];
			const auto& norAccess = model.accessors[norIdx];
			const auto& tanAccess = model.accessors[tanIdx];
			const auto& texAccess = model.accessors[texIdx];
			const auto& idxAccess = model.accessors[model.meshes[0].primitives[0].indices];

			tri->numVerts = posAccess.count;
			const int vertexSize = tri->numVerts * sizeof(tri->verts[0]);
			const int allocatedVertexBytes = ALIGN(vertexSize);
			tri->verts = (drawVert_t*)MemAlloc16(allocatedVertexBytes);
			tri->numIndexes = idxAccess.count;
			const int indexSize = tri->numIndexes * sizeof(tri->indexes[0]);
			const int allocatedIndexBytes = ALIGN(indexSize);
			tri->indexes = (elementIndex_t*)MemAlloc16(allocatedIndexBytes);

			AccessorArray<vec3>		posarray(model, posAccess);
			AccessorArray<vec3>		norarray(model, norAccess);
			AccessorArray<vec4>		tanarray(model, tanAccess);
			AccessorArray<vec2>		texarray(model, texAccess);
			AccessorArray<unsigned short> idxarray(model, idxAccess);
			const vec4 ONE(1.0f);
			for (auto i = 0; i < tri->numVerts; ++i)
			{
				vec3 xyz = *posarray[i];
				//std::swap(xyz.y, xyz.z);
				tri->verts[i].SetPos(xyz);
				tri->verts[i].SetNormal(*norarray[i]);
				tri->verts[i].SetTangent(*tanarray[i]);
				tri->verts[i].SetUV(*texarray[i]);
				tri->verts[i].SetColor(ONE);
			}

			memcpy(tri->indexes, idxarray[0], tri->numIndexes * 2);

			return tri;
		}
		return nullptr;
	}

	void R_CreateSurfFromTris(drawSurf_t& surf, surface_t& tris)
	{
		surf.frontEndGeo = &tris;
		if (tris.numIndexes == 0)
		{
			surf.numIndex = 0;
			return;
		}

		if ( ! renderSystem.vertexCache->IsCurrent(tris.vertexCache) )
		{
			tris.vertexCache = renderSystem.vertexCache->AllocTransientVertex(tris.verts, tris.numVerts * sizeof(tris.verts[0]));
		}
		if ( ! renderSystem.vertexCache->IsCurrent(tris.indexCache) )
		{
			tris.indexCache = renderSystem.vertexCache->AllocTransientIndex(tris.indexes, ( tris.numIndexes * sizeof(tris.indexes[0]) ) );
		}

		surf.indexCache = tris.indexCache;
		surf.vertexCache = tris.vertexCache;
		surf.numIndex = tris.numIndexes;

	}


	RenderSystem renderSystem;
}
