#pragma once

#include <vector>
#include <functional>

#include "../fonts.hh"
#include "../window.hh"
#include "../../buffer.hh"
#include "../../common_state.hh"
#include "../../color.hh"
#include "index.hh"
#include "drawing.hh"
#include "shaders.hh"
#include "uniforms.hh"

namespace graphics { namespace opengl { namespace buffer_draw_info {

struct BufferDrawInfo {
	drawing::DrawInfo const& tex_pixel;
	shaders::Program const& text_shdr;
	shaders::Program const& point_shdr;

	uniforms::UniformGroup<
		uniforms::GlobalDrawingUniforms&,
		uniforms::TextureUniform&,
		uniforms::BufferTextureUniform&,
		uniforms::VecUniform<GLuint, 3>&,
		uniforms::VecUniform<GLuint, 3>&
	>& always;
	window::Window& window;
	std::vector<fonts::Metrics> const& char_to_metrics;
	int space_width;
	int tab_size;
	int line_height;
	bool needs_redraw;
	color::RGBColor bg_clear_color;

	void draw(buffer::Buffer const& buffer) const;
	void scroll(common_state::ScrollState offsets);
	void scroll_drag(common_state::ScrollState offsets);
	void resize_window(int width, int height);
	buffer::PointOffset get_mouse_target(buffer::Buffer const& buffer, common_state::CursorPosState state);
};

} } }
