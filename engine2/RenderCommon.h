#pragma once

#include <glm/glm.hpp>
#include "./EngineTypes.h"
#include "./VertexCache.h"

namespace jsr {

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


	struct meshGeometry_t;
	class Material;
	class Model;
	class RenderModel;
	struct viewEntity_t;

	struct modelSurface_t
	{
		int	id;
		const Material* shader;
		meshGeometry_t* geometry;
	};

	class RenderWorld;

	struct renderEntity_t
	{

	};
	struct drawSurf_t
	{
		const meshGeometry_t* geoRef;
		int						indexCount;
		vertCacheHandle_t		indexCache;
		vertCacheHandle_t		vertexCache;
		const viewEntity_t* space;
		const Material* shader;
		float sort;
	};

	struct viewEntity_t
	{
		viewEntity_t* next;
		float			modelMatrix[16];		// local coords to global coords
		float			modelViewMatrix[16];	// local coords to eye coords
		glm::mat4		mvp;
	};

	struct viewLight_t
	{
		viewLight_t* next;
		glm::vec3		vieworg;
		glm::mat3		viewaxis;
		float			radius;
		const Material* shader;
	};

	struct viewDef_t
	{
		renderView_t	renderView;
		float			projectionMatrix[16];
		float			unprojectionToCameraMatrix[16];
		float			unprojectionToWorldMatrix[16];

		screenRect_t	viewport;
		screenRect_t	scissor;
		bool			isSubview;
		bool			isMirror;
		RenderWorld*	 renderWorld;
		drawSurf_t**	drawSurfs;
		int				numDrawSurfs;
		viewEntity_t*	viewEntites;
		viewLight_t*	viewLights;
	};

}