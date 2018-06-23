#include "window.hh"

using namespace window;

Window::Window(int width, int height, const char* name) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->handle = glfwCreateWindow(width, height, name, nullptr, nullptr);
	this->width = width;
	this->height = height;
}

Window::~Window() {
	glfwDestroyWindow(this->handle);
	glfwTerminate();
}
