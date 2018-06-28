#version 430 core

layout(location = 0)
in vec3 pos;

layout(location = 1)
in vec3 color;

layout(location = 2)
in vec2 uv;

layout(location = 0)
uniform mat4 proj;

uniform mat4 world;
uniform mat4 transform;

out vec3 f_color;
out vec2 f_uv;

void main() {
	gl_Position = proj * world * transform * vec4(pos, 1.0);
	f_uv = uv;
	f_color = color;
}
