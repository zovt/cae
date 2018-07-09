#include "input_handler.hh"

#include <iostream>
#include <cstdio>

#include "graphics/opengl/index.hh"
#define DEBUG_NAMESPACE "input"
#include "debug.hh"

using namespace input_handler;
using namespace common_state;

MouseButton convert_glfw_mouse_button(int glfw_button) {
	return
		glfw_button == GLFW_MOUSE_BUTTON_1 ? MouseButton::Mouse1
		: glfw_button == GLFW_MOUSE_BUTTON_2 ? MouseButton::Mouse2
		: glfw_button == GLFW_MOUSE_BUTTON_3 ? MouseButton::Mouse3
		: (MouseButton)0;
}

UpDownState convert_glfw_action(int action) {
	return
		action == GLFW_RELEASE ? UpDownState::Up
		: UpDownState::Down;
}

int convert_glfw_mod_mask(int mods) {
	int mask = 0;
	mask = mask | ((mods & GLFW_MOD_CONTROL) ? (int)Modifier::Ctrl : 0);
	mask = mask | ((mods & GLFW_MOD_SHIFT) ? (int)Modifier::Shift : 0);
	mask = mask | ((mods & GLFW_MOD_ALT) ? (int)Modifier::Alt : 0);
	mask = mask | ((mods & GLFW_MOD_SUPER) ? (int)Modifier::Super : 0);
	return mask;
}

void window_change_cb(GLFWwindow* window, int width, int height) {
	(void)window;
	active_input_handler->handle_window_change(width, height);
}

void mouse_scroll_cb(GLFWwindow* window, double x_off, double y_off) {
	(void)window;
	active_input_handler->handle_mouse_scroll(x_off, y_off);
}

void char_cb(GLFWwindow* window, unsigned int codepoint, int mods) {
	(void)window;

	active_input_handler->handle_char(codepoint, convert_glfw_mod_mask(mods));
}

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window;
	(void)scancode;

	active_input_handler->handle_key(key, convert_glfw_action(action), convert_glfw_mod_mask(mods));
}

void mouse_button_cb(GLFWwindow* window, int button, int action, int mods) {
	(void)window;

	active_input_handler->handle_mouse_button(convert_glfw_mouse_button(button), convert_glfw_action(action), convert_glfw_mod_mask(mods));
}

void cursor_pos_cb(GLFWwindow* window, double xpos, double ypos) {
	(void)window;

	active_input_handler->handle_cursor_pos(xpos, ypos);
}

void window_pos_cb(GLFWwindow* window, int xpos, int ypos) {
	(void)window;
	(void)xpos;
	(void)ypos;

	active_input_handler->buffer_draw_info.needs_redraw = true;
}

void InputHandler::glfw_register_callbacks(GLFWwindow* window) {
	glfwSetScrollCallback(window, mouse_scroll_cb);
	glfwSetCharModsCallback(window, char_cb);
	glfwSetKeyCallback(window, key_cb);
	glfwSetCursorPosCallback(window, cursor_pos_cb);
	glfwSetMouseButtonCallback(window, mouse_button_cb);
	glfwSetWindowPosCallback(window, window_pos_cb);
}

void InputHandler::make_active() {
	active_input_handler = this;
}

void InputHandler::handle_mouse_button(MouseButton button, UpDownState state, int mod_mask) {
	this->mod_mask = 0;
	switch (state) {
		case UpDownState::Down:
			this->mouse_button_mask = this->mouse_button_mask | (int)button;
			this->mod_mask = this->mod_mask | mod_mask;
			break;
		case UpDownState::Up:
			this->mouse_button_mask = this->mouse_button_mask & ~(int)button;
			break;
	}

	this->resolve();
}

void InputHandler::handle_mouse_scroll(double off_x, double off_y) {
	this->scroll_st.off_x = off_x;
	this->scroll_st.off_y = off_y;
	this->resolve();
}

void InputHandler::handle_window_change(int width, int height) {
	this->window_change_st.width = width;
	this->window_change_st.height = height;
	this->resolve();
}

