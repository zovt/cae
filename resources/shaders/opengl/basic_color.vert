#version 150

in vec2 position;
in vec3 color;
out vec3 f_color;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
	f_color = color;
}
