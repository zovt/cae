#include "input_handling.hh"

#include <iostream>
#include <cstdio>

#include "graphics/opengl/index.hh"
#define DEBUG_NAMESPACE "input"
#include "debug.hh"

using namespace input_handling;
using namespace common_state;
using namespace buffer;
using namespace graphics::opengl::buffer_draw_info;

MouseButton convert_glfw_mouse_button(int glfw_button) {
	return
		glfw_button == GLFW_MOUSE_BUTTON_1 ? MouseButton::Mouse1
		: glfw_button == GLFW_MOUSE_BUTTON_2 ? MouseButton::Mouse2
		: glfw_button == GLFW_MOUSE_BUTTON_3 ? MouseButton::Mouse3
		: (MouseButton)0;
}

UpDownState convert_glfw_action(int action) {
	return action == GLFW_RELEASE ? UpDownState::Up : UpDownState::Down;
}

int convert_glfw_mod_mask(int mods) {
	int mask = 0;
	mask = mask | ((mods & GLFW_MOD_CONTROL) ? (int)Modifier::Ctrl : 0);
	mask = mask | ((mods & GLFW_MOD_SHIFT) ? (int)Modifier::Shift : 0);
	mask = mask | ((mods & GLFW_MOD_ALT) ? (int)Modifier::Alt : 0);
	mask = mask | ((mods & GLFW_MOD_SUPER) ? (int)Modifier::Super : 0);
	return mask;
}

std::vector<Event> input_handling::event_queue{};

void mouse_scroll_cb(GLFWwindow* window, double x_off, double y_off) {
	(void)window;

	event_queue.push_back({ MouseEvent { MouseScrollEvent {
		x_off, y_off
	}}});
}

void char_cb(GLFWwindow* window, unsigned int codepoint, int mods) {
	(void)window;

	event_queue.push_back({ CharEvent { codepoint, convert_glfw_mod_mask(mods) }});
}

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window;
	(void)scancode;

	event_queue.push_back({ KeyEvent{
		key,
		convert_glfw_mod_mask(mods),
		convert_glfw_action(action)
	}});
}

void mouse_button_cb(GLFWwindow* window, int button, int action, int mods) {
	(void)window;

	event_queue.push_back({ MouseEvent { MouseButtonEvent {
		convert_glfw_mouse_button(button),
		convert_glfw_mod_mask(mods),
		convert_glfw_action(action)
	}}});
}

void cursor_pos_cb(GLFWwindow* window, double x_pos, double y_pos) {
	(void)window;

	event_queue.push_back({ MouseEvent { MousePositionEvent {
		x_pos,
		y_pos
	}}});
}

void window_pos_cb(GLFWwindow* window, int xpos, int ypos) {
	(void)window;
	(void)xpos;
	(void)ypos;

	event_queue.push_back({ WindowEvent { WindowMoveEvent {} }});
}

void window_fb_sz_cb(GLFWwindow* window, int width, int height) {
	(void)window;

	event_queue.push_back({ WindowEvent { WindowFramebufferSizeEvent {
		width,
		height,
	}}});
}

Event input_handling::event_queue_pop() {
	auto event = event_queue.back();
	event_queue.pop_back();
	return event;
}

void input_handling::glfw_register_callbacks(GLFWwindow* window) {
	glfwSetScrollCallback(window, mouse_scroll_cb);
	glfwSetCharModsCallback(window, char_cb);
	glfwSetKeyCallback(window, key_cb);
	glfwSetCursorPosCallback(window, cursor_pos_cb);
	glfwSetMouseButtonCallback(window, mouse_button_cb);
	glfwSetWindowPosCallback(window, window_pos_cb);
	glfwSetFramebufferSizeCallback(window, window_fb_sz_cb);
}

bool input_handling::handle_mouse_event(
	MouseEvent event,
	buffer::Buffer& buffer,
	BufferDrawInfo& draw_info,
	PersistentState& state,
	GLFWwindow* window
) {
	if (std::holds_alternative<MouseButtonEvent>(event.event)) {
		auto button_event = std::get<MouseButtonEvent>(event.event);

		dbg_printval(state.mouse_mask);
		switch (button_event.state) {
			case UpDownState::Down: state.mouse_mask = state.mouse_mask | (int)button_event.button; break;
			case UpDownState::Up: state.mouse_mask = state.mouse_mask & ~(int)button_event.button; break;
		}
		dbg_printval(state.mouse_mask);
		switch (button_event.button) {
			case MouseButton::Mouse1: {
				if (button_event.state == UpDownState::Down) {
					if (button_event.mods & (int)Modifier::Ctrl) {
						state.dragging = true;
						return false;
					}
					auto point_pos = draw_info.get_mouse_target(buffer, state.last_cursor_pos);
					buffer.set_point({point_pos, point_pos});
					return true;
				} else {
					state.dragging = false;
					return false;
				}
				break;
			}
			case MouseButton::Mouse2: {
				if (button_event.state == UpDownState::Down) {
					if (state.mouse_mask & (int)MouseButton::Mouse1) {
						auto clipboard_str = glfwGetClipboardString(window);
						buffer.insert_all(gsl::span{(const uint8_t*)clipboard_str, strlen(clipboard_str)});
						return true;
					}
				}
				break;
			}
			case MouseButton::Mouse3: {
				if (button_event.state == UpDownState::Down) {
					if (state.mouse_mask & (int)MouseButton::Mouse1) {
						auto contents = buffer.get_selection();
						std::string content_str{(const char*)contents.data(), contents.size()};
						glfwSetClipboardString(window, content_str.c_str());
						buffer.backspace();
						return true;
					}
				}
				break;
			}
		}
		return false;
	} else if (std::holds_alternative<MouseScrollEvent>(event.event)) {
		auto scroll_event = std::get<MouseScrollEvent>(event.event);
		draw_info.scroll({ scroll_event.x_off, scroll_event.y_off });
		return true;
	} else if (std::holds_alternative<MousePositionEvent>(event.event)) {
		auto pos_event = std::get<MousePositionEvent>(event.event);
		bool needs_redraw = false;
		if (state.dragging) {
			draw_info.scroll_drag({
				pos_event.x_pos - state.last_cursor_pos.x_pos,
				pos_event.y_pos - state.last_cursor_pos.y_pos
			});
			needs_redraw = true;
		} else if (state.mouse_mask & (int)MouseButton::Mouse1) {
			auto point_pos = draw_info.get_mouse_target(buffer, {pos_event.x_pos, pos_event.y_pos});
			dbg_printval(point_pos);
			buffer.set_point({point_pos, buffer.point.mark});
			needs_redraw = true;
		}
		state.last_cursor_pos.x_pos = pos_event.x_pos;
		state.last_cursor_pos.y_pos = pos_event.y_pos;
		return needs_redraw;
	}
	return false;
}

