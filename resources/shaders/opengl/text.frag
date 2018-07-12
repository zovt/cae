#version 330 core

in vec3 f_color;
in vec2 f_uv;

uniform sampler2D font_map;
uniform samplerBuffer char_to_uv_locations;

uniform uint chr;
uniform uvec3 text_bg;
uniform uvec3 text_fg;

out vec4 out_color;

struct UVLocation {
	float u_min;
	float v_min;
	float u_max;
	float v_max;
};

UVLocation get_uv_location(uint off) {
	UVLocation ret;
	int offset = int(off);
	vec4 texel = texelFetch(char_to_uv_locations, offset);
	ret.u_min = texel.x;
	ret.v_min = texel.y;
	ret.u_max = texel.z;
	ret.v_max = texel.w;
	return ret;
};

float shift_range(float zero_to_one, vec2 min_to_max) {
	return (((min_to_max.y - min_to_max.x) * zero_to_one) + min_to_max.x);
}

float lerp(float a, float b, float amount) {
	return (a * (1 - amount)) + b * amount;
}

vec3 lerp(vec3 a, vec3 b, float amount) {
	return vec3(lerp(a.x, b.x, amount), lerp(a.y, b.y, amount), lerp(a.z, b.z, amount));
}

vec3 adj_rgb(uvec3 rgb) {
	return vec3(float(rgb.r) / 255.0, float(rgb.g) / 255.0, float(rgb.z) / 255.0);
}

void main() {
	UVLocation uv_location = get_uv_location(chr);
	vec2 uv_shifted = vec2(
		shift_range(f_uv.x, vec2(uv_location.u_min, uv_location.u_max)),
		shift_range(f_uv.y, vec2(uv_location.v_min, uv_location.v_max))
	);

	float glyph_brightness = texture2D(font_map, uv_shifted).x;

	vec3 text_fg_fixed = adj_rgb(text_fg);
	vec3 text_bg_fixed = adj_rgb(text_bg);

	out_color = vec4(
		text_fg_fixed,
		glyph_brightness
	);
}
