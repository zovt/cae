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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STBI_ONLY_BMP
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
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
	{"Iosevka Term"},
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

// Dirty hack to deal with c-style callback
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

Result<Unit> run() {
	using namespace std::literals;

	try(auto font_path, get_closest_font_match(conf.fonts));
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
	if (!(
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
		char_map_data.pixel_data.reserve(tex_bmp_width * tex_bmp_height);
		char_map_data.pixel_data.assign((PixelData*)data, (PixelData*)(data + pixel_data.capacity()));
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

	std::raise(SIGINT);

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

	GlobalDrawingUniforms globals(window.width, window.height);

	Texture blank{blank_tex_data, 1, 1, GL_RGB, GL_FLOAT, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR};

	int test_png_width;
	int test_png_height;
	int test_png_n;
	auto test_png_data = stbi_load_from_memory(
		test_png.data(),
		test_png.size(),
		&test_png_width,
		&test_png_height,
		&test_png_n,
		3
	);
	std::vector<unsigned char> test_png_owned_data((test_png_width * test_png_height) * 3);
	test_png_owned_data.assign(test_png_data, test_png_data + test_png_owned_data.capacity());
	stbi_image_free(test_png_data);
	Texture test{
		test_png_owned_data,
		test_png_width,
		test_png_height,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		GL_CLAMP_TO_EDGE,
		GL_LINEAR_MIPMAP_LINEAR,
		GL_LINEAR
	};

	glfwSetWindowSizeCallback(window.handle, window_change_cb);

	auto transform = glm::scale(glm::translate(glm::mat4(1.f), {400.f, 300.f, 0.f}), {100.f, 100.f, 1.f});

	while (!glfwWindowShouldClose(window.handle)) {
		if (window_size_changed) {
			window_size_changed = false;
			globals.regen_proj(changed_width, changed_height);
		}

		glfwPollEvents();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		tex_pixel.draw(text_shdr, UniformGroup{std::cref(globals), TransformUniform{transform}, std::cref(test)});

		glfwSwapBuffers(window.handle);
	}

	return unit;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	auto out = run();
	if (std::holds_alternative<Err>(out)) {
		std::cerr << "Error: " << std::get<Err>(out) << std::endl;
	}
	return 0;
}
