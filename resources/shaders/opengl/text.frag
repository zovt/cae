#version 430 core

in vec3 f_color;
in vec2 f_uv;

uniform sampler2D tex;

out vec4 out_color;

void main() {
	out_color = vec4(texture(tex, f_uv).xyz, 1.0);
}
