#include "window.hh"

using namespace window;

Window::Window(int width, int height, const char* name) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	this->handle = glfwCreateWindow(width, height, name, nullptr, nullptr);

	glfwMakeContextCurrent(this->handle);

	glewExperimental = GL_TRUE;
	glewInit();

	this->width = width;
	this->height = height;
}

Window::~Window() {
	glfwDestroyWindow(this->handle);
	glfwTerminate();
}
