#pragma once

#include "Common.h"

/**
\brief The type of a shader.
\ingroup Resources
*/
enum class ShaderType : uint {
	VERTEX, ///< Vertex shader.
	TESSCONTROL, ///< Tessellation control shader.
	TESSEVAL, ///< Tessellation evaluation shader.
	FRAGMENT, ///< Fragment shader.
	COMPUTE, ///< Compute shader.
	COUNT
};

STD_HASH(ShaderType);

/**
\brief The type of data a buffer is storing, determining its use.
\ingroup Resources
*/
enum class BufferType : uint {
	VERTEX, ///< Vertex data.
	INDEX, ///< Element indices.
	UNIFORM, ///< Uniform data.
//	CPUTOGPU, ///< Transfer.
//	GPUTOCPU, ///< Transfer.
	STORAGE, ///< Compute storage.
	TEXTURE
};

STD_HASH(BufferType);

/**
\brief The frequency at which a uniform buffer might be updated.
\ingroup Resources
*/
enum class UniformFrequency : uint {
	STATIC, ///< Data won't be updated after upload.
	FRAME,  ///< Data will be updated at most once per frame.
	VIEW,  ///< Data will be updated a few times per frame.
	DYNAMIC ///< Data will be updated many times per frame.
};

/**
\brief Depth or stencil test function.
\ingroup Resources
*/
enum class TestFunction : uint {
	NEVER, ///< Fail in all cases
	LESS, ///< Pass if lower
	LEQUAL, ///< Pass if lower or equal
	EQUAL, ///< Pass if equal
	GREATER, ///< Pass if greater
	GEQUAL, ///< Pass if greater or equal
	NOTEQUAL, ///< Pass if different
	ALWAYS ///< Always pass
};

STD_HASH(TestFunction);

/**
\brief Stencil operation to perform.
\ingroup Resources
*/
enum class StencilOp : uint {
	KEEP, ///< Keep current value.
	ZERO,	///< Set value to zero.
	REPLACE,	///< Set value to reference.
	INCR,	///< Increment value and clamp.
	INCRWRAP, ///< Increment value and wrap.
	DECR,	///< Decrement value and clamp.
	DECRWRAP, ///< Decrement value and wrap.
	INVERT ///< Invert value bitwise.
};

STD_HASH(StencilOp);

/**
\brief Blending mix equation for each component. Below we use src and dst to denote
 the (modulated by the blend functions) values to blend.
 Note that min and max do not apply the modulation factor to each term.
\ingroup Resources
*/
enum class BlendEquation : uint {
	ADD, ///< Perform src + dst
	SUBTRACT, ///< Perform src - dst
	REVERSE_SUBTRACT, ///< Perform dst - src
	MIN, ///< Perform min(src, dst)
	MAX ///< Perform max(src, dst)
};

STD_HASH(BlendEquation);


/**
\brief How the source and destination values to blend are obtained from the pixel data by scaling.
\ingroup Resources
*/
enum class BlendFunction : uint {
	ZERO, ///< Multiply by 0
	ONE, ///< Multiply by 1
	SRC_COLOR, ///< Multiply by src color, component wise
	ONE_MINUS_SRC_COLOR, ///< Multiply by 1-src color, component wise
	DST_COLOR, ///< Multiply by dst color, component wise
	ONE_MINUS_DST_COLOR, ///< Multiply by 1-dst color, component wise
	SRC_ALPHA, ///< Multiply by src scalar alpha
	ONE_MINUS_SRC_ALPHA, ///< Multiply by 1-src scalar alpha
	DST_ALPHA, ///< Multiply by dst scalar alpha
	ONE_MINUS_DST_ALPHA ///< Multiply by 1-dst scalar alpha
};

STD_HASH(BlendFunction);

/**
\brief Used to select a subset of faces. Front faces are defined counter-clockwise.
\ingroup Resources
*/
enum class Faces : uint {
	FRONT, ///< Front (CCW) faces
	BACK, ///< Back (CW) faces
	ALL ///< All faces
};

STD_HASH(Faces);

/**
\brief How polygons should be rasterized
\ingroup Resources
*/
enum class PolygonMode : uint {
	FILL, ///< As filled polygons.
	LINE, ///< As wireframe edges.
	POINT ///< As vertex points.
};

STD_HASH(PolygonMode);

/**
 \brief The shape of a texture: dimensions, layers organisation.
 \ingroup Resources
 */
enum class TextureShape : uint {
	D1		  = 1 << 1,		 ///< 1D texture.
	D2		  = 1 << 2,		 ///< 2D texture.
	D3		  = 1 << 3,		 ///< 3D texture.
	Cube	  = 1 << 4,		 ///< Cubemap texture.
	Array	  = 1 << 5,		 ///< General texture array flag.
	Array1D   = D1 | Array,  ///< 1D texture array.
	Array2D   = D2 | Array,  ///< 2D texture array.
	ArrayCube = Cube | Array ///< Cubemap texture array.
};

/** Combining operator for TextureShape.
 \param t0 first flag
 \param t1 second flag
 \return the combination of both flags.
 */
inline TextureShape operator|(TextureShape t0, TextureShape t1) {
	return static_cast<TextureShape>(static_cast<uint>(t0) | static_cast<uint>(t1));
}

/** Extracting operator for TextureShape.
 \param t0 reference flag
 \param t1 flag to extract
 \return true if t0 'contains' t1
 */
inline bool operator&(TextureShape t0, TextureShape t1) {
	return bool(static_cast<uint>(t0) & static_cast<uint>(t1));
}

/** Combining operator for TextureShape.
 \param t0 first flag
 \param t1 second flag
 \return reference to the first flag after combination with the second flag.
 */
inline TextureShape & operator|=(TextureShape & t0, TextureShape & t1) {
	return t0 = t0 | t1;
}

STD_HASH(TextureShape);

/**
 \brief The filtering mode of a texture: we deduce the magnification
 filter from the minification filter for now.
 \ingroup Resources
 */
enum class Filter : uint {
	NEAREST = 0, ///< Nearest neighbour, no mipmap.
	LINEAR, ///< Bilinear, no mipmap.
	NEAREST_NEAREST, ///< Nearest neighbour, closest mipmap.
	LINEAR_NEAREST, ///< Bilinear, closest mipmap.
	NEAREST_LINEAR, ///< Nearest neighbour, linear blend of mipmaps.
	LINEAR_LINEAR ///< Bilinear, linear blend of mipmaps.
};

STD_HASH(Filter);

/**
 \brief The wrapping mode of a texture.
 \ingroup Resources
 */
enum class Wrap : uint {
	CLAMP = 0, ///< Clamp to the edges of the texture.
	REPEAT, ///< Repeat the texture.
	MIRROR ///< Repeat the texture using flipped versions to ensure continuity.
};

STD_HASH(Wrap);

/**
 \brief The layout of a texture: components count and type.
 \ingroup Resources
 */
enum class Layout : uint {
	R, R16, R16F, RG, RG16, RG16F, RGB, RGBA, SRGB, SRGB_A, RGBA16F, RGB10A2, RGBA32F, DEPTH24_STENCIL_8, COMPRESSED_RGBA, COMPRESSED_SRGB, R11F_G11F_B10F, RGB5_A1, RGB565
};

STD_HASH(Layout);
