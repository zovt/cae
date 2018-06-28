#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <fstream>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <filesystem>
#include <unistd.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <stb_image.h>
#include <stb_image_write.h>

#include "graphics/opengl/index.hh"
#include "graphics/opengl/drawing.hh"
#include "graphics/opengl/shaders.hh"
#include "graphics/opengl/primitives.hh"
#include "graphics/opengl/uniforms.hh"
#include "graphics/opengl/textures.hh"
#include "graphics/fonts.hh"
#include "graphics/window.hh"
#include "unit.hh"
#include "config.hh"
#include "color.hh"
#include "fonts.hh"
#include "defer.hh"
#include "macros.hh"
#include "resources/shaders/opengl/text.vert.hh"
#include "resources/shaders/opengl/text.frag.hh"
#include "resources/textures/test.png.hh"

using namespace config;
using namespace color;
using namespace fonts;
using namespace err;
using namespace defer;
using namespace window;
using namespace graphics::primitives;
using namespace graphics::fonts;
using namespace graphics::opengl::drawing;
using namespace graphics::opengl::shaders;
using namespace graphics::opengl::primitives;
using namespace graphics::opengl::uniforms;
using namespace graphics::opengl::textures;

static const Config conf(
	{"Iosevka"},
	RGBColor(255, 255, 255),
	RGBColor(30, 30, 30),
	2,
	18
);

DEBUG_ONLY(
void GLAPIENTRY message_cb(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
) {
	(void)source;
	(void)userParam;
	(void)length;

	std::cerr << "GL Message: "
		<< (type == GL_DEBUG_TYPE_ERROR ? "!! ERROR !! " : " ")
		<< "severity: " << severity
		<< " error_id: " << id
		<< " message: " << message << std::endl;
}
)

// FIXME: Dirty hacks to deal with c-style callbacks
// Probably best to create a global state struct that updates everything once
// per frame
bool window_size_changed = false;
int changed_width;
int changed_height;
void window_change_cb(GLFWwindow* window, int width, int height) {
	(void)window;

	window_size_changed = true;
	changed_width = width;
	changed_height = height;
	glViewport(0, 0, width, height);
}

template <typename T>
int8_t sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

bool mouse_scrolled = false;
double x_off;
double y_off;
int g_tab_width = 0;
int g_line_height = 0;
void mouse_scroll_cb(GLFWwindow* window, double x_off_p, double y_off_p) {
	(void)window;

	DEBUG_ONLY(
	std::cerr << "Mouse scrolled" << std::endl;
	);

	mouse_scrolled = true;
	x_off = sgn(x_off_p) * g_tab_width;
	y_off = sgn(y_off_p) * g_line_height * 4;
}

bool ctrl_down = false;
void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window;
	(void)key;
	(void)scancode;

	if (mods == GLFW_MOD_CONTROL && action == GLFW_PRESS) {
		ctrl_down = true;
	}

	if (mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) {
		ctrl_down = false;
	}
}

bool left_mouse_down = false;
void mouse_button_cb(GLFWwindow* window, int button, int action, int mods) {
	(void)window;

	if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
		ctrl_down = true;
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		left_mouse_down = true;
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
		left_mouse_down = false;
	}

	if (action == GLFW_RELEASE && mods == GLFW_MOD_CONTROL) {
		ctrl_down = false;
	}
}

bool screen_dragged = false;
double cursor_x = 0;
double cursor_y = 0;
double drag_delta_x = 0;
double drag_delta_y = 0;
void cursor_pos_cb(GLFWwindow* window, double xpos, double ypos) {
	(void)window;

	if (ctrl_down && left_mouse_down) {
		screen_dragged = true;
		drag_delta_x = xpos - cursor_x;
		drag_delta_y = ypos - cursor_y;
	} else {
		screen_dragged = false;
	}

	cursor_x = xpos;
	cursor_y = ypos;
}

