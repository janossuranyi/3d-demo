#pragma once

#include <glm/glm.hpp>
#include "./EngineTypes.h"
#include "./VertexCache.h"
#include "./Bounds.h"
#include "./Frustum.h"
//#include "./Model.h"

namespace jsr {

	enum eTopology
	{
		TP_POINTS,
		TP_LINES,
		TP_LINE_STRIPS,
		TP_TRIANGLE_FANS,
		TP_TRIANGLE_STRIPS,
		TP_TRIANGLES
	};

	struct screenRect_t
	{
		int		x;
		int		y;
		int		w;
		int		h;
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


	class Material;

	class RenderWorld;

	struct drawSurf_t;
	struct viewEntity_t
	{
		viewEntity_t*	next;
		glm::mat4		modelMatrix;		// local coords to global coords
		glm::mat4		modelViewMatrix;	// local coords to eye coords
		glm::mat4		mvp;
		drawSurf_t*		surf;
	};

	struct vertexData_t;
	struct drawSurf_t
	{
		const vertexData_t*	frontEndGeo;
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
		Frustum			frustum;
	};

}