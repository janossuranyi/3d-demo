#include "./RenderBackend.h"
#include "./RenderProgs.h"
#include "./Logger.h"
#include "./GLState.h"
#include <memory>

namespace jsr {

	glcontext_t glcontext{};

	backendCounters_t perfCounters;

	RenderBackend::RenderBackend()
	{
		glcontext.frameCounter = 0;
		std::memset(glcontext.tmu, 0xffff, sizeof(glcontext.tmu));
		currenttmu = 0;
		currentFramebuffer = nullptr;
		clearColor[0] = .0f;
		clearColor[1] = .0f;
		clearColor[2] = .0f;
		clearColor[3] = 1.0f;
		for (int i = 0; i < MAX_BINDING; ++i) glcontext.vtxBindings[i] = { 0,0,0 };
		initialized = false;
		view = {};
	}

	bool RenderBackend::IsInitialized() const
	{
		return initialized;
	}

	tmu_t* RenderBackend::GetTextureUnit(int index)
	{
		return &glcontext.tmu[index];
	}

	RenderBackend::~RenderBackend()
	{
		Shutdown();
	}

	bool RenderBackend::Init()
	{
		Info("---- Init graphics subsystem ----");
		if (!R_InitGfxAPI())
		{
			return false;
		}

		blendingState_t blendState{};
		blendState.enabled = false;
		blendState.opts.alphaOp = BOP_ADD;
		blendState.opts.colorOp = BOP_ADD;
		blendState.opts.alphaDst = BFUNC_ZERO;
		blendState.opts.colDst = BFUNC_ZERO;
		blendState.opts.alphaSrc = BFUNC_ONE;
		blendState.opts.colSrc = BFUNC_ONE;

		SetBlendingState(blendState);
		SetDepthState({ true, true, CMP_LEQ });

		initialized = true;
		return true;
	}

	void RenderBackend::Shutdown()
	{
		if (initialized)
		{
			R_ShutdownGfxAPI();
			initialized = false;
		}
	}

	glm::vec4 RenderBackend::GetClearColor() const
	{
		return glm::vec4(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	}

	int RenderBackend::GetCurrentTextureUnit() const
	{
		return currenttmu;
	}
	bool blendingState_t::operator==(const blendingState_t& other) const
	{
		return memcmp(this, &other, sizeof(other)) == 0;
	}
	bool blendingState_t::operator!=(const blendingState_t& other) const
	{
		return memcmp(this, &other, sizeof(other)) != 0;
	}
	bool blendingState_t::operator()(const blendingState_t* a, const blendingState_t* b) const
	{
		return memcmp(a, b, sizeof(this)) < 0;
	}
	bool depthState_t::operator==(const depthState_t& other) const
	{
		return enabled == other.enabled && depthMask == other.depthMask && func == other.func;
	}
	bool depthState_t::operator!=(const depthState_t& other) const
	{
		return !operator==(other);
	}
	bool rasterizerState_t::operator==(const rasterizerState_t& other) const
	{
		return fillMode == other.fillMode && currentCullMode == other.currentCullMode && cullEnabled == other.cullEnabled;
	}
	bool rasterizerState_t::operator!=(const rasterizerState_t& other) const
	{
		return !operator==(other);
	}
	bool bufferBinding_t::operator==(const bufferBinding_t& other) const
	{
		return other.offset == offset && other.stride == stride && other.buffer == buffer;
	}
	bool stencilState_t::operator==(const stencilState_t& other) const
	{
		return memcmp(this, &other, sizeof(*this)) == 0;
	}
}
