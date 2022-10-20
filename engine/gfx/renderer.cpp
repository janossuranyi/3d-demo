#include "renderer.h"

namespace gfx {

	void View::clear() {
		clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		render_items.clear();
		frame_buffer = FrameBufferHandle::invalid;
	}

	void RenderItem::encode_state() {
        encoded_state = 0ull;
        if (scissor)    encoded_state |= (1ULL << SCISSOR_SHIFT);
        if (depth_test) encoded_state |= (1ULL << DEPTHTEST_SHIFT);
        if (cull_face)  encoded_state |= (1ULL << CULLFACE_SHIFT);
        if (color_mask) encoded_state |= (1ULL << COLOR_MASK_SHIFT);
        if (depth_mask) encoded_state |= (1ULL << DEPTH_MASK_SHIFT);

        encoded_state |= (uint64_t(cull_front_face) << CULLFACE_SHIFT) & CULLFACE_MASK;
        encoded_state |= (uint64_t(polygon_mode) << POLYGON_MODE_SHIFT) & POLYGON_MODE_MASK;
        encoded_state |= (uint64_t(blend_eq_color) << BLEND_EQ_SHIFT) & BLEND_EQ_MASK;
        encoded_state |= (uint64_t(blend_eq_alpha) << BLEND_EQ_A_SHIFT) & BLEND_EQ_A_MASK;
        encoded_state |= (uint64_t(blend_src_color) << BLEND_FUNC_SRC_SHIFT) & BLEND_FUNC_SRC_MASK;
        encoded_state |= (uint64_t(blend_src_alpha) << BLEND_FUNC_SRC_A_SHIFT) & BLEND_FUNC_SRC_A_MASK;
        encoded_state |= (uint64_t(blend_dst_color) << BLEND_FUNC_DST_SHIFT) & BLEND_FUNC_DST_MASK;
        encoded_state |= (uint64_t(blend_dst_alpha) << BLEND_FUNC_DST_A_SHIFT) & BLEND_FUNC_DST_A_MASK;

	}

}