void draw_text(
	std::string const& text,
	int space_width,
	int tab_size,
	int line_height,
	std::vector<Metrics> const& char_to_metrics,
	Program const& text_shdr,
	DrawInfo const& tex_pixel
) {
	DEBUG_ONLY(
	std::cerr << "STARTING DRAW TEXT" << std::endl;
	std::cerr << "line_height: " << line_height << std::endl;
	std::cerr << "space_width: " << space_width << std::endl;
	);
	int line_height_adj = (float)line_height * 1.2f;
	int cursor_x = 0;
	int cursor_y = line_height_adj;

	tex_pixel.vao.activate();
	for (auto chr : text) {
		if (chr == ' ') {
			cursor_x += space_width;
			continue;
		}
		if (chr == '\t') {
			cursor_x += space_width * tab_size;
			continue;
		}
		if (chr == '\n') {
			cursor_x = 0;
			cursor_y += line_height_adj;
			continue;
		}

		auto const metrics = char_to_metrics[chr];
		int x_coord = cursor_x + metrics.offset_x;
		int y_coord = cursor_y - metrics.offset_y;
		int width = metrics.width;
		int height = metrics.height;

		DEBUG_ONLY(
		std::cerr << "Drawing char '" << chr << "'" << std::endl;
		std::cerr << "x_coord " << x_coord << " y_coord " << y_coord << std::endl;
		std::cerr << "width " << width << " height " << height << std::endl;
		);
		auto transform = glm::scale(
			glm::translate(glm::mat4(1.f), {(float)x_coord, (float)y_coord, 0.f}),
			{width, height, 1.f}
		);

		TransformUniform transform_uni{transform, "transform"};
		SimpleUniform<uint32_t, GLuint> chr_uni{(uint32_t)chr, "chr", glUniform1ui};
		UniformGroup unis{std::move(transform_uni), std::move(chr_uni)};
		unis.activate(text_shdr.program);
		tex_pixel.draw();

		cursor_x += metrics.advance;
	}
}

std::string slurp(std::filesystem::path&& path) {
	std::string result;
	result.resize(std::filesystem::file_size(path));
	std::ifstream in(path.native(), std::ios::in | std::ios::binary);
	in.read(result.data(), result.size());
	return result;
}

