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
	auto point_pos_x = 0;
	auto point_pos_y = 0;

	// FIXME: Make this configurable
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	this->text_shdr.activate();
	this->always.activate(this->text_shdr.program);
	this->tex_pixel.vao.activate();
	for (size_t i = 0; i < buffer.contents.size(); i++) {
		auto chr = buffer.contents[i];
		if (i == buffer.point.index) {
			point_pos_x = cursor_x;
			point_pos_y = cursor_y;
		}

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
	if (buffer.point.index == buffer.contents.size()) {
		point_pos_x = cursor_x;
		point_pos_y = cursor_y;
	}

	this->point_shdr.activate();
	auto transform = glm::scale(
		glm::translate(glm::mat4(1.f), {(float)point_pos_x, (float)point_pos_y - this->line_height, 0.f}),
		{2.f, this->line_height, 1.f}
	);
	TransformUniform transform_uni{transform, "transform"};
	transform_uni.activate(this->point_shdr.program);
	this->always.activate(this->point_shdr.program);
	this->tex_pixel.draw();

	glfwSwapBuffers(this->window.handle);
}

void BufferDrawInfo::resize_window(int width, int height) {
	this->window.width = width;
	this->window.height = height;
	this->always.uni.regen_proj(width, height);
	this->needs_redraw = true;
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
	this->needs_redraw = true;
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
	this->needs_redraw = true;
}

PointOffset BufferDrawInfo::get_mouse_target(Buffer const& buffer, CursorPosState state) {
	if (buffer.contents.size() == 0) {
		return { 0 };
	}

	auto x = state.pos_x;
	auto y = state.pos_y;
	glm::vec4 orig{x, y, 0.f, 1.f};
	auto world_inv_tr = glm::transpose(glm::inverse(
		this->always.uni.world
	));
	auto offset = (orig * world_inv_tr);

	auto n_lines = (int)((float)offset.y / ((float)this->line_height * 1.2));
	size_t point_offset = 0;
	for (; point_offset < buffer.contents.size(); ++point_offset) {
		if (n_lines <= 0) {
			break;
		}
		auto chr = buffer.contents[point_offset];
		if (chr == '\n') {
			--n_lines;
		}
	}
	auto x_rem = offset.x;
	while (
		x_rem >= 0 && buffer.contents[point_offset] != '\n'
		&& point_offset < buffer.contents.size()
	) {
		auto chr = buffer.contents[point_offset];
		if (chr == ' ') {
			x_rem -= this->space_width;
		} else if (chr == '\t') {
			x_rem -= this->space_width * this->tab_size;
		} else {
			auto const& metrics = this->char_to_metrics[chr];
			x_rem -= metrics.advance;
		}
		++point_offset;
	}
	if (x_rem <= 0) {
		--point_offset;
	}
	this->needs_redraw = true;
	return {point_offset};
}
