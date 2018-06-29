#pragma once

#include <vector>
#include <functional>

#include "../fonts.hh"
#include "../window.hh"
#include "../../buffer.hh"
#include "../../common_state.hh"
#include "index.hh"
#include "drawing.hh"
#include "shaders.hh"
#include "uniforms.hh"

namespace graphics { namespace opengl { namespace buffer_draw_info {

struct BufferDrawInfo {
	drawing::DrawInfo const& tex_pixel;
	shaders::Program const& text_shdr;

	uniforms::UniformGroup<
		uniforms::GlobalDrawingUniforms&,
		uniforms::TextureUniform&,
		uniforms::BufferTextureUniform&
	>& always;
	window::Window& window;
	std::vector<fonts::Metrics> const& char_to_metrics;
	int space_width;
	int tab_size;
	int line_height;
	bool needs_redraw;

	void draw(buffer::Buffer const& buffer) const;
	void scroll(common_state::ScrollState offsets);
	void scroll_drag(common_state::ScrollState offsets);
	void resize_window(int width, int height);
};

} } }
