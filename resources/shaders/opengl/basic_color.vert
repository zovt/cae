#version 330 core

in vec3 position;
in vec3 color;

uniform mat4 proj;
uniform mat4 world;
uniform mat4 transform;

out vec3 f_color;

void main() {
	gl_Position = proj * world * transform * vec4(position, 1.0);
	f_color = color;
}
