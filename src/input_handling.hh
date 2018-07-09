#pragma once

#include <variant>

#include "graphics/opengl/buffer_draw_info.hh"
#include "buffer.hh"

namespace input_handling {

enum struct UpDownState {
	Up,
	Down,
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

struct MouseButtonEvent {
	MouseButton button;
	int mods;
	UpDownState state;
};

struct MouseScrollEvent {
	double x_off;
	double y_off;
};

struct MousePositionEvent {
	double x_pos;
	double y_pos;
};

struct MouseEvent {
	std::variant<MouseButtonEvent, MouseScrollEvent, MousePositionEvent> event;
};

struct KeyEvent {
	int key;
	int mods;
	UpDownState state;
};

struct CharEvent {
	unsigned int codepoint;
	int mods;
};

struct WindowMoveEvent {};

struct WindowFramebufferSizeEvent {
	int width;
	int height;
};

struct WindowEvent {
	std::variant<WindowMoveEvent, WindowFramebufferSizeEvent> event;
};

struct Event {
	std::variant<MouseEvent, CharEvent, KeyEvent, WindowEvent> event;
};

// state that needs to be held cross-frame
struct PersistentState {
	// TODO: Do these need to persist?
	int mod_mask;
	int mouse_mask;

	common_state::CursorPosState last_cursor_pos;
	bool dragging;
};

void handle_mouse_event(
	MouseEvent event,
	buffer::Buffer& buffer,
	graphics::opengl::buffer_draw_info::BufferDrawInfo& draw_info,
	PersistentState& state
);

bool handle_char_event(
	CharEvent event,
	buffer::Buffer& buffer
);

bool handle_key_event(
	KeyEvent event,
	buffer::Buffer& buffer,
	GLFWwindow* window
);

void handle_window_event(
	WindowEvent event,
	graphics::opengl::buffer_draw_info::BufferDrawInfo& draw_info
);

bool handle_event(
	Event event,
	buffer::Buffer& buffer,
	graphics::opengl::buffer_draw_info::BufferDrawInfo& draw_info,
	PersistentState& state
);

extern std::vector<Event> event_queue;
Event event_queue_pop();

void glfw_register_callbacks(GLFWwindow* window);

}
