#version 330 core

in vec3 f_color;
in vec2 f_uv;
out vec4 color;

uniform sampler2D glyph;

vec4 lerp(vec4 a, vec4 b, float ratio) {
	return vec4(
		(1.0 - ratio) * a.x + b.x * ratio,
		(1.0 - ratio) * a.y + b.y * ratio,
		(1.0 - ratio) * a.z + b.z * ratio,
		(1.0 - ratio) * a.w + b.w * ratio
	);
}

void main() {
	color = lerp(vec4(1.0, 1.0, 1.0, 1.0), vec4(f_color, 1.0), texture(glyph, f_uv).x);
}