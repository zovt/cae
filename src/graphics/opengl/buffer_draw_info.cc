#include "buffer_draw_info.hh"

#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define DEBUG_NAMESPACE "drawing"
#include "../../debug.hh"

using namespace graphics::opengl::buffer_draw_info;
using namespace graphics::opengl::uniforms;
using namespace buffer;
using namespace common_state;
using namespace graphics::fonts;

int get_x_in_line(
	size_t index,
	std::vector<uint8_t> const& contents,
	std::vector<Metrics> const& char_to_metrics,
	int space_width,
	int tab_size
) {
	int offset = 0;
	for (size_t i = index; i < contents.size(); i--) {
		auto chr = contents[i];
		dbg_printval(chr);
		if (chr == '\n') {
			break;
		}
		if (chr == '\t') {
			offset += space_width * tab_size;
			continue;
		}
		if (chr == ' ') {
			offset += space_width;
			continue;
		}
		offset += char_to_metrics[chr].advance;
	}
	return offset;
}

size_t find_newline_after(size_t index, std::vector<uint8_t> const& contents) {
	for (; index < contents.size(); ++index) {
		if (contents[index] == '\n') {
			return index;
		}
	}
	return index;
}

void BufferDrawInfo::draw(Buffer const& buffer) const {
	dbg_println("starting draw text");
	dbg_printval(line_height);
	dbg_printval(space_width);

	int line_height_adj = (float)this->line_height * 1.2f;
	int cursor_x = 0;
	int cursor_y = line_height_adj;
	auto point_pos_x = 0;
	auto point_pos_y = 0;

	auto selection_min = std::min(buffer.point.point, buffer.point.mark);
	auto selection_max = std::max(buffer.point.point, buffer.point.mark);
	auto& bg_uni = always.rest.rest.rest.rest.uni;

	glClearColor(bg_clear_color.red / 255.f, bg_clear_color.green / 255.f, bg_clear_color.blue / 255.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	int newlines_before = 0;
	for (size_t i = 0; i < selection_min; i++) {
		if (buffer.contents[i] == '\n') {
			++newlines_before;
		}
	}
	int newlines_in = 0;
	for (size_t i = selection_min; i < selection_max; i++) {
		if (buffer.contents[i] == '\n') {
			++newlines_in;
		}
	}

	int selection_start_x = get_x_in_line(selection_min - 1, buffer.contents, char_to_metrics, space_width, tab_size);
	int selection_end_x = get_x_in_line(selection_max - 1, buffer.contents, char_to_metrics, space_width, tab_size);
	int selection_start_y = newlines_before * line_height_adj + line_height_adj;
	int selection_end_y = selection_start_y + newlines_in * line_height_adj;

	point_shdr.activate();
	always.activate(point_shdr.program);
	VecUniform<GLuint, 3> rect_color = {selection_color.data, "text_fg"};
	rect_color.activate(point_shdr.program);
	auto n_lines = newlines_in;
	auto current_index = selection_min;
	for (int i = 0; i <= n_lines; i++) {
		auto rect_start_x = i == 0 ? selection_start_x : 0;
		auto rect_start_y = selection_start_y + (i * line_height_adj) - line_height_adj;
		auto rect_end_x = selection_end_x;
		if (i != n_lines) {
			rect_end_x = get_x_in_line(find_newline_after(current_index, buffer.contents) - 1, buffer.contents, char_to_metrics, space_width, tab_size);
		}
		auto rect_end_y = selection_start_y + i * line_height_adj;
		auto rect_width = rect_end_x - rect_start_x;
		auto rect_height = rect_end_y - rect_start_y;

		auto transform = glm::scale(
			glm::translate(glm::mat4(1.f), {(float)rect_start_x, (float)rect_start_y, -0.2f}),
			{rect_width, rect_height, 1.f}
		);
		TransformUniform transform_uni{transform, "transform"};
		transform_uni.activate(point_shdr.program);
		tex_pixel.draw();
		current_index = 1 + find_newline_after(current_index, buffer.contents);
	}


	text_shdr.activate();
	always.activate(this->text_shdr.program);
	tex_pixel.vao.activate();
	for (size_t i = 0; i < buffer.contents.size(); i++) {
		auto chr = buffer.contents[i];
		if (i == buffer.point.point) {
			point_pos_x = cursor_x;
			point_pos_y = cursor_y;
		}

		if (i == selection_min) {
			selection_color.activate(text_shdr.program);
			selection_start_x = cursor_x;
			selection_start_y = cursor_y;
		}

		if (i == selection_max) {
			bg_uni.activate(text_shdr.program);
			selection_end_x = cursor_x;
			selection_end_y = cursor_y;
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

		#ifdef CAE_DRAW_DEBUG
		dbg_println("Drawing char '%c'", chr);
		dbg_printval(x_coord);
		dbg_printval(y_coord);
		dbg_printval(width);
		dbg_printval(height);
		#endif
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
	if (buffer.point.point == buffer.contents.size()) {
		point_pos_x = cursor_x;
		point_pos_y = cursor_y;
	}

	point_shdr.activate();
	always.activate(point_shdr.program);
	auto transform = glm::scale(
		glm::translate(glm::mat4(1.f), {(float)point_pos_x, (float)point_pos_y - line_height, 0.2f}),
		{2.f, this->line_height, 1.f}
	);
	TransformUniform transform_uni{transform, "transform"};
	transform_uni.activate(this->point_shdr.program);
	this->tex_pixel.draw();

	glfwSwapBuffers(this->window.handle);
}

void BufferDrawInfo::resize_window(int width, int height) {
	dbg_println("_____________");
	dbg_println("resize_window");
	this->window.width = width;
	this->window.height = height;
	dbg_printval(width);
	dbg_printval(height);
	glViewport(0, 0, width, height);
	this->always.uni.regen_proj(width, height);
	this->needs_redraw = true;
}

void BufferDrawInfo::scroll(ScrollState offsets) {
	this->always.uni.world = glm::translate(
		this->always.uni.world,
		{
			offsets.x_off * this->tab_size * this->space_width,
			offsets.y_off * this->line_height * 4,
			0.f
		}
	);
	this->needs_redraw = true;
}

void BufferDrawInfo::scroll_drag(ScrollState offsets) {
	this->always.uni.world = glm::translate(
		this->always.uni.world,
		{
			offsets.x_off,
			offsets.y_off,
			0.f
		}
	);
	this->needs_redraw = true;
}

size_t BufferDrawInfo::get_mouse_target(Buffer const& buffer, CursorPosState state) {
	if (buffer.contents.size() == 0) {
		return 0;
	}

	auto x = state.x_pos;
	auto y = state.y_pos;
	glm::vec4 orig{x, y, 0.f, 1.f};
	auto world_inv_tr = glm::transpose(glm::inverse(
		this->always.uni.world
	));
	auto offset = (orig * world_inv_tr);

	int line_height_adj = (float)this->line_height * 1.2f;
	auto n_lines = (int)((float)offset.y / line_height_adj);
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
	return point_offset;
}
