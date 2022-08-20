#pragma once

#include <GL/glew.h>
#include "gpu_types.h"
/*

stream: vertex data stream index (vertex buffer binding)
index: vertex attrib index
size: number of elements of the attrib
type: the data type of the attrib
stride: the distance between elemens in bytes
offset: offset from the first attrib in bytes

*/

struct VertexLayout
{
	~VertexLayout();
	VertexLayout& begin();
	VertexLayout& with(
		unsigned int index,
		unsigned int size,
		eDataType type,
		bool normalized,
		unsigned int offset,
		unsigned int stream);

	void end() const;
	void bind() const;

	GLuint m_vao{0};	// vertex array object
	int m_numAttribs;
};