void InputHandler::handle_key(int key, UpDownState state, int mod_mask) {
	if (state == UpDownState::Up) {
		return;
	}
	switch (key) {
		case GLFW_KEY_BACKSPACE:
			this->buffer.backspace();
			this->buffer_draw_info.needs_redraw = true;
			break;
		case GLFW_KEY_ENTER:
			this->buffer.insert('\n');
			this->buffer_draw_info.needs_redraw = true;
			break;
		case GLFW_KEY_Z:
			if (mod_mask & (int)Modifier::Ctrl) {
				this->buffer.undo();
				this->buffer_draw_info.needs_redraw = true;
			}
			break;
		case GLFW_KEY_R:
			if (mod_mask & (int)Modifier::Ctrl) {
				this->buffer.redo();
				this->buffer_draw_info.needs_redraw = true;
			}
			break;
		case GLFW_KEY_S:
			if (mod_mask & (int)Modifier::Ctrl) {
				buffer.save();
			}
			break;
	}
}

void InputHandler::handle_char(uint8_t codepoint, int mod_mask) {
	this->mod_mask = this->mod_mask | mod_mask;
	this->buffer.insert(codepoint);
	this->buffer_draw_info.needs_redraw = true;
}

void InputHandler::handle_cursor_pos(double pos_x, double pos_y) {
	this->cursor_pos_st_current.pos_x = pos_x;
	this->cursor_pos_st_current.pos_y = pos_y;
	this->resolve();
}

void print_mouse_buttons(int mouse_mask) {
	dbg_print("");
	fprintf(stderr, "%d ", mouse_mask);
	if (mouse_mask & (int)MouseButton::Mouse1) {
		fprintf(stderr, "Mouse1 ");
	}
	if (mouse_mask & (int)MouseButton::Mouse2) {
		fprintf(stderr, "Mouse2 ");
	}
	if (mouse_mask & (int)MouseButton::Mouse3) {
		fprintf(stderr, "Mouse3 ");
	}
	fprintf(stderr, "\n");
}

void print_modifiers(int mod_mask) {
	dbg_print("");
	fprintf(stderr, "%d ", mod_mask);
	if (mod_mask & (int)Modifier::Ctrl) {
		fprintf(stderr, "Ctrl ");
	}
	if (mod_mask & (int)Modifier::Shift) {
		fprintf(stderr, "Shift ");
	}
	if (mod_mask & (int)Modifier::Alt) {
		fprintf(stderr, "Alt ");
	}
	if (mod_mask & (int)Modifier::Super) {
		fprintf(stderr, "Super ");
	}
	fprintf(stderr, "\n");
}

// FIXME: This call should be synchronous, so just handle things in the
// associated handler
void InputHandler::resolve() {
	DEBUG_ONLY(
	dbg_println("Resolving events");
	print_mouse_buttons(this->mouse_button_mask);
	print_modifiers(this->mod_mask);
	);
	if ((this->scroll_st.off_x != 0) || (this->scroll_st.off_y != 0)) {
		DEBUG_ONLY(
		std::cerr << "Scrolling" << std::endl;
		);
		this->buffer_draw_info.scroll(this->scroll_st);
		this->scroll_st = {};
	}

	if (this->window_change_st.width > 0) {
		this->buffer_draw_info.resize_window(this->window_change_st.width, this->window_change_st.height);
		this->window_change_st = {};
	}

	auto before = this->cursor_pos_st_old;
	this->cursor_pos_st_old = this->cursor_pos_st_current;

	if ((this->mouse_button_mask & (int)MouseButton::Mouse1)
		&& (this->mod_mask & (int)Modifier::Ctrl)) {
		this->buffer_draw_info.scroll_drag(ScrollState {
			(this->cursor_pos_st_current.pos_x - before.pos_x),
			(this->cursor_pos_st_current.pos_y - before.pos_y)
		});
	} else if (this->mouse_button_mask & (int)MouseButton::Mouse1) {
		auto point_pos = this->buffer_draw_info.get_mouse_target(
			this->buffer,
			this->cursor_pos_st_current
		);
		this->buffer.set_point(point_pos);
	}
}
