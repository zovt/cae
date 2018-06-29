#include "buffer_draw_info.hh"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../macros.hh"

using namespace graphics::opengl::buffer_draw_info;
using namespace graphics::opengl::uniforms;
using namespace buffer;
using namespace common_state;

void BufferDrawInfo::draw(Buffer const& buffer) const {
	DEBUG_ONLY(
	std::cerr << "STARTING DRAW TEXT" << std::endl;
	std::cerr << "line_height: " << this->line_height << std::endl;
	std::cerr << "space_width: " << this->space_width << std::endl;
	);
	int line_height_adj = (float)this->line_height * 1.2f;
	int cursor_x = 0;
	int cursor_y = line_height_adj;


	// FIXME: Make this configurable
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	always.activate(this->text_shdr.program);
	this->tex_pixel.vao.activate();
	for (auto chr : buffer.contents) {
		if (chr == ' ') {
			cursor_x += this->space_width;
			continue;
		}
		if (chr == '\t') {
			cursor_x += this->space_width * this->tab_size;
			continue;
		}
		if (chr == '\n') {
			cursor_x = 0;
			cursor_y += line_height_adj;
			continue;
		}

		auto const metrics = this->char_to_metrics[chr];
		int x_coord = cursor_x + metrics.offset_x;
		int y_coord = cursor_y - metrics.offset_y;
		int width = metrics.width;
		int height = metrics.height;

		DEBUG_ONLY(
		std::cerr << "Drawing char '" << chr << "'" << std::endl;
		std::cerr << "x_coord " << x_coord << " y_coord " << y_coord << std::endl;
		std::cerr << "width " << width << " height " << height << std::endl;
		);
		auto transform = glm::scale(
			glm::translate(glm::mat4(1.f), {(float)x_coord, (float)y_coord, 0.f}),
			{width, height, 1.f}
		);

		TransformUniform transform_uni{transform, "transform"};
		SimpleUniform<uint32_t, GLuint> chr_uni{(uint32_t)chr, "chr", glUniform1ui};
		UniformGroup unis{std::move(transform_uni), std::move(chr_uni)};
		unis.activate(this->text_shdr.program);
		this->tex_pixel.draw();

		cursor_x += metrics.advance;
	}

	glfwSwapBuffers(this->window.handle);
}

void BufferDrawInfo::resize_window(int width, int height) {
	this->window.width = width;
	this->window.height = height;
	this->always.uni.regen_proj(width, height);
}

void BufferDrawInfo::scroll(ScrollState offsets) {
	this->always.uni.world = glm::translate(
		this->always.uni.world,
		{
			offsets.off_x * this->tab_size * this->space_width,
			offsets.off_y * this->line_height * 4,
			0.f
		}
	);
}

void BufferDrawInfo::scroll_drag(ScrollState offsets) {
	this->always.uni.world = glm::translate(
		this->always.uni.world,
		{
			offsets.off_x,
			offsets.off_y,
			0.f
		}
	);
}
