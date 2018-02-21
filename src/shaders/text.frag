#version 330 core

in vec3 f_color;
in vec2 f_uv;
out vec4 color;

uniform sampler2D glyph;

void main() {
	color = texture(glyph, f_uv) * vec4(f_color, 0.0);
}
