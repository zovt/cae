#version 150

in vec3 position;
out vec3 f_color;

void main() {
	gl_Position = vec4(position, 1.0);
	f_color = vec3(1.0, 0.0, 0.0);
}
