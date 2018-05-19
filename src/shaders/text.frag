#version 330 core

in vec3 f_color;
in vec2 f_uv;
out vec4 color;

uniform sampler2D glyph;
uniform vec3 fg;
uniform vec3 bg;

vec4 lerp(vec4 a, vec4 b, float ratio) {
	return vec4(
		(1.0 - ratio) * a.x + b.x * ratio,
		(1.0 - ratio) * a.y + b.y * ratio,
		(1.0 - ratio) * a.z + b.z * ratio,
		(1.0 - ratio) * a.w + b.w * ratio
	);
}

void main() {
	color = lerp(vec4(bg, 1.0), vec4(fg, 1.0), texture(glyph, f_uv).x);
}