Result<Unit> run(int argc, char* argv[]) {
	using namespace std::literals;
	try(auto font_path, get_closest_font_match(conf.fonts));

	if (argc < 2) {
		return "Missing file argument";
	}
	auto file_contents = slurp(std::filesystem::path(argv[1]));


	auto path_hash = std::hash<std::string>{}(font_path);
	auto size_hash = std::hash<int>{}(conf.font_size);
	auto hash_name = std::to_string(path_hash + (size_hash * 13));
	auto home_dir = std::getenv("HOME");
	if (home_dir == nullptr) {
		return "Home dir does not exist"sv;
	}
	auto data_dir = std::filesystem::path(home_dir) / ".local" / "share" / "cae";
	if (!std::filesystem::exists(data_dir)) {
		std::filesystem::create_directory(data_dir);
	}
	auto font_data_folder = data_dir / hash_name;

	auto tex_path = font_data_folder / "tex.bmp";
	auto uv_path = font_data_folder / "uv.dat";
	auto metrics_path = font_data_folder / "metrics.dat";
	auto md_path = font_data_folder / "meta.dat";

	CharMapData char_map_data;
	// FIXME: filesystem caching is broken
	if (true || !(
		std::filesystem::exists(font_data_folder)
		&& std::filesystem::exists(tex_path)
		&& std::filesystem::exists(uv_path)
		&& std::filesystem::exists(metrics_path)
		&& std::filesystem::exists(md_path)
	)) {
		std::filesystem::create_directory(font_data_folder);

		char_map_data = get_char_map_data(font_path, conf.font_size);

		stbi_write_bmp(tex_path.c_str(), char_map_data.md.image_width, char_map_data.md.image_height, 1, char_map_data.pixel_data.data());

		std::ofstream uv(uv_path.native(), std::ios::binary);
		uv.write((char const*)char_map_data.char_to_uv_locations.data(), char_map_data.char_to_uv_locations.size() * sizeof(UVLocation));

		std::ofstream metrics(metrics_path.native(), std::ios::binary);
		metrics.write((char const*)char_map_data.char_to_metrics.data(), char_map_data.char_to_metrics.size() * sizeof(Metrics));

		std::ofstream md(md_path.native(), std::ios::binary);
		md.write((char const*)(&char_map_data.md), sizeof(CharMapData::Metadata));
	} else {
		int tex_bmp_width;
		int tex_bmp_height;
		int tex_bmp_n;
		auto data = stbi_load(tex_path.c_str(), &tex_bmp_width, &tex_bmp_height, &tex_bmp_n, 1);
		char_map_data.pixel_data.assign((PixelData*)data, (PixelData*)(data + (tex_bmp_width * tex_bmp_height)));
		stbi_image_free(data);


		auto metrics_size = std::filesystem::file_size(metrics_path);
		std::ifstream metrics(metrics_path.native(), std::ios::binary);
		char_map_data.char_to_metrics.resize(metrics_size / sizeof(Metrics));
		metrics.read((char*)char_map_data.char_to_metrics.data(), metrics_size);

		auto uv_size = std::filesystem::file_size(uv_path);
		std::ifstream uv(metrics_path.native(), std::ios::binary);
		char_map_data.char_to_uv_locations.resize(uv_size / sizeof(UVLocation));
		uv.read((char*)char_map_data.char_to_uv_locations.data(), uv_size);

		std::ifstream md(md_path.native(), std::ios::binary);
		md.read((char*)(&char_map_data.md), sizeof(CharMapData::Metadata));
	}

	Window window(800, 600, "cae");

	DEBUG_ONLY(
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(message_cb, 0);
	)

	auto tex_pixel_tup = DrawInfo::make(tex_pixel_data, pixel_indices);
	auto tex_pixel = std::get<2>(std::move(tex_pixel_tup));

	VertShader text_vert_shdr(std::string{text_vert.begin(), text_vert.end()});
	FragShader text_frag_shdr(std::string{text_frag.begin(), text_frag.end()});
	Program const text_shdr(text_vert_shdr, text_frag_shdr);

	GlobalDrawingUniforms globals(window.width, window.height, "proj", "world");

	Texture blank{blank_tex_data, 1, 1, GL_RGB, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST, false};

	Texture font{
		char_map_data.pixel_data,
		char_map_data.md.image_width,
		char_map_data.md.image_height,
		GL_RED,
		GL_RED,
		GL_UNSIGNED_BYTE,
		GL_REPEAT,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_NEAREST,
		true
	};

	TextureUniform font_map{
		font,
		"font_map",
		GL_TEXTURE0
	};

	BufferTextureUniform char_to_uv_locations(
		char_map_data.char_to_uv_locations,
		GL_RGBA32F,
		"char_to_uv_locations",
		GL_TEXTURE1
	);

	UniformGroup always = {
		std::cref(globals),
		std::cref(font_map),
		std::cref(char_to_uv_locations),
	};


	glfwSetWindowSizeCallback(window.handle, window_change_cb);

	g_tab_width = char_map_data.md.space_width * conf.tab_size;
	g_line_height = char_map_data.md.line_height;
	glfwSetScrollCallback(window.handle, mouse_scroll_cb);
	glfwSetKeyCallback(window.handle, key_cb);
	glfwSetCursorPosCallback(window.handle, cursor_pos_cb);
	glfwSetMouseButtonCallback(window.handle, mouse_button_cb);

	text_shdr.activate();
	while (!glfwWindowShouldClose(window.handle)) {
		glfwPollEvents();

		if (window_size_changed) {
			window_size_changed = false;
			globals.regen_proj(changed_width, changed_height);
		}

		if (mouse_scrolled) {
			mouse_scrolled = false;
			globals.world = glm::translate(globals.world, {x_off, y_off, 0.f});
		}

		if (screen_dragged) {
			globals.world = glm::translate(globals.world, {2*drag_delta_x, 2*drag_delta_y, 0.f});
			drag_delta_x = 0;
			drag_delta_y = 0;
			screen_dragged = false;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		always.activate(text_shdr.program);
		draw_text(file_contents, char_map_data.md.space_width, conf.tab_size, char_map_data.md.line_height, char_map_data.char_to_metrics, text_shdr, tex_pixel);

		glfwSwapBuffers(window.handle);
	}

	return unit;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	auto out = run(argc, argv);
	if (std::holds_alternative<Err>(out)) {
		std::cerr << "Error: " << std::get<Err>(out) << std::endl;
	}
	return 0;
}
