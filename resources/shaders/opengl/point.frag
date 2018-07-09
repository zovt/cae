#version 430 core

out vec4 out_color;

uniform uvec3 text_fg;

vec3 adj_rgb(uvec3 rgb) {
	return vec3(float(rgb.r) / 255.0, float(rgb.g) / 255.0, float(rgb.b) / 255.0);
}

void main() {
	out_color = vec4(adj_rgb(text_fg), 1.0);
}
