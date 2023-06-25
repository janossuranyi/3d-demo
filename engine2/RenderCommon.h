#pragma once

#include <glm/glm.hpp>

#include "./EngineTypes.h"
#include "./VertexCache.h"
#include "./Bounds.h"
#include "./Frustum.h"

//#include "./Model.h"

namespace jsr {

	enum eLightType
	{
		LIGHT_DIRECTED,
		LIGHT_POINT,
		LIGHT_SPOT,
		LIGHT_BOX,
		LIGHT_COUNT
	};

	enum eFaceSide
	{
		FACE_FRONT,
		FACE_BACK,
		FACE_FRONT_AND_BACK,
		FACE_COUNT
	};

	enum eCompOp
	{
		CMP_NEVER,
		CMP_ALWAYS,
		CMP_LT,
		CMP_LEQ,
		CMP_EQ,
		CMP_GT,
		CMP_GEQ,
		CMP_NOTEQ,
		CMP_COUNT
	};

	enum eBlendFunc
	{
		BFUNC_ZERO,
		BFUNC_ONE,
		BFUNC_SRC_ALPHA,
		BFUNC_DST_ALPHA,
		BFUNC_SRC_COLOR,
		BFUNC_DST_COLOR,
		BFUNC_ONE_MINUS_SRC_ALPHA,
		BFUNC_ONE_MINUS_DST_ALPHA,
		BFUNC_ONE_MINUS_SRC_COLOR,
		BFUNC_ONE_MINUS_DST_COLOR,
		BFUNC_COUNT
	};

	enum eBlendOp
	{
		BOP_ADD,
		BOP_SUB,
		BOP_RSUB,
		BOP_MIN,
		BOP_MAX,
		BOP_COUNT
	};

	enum eStencilOp
	{
		SO_KEEP,
		SO_ZERO,
		SO_REPLACE,
		SO_INC,
		SO_INC_WRAP,
		SO_DEC,
		SO_DEC_WRAP,
		SO_INVERT,
		SO_COUNT
	};

	enum eCoverage
	{
		COVERAGE_SOLID,
		COVERAGE_MASK,
		COVERAGE_BLEND,
		COVERAGE_COUNT
	};

	enum eStageType
	{
		STAGE_GBUFFER,
		STAGE_PRELIGHT,
		STAGE_POSTLIGHT,
		STAGE_SHADOW,
		STAGE_PP,
		STAGE_DEBUG,
		STAGE_COUNT
	};

	enum eShaderProg
	{
		PRG_TEXTURED,
		PRG_ZPASS,
		PRG_EQUIRECT_TEXTURE,
		PRG_DEFERRED_GBUFFER_MR,
		PRG_DEFERRED_LIGHT,
		PRG_DEFERRED_DIR_LIGHT,
		PRG_PP_HDR,
		PRG_COLOR,
		PRG_FXAA3,
		PRG_EMISSIVE,
		PRG_GAUSS_FILTER,
		PRG_BLOOM_FILTER,
		PRG_SSAO_GEN,
		PRG_KERNEL,
		PRG_COUNT
	};

	enum eCullMode
	{
		CULL_NONE,
		CULL_FRONT,
		CULL_BACK,
		CULL_COUNT
	};

	enum eTopology
	{
		TP_POINTS,
		TP_LINES,
		TP_LINE_STRIPS,
		TP_TRIANGLE_FANS,
		TP_TRIANGLE_STRIPS,
		TP_TRIANGLES,
		TP_COUNT
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
		glm::mat4	viewMatrix;
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
		vertCacheHandle_t highFreqVert;
		bool			shadowOnly;
	};

	struct surface_t;
	struct drawSurf_t
	{
		const surface_t*	frontEndGeo;
		int					numIndex;
		vertCacheHandle_t	indexCache;
		vertCacheHandle_t	vertexCache;
		vertCacheHandle_t	highFreqFrag[STAGE_COUNT];
		const viewEntity_t* space;
		const Material*		shader;
		const drawSurf_t*	next;
		float				sort;
	};

	class Node3D;
	struct shadowEntity_t
	{
		shadowEntity_t* next;
		Node3D* renderNode;
	};

	struct viewLight_t
	{
		viewLight_t*		next;
		glm::vec3			origin;	// view space position
		glm::vec3			axis;
		glm::vec4			color;
		float				range;
		float				coneAngle;
		eShaderProg			shader;
		eLightType			type;
		vertCacheHandle_t	highFreqVert;
		vertCacheHandle_t	lightData;
		Frustum				frustum;
		bool				remove;
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
		const drawSurf_t**	drawSurfs;
		int				numDrawSurfs;
		viewEntity_t*	viewEntites;
		viewLight_t*	viewLights;
		viewLight_t*	viewSunLight;
		Frustum			frustum;
		float			exposure;
		float			farClipDistance;
		float			nearClipDistance;
		vertCacheHandle_t freqLowVert;
		vertCacheHandle_t freqLowFrag;
	};

}