#version 330 core

layout(location = 0)
in vec3 pos;

uniform mat4 proj;
uniform mat4 world;
uniform mat4 transform;

void main() {
	gl_Position = proj * world * transform * vec4(pos, 1.0);
}
