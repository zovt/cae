#version 430 core

in vec3 position;
in vec3 color;

layout(location = 0)
uniform mat4 proj;

layout(location = 1)
uniform mat4 world;

layout(location = 2)
uniform mat4 transform;

out vec3 f_color;

void main() {
	gl_Position = proj * world * transform * vec4(position, 1.0);
	f_color = color;
}
