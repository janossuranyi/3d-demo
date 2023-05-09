#pragma once

#include <cinttypes>
#include <glm/glm.hpp>
#include "./RenderCommon.h"
#include "./RenderProgs.h"

namespace jsr {

	const int MAX_TEXTURE_UNITS = 14;

	enum eRenderCommand
	{
		RC_NOP
	};

	struct emptyCommand_t
	{
		eRenderCommand command;
		emptyCommand_t* next;
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

	struct glcontext_t
	{
		uint64_t		frameCounter;
		tmu_t			tmu[MAX_TEXTURE_UNITS];
		unsigned int	currentVertexBuffer;
		unsigned int	currentIndexBuffer;
		unsigned int	currentProgram;
		eVertexLayout	currentVertexLayout;
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
		void		RenderCommandBuffer(emptyCommand_t* cmds);
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