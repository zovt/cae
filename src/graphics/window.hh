#pragma once

#include <cstdint>

#include <GLFW/glfw3.h>

namespace window {

struct Window {
	GLFWwindow* handle;
	int width;
	int height;

	Window(int width, int height, const char* name);
	~Window();
};

}
