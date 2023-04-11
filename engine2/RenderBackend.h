#pragma once

#include <cinttypes>
#include <glm/glm.hpp>

namespace jsr {

	const int MAX_TEXTURE_UNITS = 14;

	enum eVertexLayout
	{
		VL_NONE = -1,
		VL_DRAW_VIEW,
		VL_POSITION_ONLY
	};

	struct tmu_t
	{
		unsigned int	current2DMap;
		unsigned int	current2DArray;
		unsigned int	currentCubeMap;
		unsigned int	currentCubeMapArray;
	};

	struct renderView_t
	{

	};

	struct glcontext_t
	{
		uint64_t	frameCounter;
		tmu_t		tmu[MAX_TEXTURE_UNITS];
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
	private:
		float	clearColor[4];
		int		currenttmu;
		bool	initialized;
	public:	
		eVertexLayout currentLayout;
		Framebuffer* currentFramebuffer{};
	};

	extern bool R_InitGfxAPI();
	extern void R_ShutdownGfxAPI();
}