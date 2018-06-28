#version 430 core

in vec3 f_color;
in vec2 f_uv;

uniform sampler2D font_map;
uniform samplerBuffer char_to_uv_locations;

uniform uint chr;
uniform vec3 bg;
uniform vec3 fg;

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

void main() {
	UVLocation uv_location = get_uv_location(chr);
	vec2 uv_shifted = vec2(
		shift_range(f_uv.x, vec2(uv_location.u_min, uv_location.u_max)),
		shift_range(f_uv.y, vec2(uv_location.v_min, uv_location.v_max))
	);

	float tex_color = texture2D(font_map, uv_shifted).x;
	out_color = vec4(tex_color, tex_color, tex_color, 1.0);
}
