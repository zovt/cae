#pragma once

#include <cstdint>

#include "opengl/index.hh"

namespace window {

struct Window {
	GLFWwindow* handle;
	int width;
	int height;

	Window(int width, int height, const char* name);
	~Window();
};

}
