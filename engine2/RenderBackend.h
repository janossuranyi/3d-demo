#pragma once

#include <cinttypes>
#include <glm/glm.hpp>
#include "./RenderCommon.h"
#include "./RenderProgs.h"

namespace jsr {

	const int MAX_TEXTURE_UNITS = 14;
	const int MAX_BINDING = 8;

	enum eFillMode
	{
		FM_SOLID,
		FM_WIREFRAME
	};

	enum eRenderCommand
	{
		RC_NOP,
		RC_DRAW_VIEW,
		RC_COUNT
	};

	struct emptyCommand_t
	{
		eRenderCommand command;
		eRenderCommand* next;
	};

	struct drawViewCommand_t
	{
		eRenderCommand command;
		eRenderCommand* next;
		viewDef_t* view;
	};

	struct rasterizerState_t
	{
		eFillMode		fillMode;
		eCullMode		currentCullMode;
		bool			cullEnabled;
		bool operator==(const rasterizerState_t& other) const;
		bool operator!=(const rasterizerState_t& other) const;
	};

	struct depthState_t
	{
		bool	enabled;
		bool	depthMask;
		eCompOp	func;
		bool operator==(const depthState_t& other) const;
		bool operator!=(const depthState_t& other) const;
	};

	struct blendingState_t
	{
		bool enabled;
		eBlendFunc colSrc;
		eBlendFunc colDst;
		eBlendFunc alphaSrc;
		eBlendFunc alphaDst;
		eBlendOp colorOp;
		eBlendOp alphaOp;

		bool operator==(const blendingState_t& other) const;
		bool operator!=(const blendingState_t& other) const;
		bool operator()(const blendingState_t* a, const blendingState_t* b) const;
	};

	struct backendCounters_t
	{
		unsigned int drawElements{};
		unsigned int drawIndexes{};
	};
	struct tmu_t
	{
		unsigned int	current2DMap;
		unsigned int	current2DArray;
		unsigned int	currentCubeMap;
		unsigned int	currentCubeMapArray;
	};

	struct bufferBinding_t
	{
		unsigned int buffer;
		unsigned int offset;
		unsigned int stride;
		bool operator==(const bufferBinding_t& other) const;
	};


	struct glcontext_t
	{
		uint64_t		frameCounter;
		tmu_t			tmu[MAX_TEXTURE_UNITS];
		unsigned int	currentProgram;
		unsigned int	currentIndexBuffer;
		unsigned int	currentFramebuffer;
		unsigned int	currentReadFramebuffer;
		glm::bvec4		writeMask;
		glm::vec4		clearColor;
		eVertexLayout	currentVertexLayout;
		blendingState_t blendState;
		depthState_t	depthState;
		bufferBinding_t vtxBindings[MAX_BINDING];
		bufferBinding_t uboBindings[MAX_BINDING];
		rasterizerState_t rasterizer;
	};

	extern glcontext_t glcontext;

	class Framebuffer;
	class RenderBackend
	{
	public:
		RenderBackend();
		~RenderBackend();

		bool		Init();
		void		Shutdown();
		void		GetScreenSize(int& x, int& y) const;
		void		SetClearColor(float r, float g, float b, float a);
		void		SetClearColor(glm::vec4 color);
		glm::vec4	GetClearColor() const;
		void		Clear(bool color, bool depth, bool stencil);
		int			GetUniformBufferAligment() const;
		void		EndFrame();
		int			GetCurrentTextureUnit() const;
		void		SetCurrentTextureUnit(int unit);
		bool		IsInitialized() const;
		tmu_t*		GetTextureUnit(int index);
		void		RenderView(viewDef_t* view);
		void		RenderCommandBuffer(const emptyCommand_t* cmds);
		void		SetCullMode(eCullMode mode);
	private:
		float		clearColor[4];
		int			currenttmu;
		bool		initialized;
		viewDef_t*	view;
		void		RenderDebugPass();
		void		RenderDepthPass();
		void		RenderShadow();
		void		RenderDeferred_GBuffer();
		void		RenderDeferred_Lighting();
		void		RenderHDRtoLDR();
	public:
		Framebuffer* currentFramebuffer{};
		drawSurf_t unitRectSurface{};
		drawSurf_t unitCubeSurface{};
		drawSurf_t unitSphereSurface{};
	};

	extern bool R_InitGfxAPI();
	extern void R_ShutdownGfxAPI();
	extern backendCounters_t perfCounters;
}