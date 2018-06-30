#pragma once

#include "graphics/opengl/buffer_draw_info.hh"
#include "buffer.hh"

namespace input_handler {

enum struct UpDownState {
	Up,
	Down,
};

struct KeyState {
	int key;
	UpDownState state;
};

enum struct MouseButton {
	Mouse1 = 1,
	Mouse2 = 1 << 1,
	Mouse3 = 1 << 2,
};

enum struct Modifier {
	Ctrl = 1,
	Shift = 1 << 1,
	Alt = 1 << 2,
	Super = 1 << 3
};

struct WindowChangeState {
	int width;
	int height;
};

struct InputHandler {
	buffer::Buffer& buffer;
	graphics::opengl::buffer_draw_info::BufferDrawInfo& buffer_draw_info;

	int mouse_button_mask;
	int mod_mask;
	common_state::ScrollState scroll_st;
	WindowChangeState window_change_st;
	common_state::CursorPosState cursor_pos_st_old;
	common_state::CursorPosState cursor_pos_st_current;

	void make_active();
	void resolve();
	void handle_mouse_button(MouseButton button, UpDownState state, int mod_mask);
	void handle_mouse_scroll(double x_off, double y_off);
	void handle_window_change(int width, int height);
	void handle_char(uint8_t codepoint, int mod_mask);
	void handle_cursor_pos(double xpos, double ypos);

	static void glfw_register_callbacks(GLFWwindow* window);
};

static InputHandler* active_input_handler = nullptr;

}
