#pragma once

#include <cinttypes>
#include <string>
#include <glm/glm.hpp>

#include "renderer.h"

namespace gfx {

	class RenderContext {
	public:
		virtual ~RenderContext() = default;
		virtual bool create_window(uint16_t w, uint16_t h, bool fullscreen, const std::string& name) = 0;
		virtual void destroy_window() = 0;
		virtual void set_state(StateBits stateBits, bool force) = 0;
		virtual void start_rendering() = 0;
		virtual void stop_rendering() = 0;
		virtual void process_command_list(const std::vector<RenderCommand>& cmds) = 0;
		virtual bool frame(const Frame* frame) = 0;
		virtual glm::ivec2 get_window_size() const = 0;
		virtual int red_bits() const = 0;
		virtual int green_bits() const = 0;
		virtual int blue_bits() const = 0;
		virtual int depth_bits() const = 0;
		virtual int stencil_bits() const = 0;
		virtual int uniform_offset_aligment() const = 0;
		virtual void* get_mapped_address(BufferHandle handle) = 0;

	};
} /* namespace gfx */