bool input_handling::handle_char_event(
	CharEvent event,
	buffer::Buffer& buffer
) {
	buffer.insert(event.codepoint);
	return true;
}

bool input_handling::handle_key_event(
	KeyEvent event,
	buffer::Buffer& buffer,
	GLFWwindow* window
) {
	if (event.state == UpDownState::Up) {
		return false;
	}

	switch (event.key) {
		case GLFW_KEY_BACKSPACE:
			buffer.backspace();
			return true;
		case GLFW_KEY_ENTER:
			buffer.insert('\n');
			return true;
		case GLFW_KEY_TAB:
			buffer.insert('\t');
			return true;
		case GLFW_KEY_Z:
			if (event.mods & (int)Modifier::Ctrl) {
				buffer.undo();
				return true;
			}
			return false;
		case GLFW_KEY_R:
			if (event.mods & (int)Modifier::Ctrl) {
				buffer.redo();
				return true;
			}
			return false;
		case GLFW_KEY_S:
			if (event.mods & (int)Modifier::Ctrl) {
				buffer.save();
			}
			return false;
		case GLFW_KEY_V:
			if (event.mods & (int)Modifier::Ctrl) {
				auto clipboard_str = glfwGetClipboardString(window);
				buffer.insert_all(gsl::span{(const uint8_t*)clipboard_str, strlen(clipboard_str)});
				return true;
			}
			return false;
		case GLFW_KEY_C:
			if (event.mods & (int)Modifier::Ctrl) {
				auto contents = buffer.get_selection();
				std::string content_str{(const char*)contents.data(), contents.size()};
				glfwSetClipboardString(window, content_str.c_str());
			}
			return false;
		case GLFW_KEY_X:
			if (event.mods & (int)Modifier::Ctrl) {
				auto contents = buffer.get_selection();
				std::string content_str{(const char*)contents.data(), contents.size()};
				glfwSetClipboardString(window, content_str.c_str());
				buffer.backspace();
				return true;
			}
			return false;
		case GLFW_KEY_UP:
			buffer.point_up();
			return true;
		case GLFW_KEY_LEFT:
			buffer.point_left();
			return true;
		case GLFW_KEY_RIGHT:
			buffer.point_right();
			return true;
		case GLFW_KEY_DOWN:
			buffer.point_down();
			return true;
	}
	return false;
}

void input_handling::handle_window_event(
	WindowEvent event,
	BufferDrawInfo& draw_info
) {
	if (std::holds_alternative<WindowMoveEvent>(event.event)) {
		draw_info.needs_redraw = true;
	} else if (std::holds_alternative<WindowFramebufferSizeEvent>(event.event)) {
		auto fb_sz_event = std::get<WindowFramebufferSizeEvent>(event.event);
		draw_info.resize_window(fb_sz_event.width, fb_sz_event.height);
	}
}

bool input_handling::handle_event(
	Event event,
	Buffer& buffer,
	BufferDrawInfo& draw_info,
	PersistentState& state
) {
	if (std::holds_alternative<MouseEvent>(event.event)) {
		return handle_mouse_event(std::get<MouseEvent>(event.event), buffer, draw_info, state, draw_info.window.handle);
	} else if (std::holds_alternative<CharEvent>(event.event)) {
		auto char_event = std::get<CharEvent>(event.event);
		return handle_char_event(char_event, buffer);
	} else if (std::holds_alternative<KeyEvent>(event.event)) {
		auto key_event = std::get<KeyEvent>(event.event);
		return handle_key_event(key_event, buffer, draw_info.window.handle);
	} else if (std::holds_alternative<WindowEvent>(event.event)) {
		auto window_event = std::get<WindowEvent>(event.event);
		handle_window_event(window_event, draw_info);
		return false;
	}
	return false;
}
