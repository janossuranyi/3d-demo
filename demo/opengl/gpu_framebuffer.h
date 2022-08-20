#pragma once

#include <cinttypes>
#include <vector>
#include <memory>
#include "gpu_types.h"
#include "gpu_utils.h"
#include "gpu_texture.h"

class GpuFrameBuffer
{
public:
	~GpuFrameBuffer();
	GpuFrameBuffer() :
		m_fbo(0),
		m_depthRenderBuffer(0),
		m_depthRenderTexture(nullptr),
		m_completed(false) {}

	GpuFrameBuffer& create();
	GpuFrameBuffer& addColorAttachment(int index, std::shared_ptr<GpuTexture2D> texture);
	GpuFrameBuffer& addColorAttachment(int index, std::shared_ptr<GpuTextureCubeMap> texture);
	GpuFrameBuffer& addColorAttachment(int index, int w, int h, ePixelFormat format);
	GpuFrameBuffer& setDepthStencilAttachment(int w, int h);
	GpuFrameBuffer& setDepthStencilAttachment(std::shared_ptr<GpuTexture2D> texture);
	
	bool checkCompletness();
	void bind();

private:
	GLuint m_fbo;
	GLuint m_depthRenderBuffer;
	std::shared_ptr<GpuTexture2D> m_depthRenderTexture;
	std::vector<std::shared_ptr<GpuTexture>> m_textures;
	std::vector<GLuint> m_renderBuffers;
	bool m_completed;
};