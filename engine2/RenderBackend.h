#pragma once

#include <cinttypes>
#include <glm/glm.hpp>
#include "./RenderCommon.h"
#include "./RenderProgs.h"

namespace jsr {

	const int MAX_TEXTURE_UNITS = 14;
	const int MAX_BINDING = 8;

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

	};

	struct blendingState_t
	{

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
	};

	struct glcontext_t
	{
		uint64_t		frameCounter;
		tmu_t			tmu[MAX_TEXTURE_UNITS];
		unsigned int	currentProgram;
		unsigned int	currentIndexBuffer;
		eVertexLayout	currentVertexLayout;
		bufferBinding_t vtxBindings[MAX_BINDING];
		bufferBinding_t uboBindings[MAX_BINDING];
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
	private:
		float	clearColor[4];
		int		currenttmu;
		bool	initialized;
	public:	
		Framebuffer* currentFramebuffer{};
	};

	extern bool R_InitGfxAPI();
	extern void R_ShutdownGfxAPI();
}