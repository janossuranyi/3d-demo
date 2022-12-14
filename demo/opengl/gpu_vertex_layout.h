#pragma once

#include <GL/glew.h>
#include "gpu_types.h"
#include "gpu_buffer.h"
/*

stream: vertex data stream index (vertex buffer binding)
index: vertex attrib index
size: number of elements of the attrib
type: the data type of the attrib
stride: the distance between elemens in bytes
offset: offset from the first attrib in bytes

*/

struct VertexAttribute
{
	char* name;
	eDataType type;
	int size;
	int count;
	bool normalized;
	unsigned int offset;
	unsigned int stride;
	unsigned int stream;
	unsigned int byteSize;
};

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

	VertexLayout& with(
		unsigned int index,
		unsigned int size,
		eDataType type,
		bool normalized,
		unsigned int offset,
		unsigned int stride,
		GpuBuffer* target);

	void end() const;
	void bind() const;

	GLuint m_vao{0};	// vertex array object
	int m_numAttribs;

	GpuBuffer* m_lastBuffer{};
};