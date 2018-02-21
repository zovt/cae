#version 330 core

in vec2 pos;
in vec2 uv;

uniform mat4 transform;
uniform mat4 world;
uniform mat4 proj;
uniform vec3 color;

out vec3 f_color;
out vec2 f_uv;

void main() {
	gl_Position = proj * world * transform * vec4(pos, 0.0, 1.0);
	f_color = color;
	f_uv = uv;
}
