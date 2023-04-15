#pragma once

#include <glm/glm.hpp>
#include "./EngineTypes.h"
#include "./VertexCache.h"

namespace jsr {

	enum eVertexLayout
	{
		VL_NONE = -1,
		VL_DRAW_VERT,
		VL_POSITION_ONLY
	};

	struct screenRect_t
	{
		int		x1;
		int		y1;
		int		x2;
		int		y2;
	};

	struct renderView_t
	{
		int			viewID;
		float		fov;
		glm::vec3	vieworg;
		glm::mat3	viewaxis;
		int			rdflags;
	};

	// vertex index type (16bit)
	typedef unsigned short elementIndex_t;

	// Triangle list based surface
	struct triangles_t
	{
		int numVertices;
		int numIndices;
		drawVert_t* vertices;
		elementIndex_t* indices;
	};

	class Material;
	class Model;
	class RenderModel;

	struct modelSurface_t
	{
		int	id;
		const Material* shader;
		triangles_t* surf;
	};

	class RenderWorld;

	struct renderEntity_t
	{

	};

	struct drawSurf_t;
	struct viewEntity_t
	{
		viewEntity_t*	next;
		glm::mat4		modelMatrix;		// local coords to global coords
		glm::mat4		modelViewMatrix;	// local coords to eye coords
		glm::mat4		mvp;
		drawSurf_t*		surf;
	};

	struct drawSurf_t
	{
		const triangles_t*	refSurf;
		int					numIndex;
		vertCacheHandle_t	indexCache;
		vertCacheHandle_t	vertexCache;
		const viewEntity_t* space;
		const Material*		shader;
		float				sort;
	};


	struct viewLight_t
	{
		viewLight_t*	next;
		glm::vec3		origin;
		glm::mat3		axis;
		float			radius;
		const Material* shader;
	};

	struct viewDef_t
	{
		renderView_t	renderView;
		glm::mat4		projectionMatrix;
		glm::mat4		unprojectionToCameraMatrix;
		glm::mat4		unprojectionToWorldMatrix;

		screenRect_t	viewport;
		screenRect_t	scissor;
		bool			isSubview;
		bool			isMirror;
		RenderWorld*	renderWorld;
		drawSurf_t**	drawSurfs;
		int				numDrawSurfs;
		viewEntity_t*	viewEntites;
		viewLight_t*	viewLights;
	};

}