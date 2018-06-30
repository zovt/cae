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


#include <stb_image.h>
#include <stb_image_write.h>

#include "graphics/opengl/index.hh"
#include "graphics/opengl/drawing.hh"
#include "graphics/opengl/shaders.hh"
#include "graphics/opengl/primitives.hh"
#include "graphics/opengl/uniforms.hh"
#include "graphics/opengl/buffer_draw_info.hh"
#include "graphics/fonts.hh"
#include "graphics/window.hh"
#include "unit.hh"
#include "config.hh"
#include "color.hh"
#include "fonts.hh"
#include "defer.hh"
#include "macros.hh"
#include "buffer.hh"
#include "input_handler.hh"
#include "resources/shaders/opengl/text.vert.hh"
#include "resources/shaders/opengl/text.frag.hh"
#include "resources/shaders/opengl/point.vert.hh"
#include "resources/shaders/opengl/point.frag.hh"

using namespace config;
using namespace color;
using namespace fonts;
using namespace err;
using namespace defer;
using namespace window;
using namespace buffer;
using namespace input_handler;
using namespace graphics::primitives;
using namespace graphics::fonts;
using namespace graphics::opengl::drawing;
using namespace graphics::opengl::shaders;
using namespace graphics::opengl::primitives;
using namespace graphics::opengl::uniforms;
using namespace graphics::opengl::textures;
using namespace graphics::opengl::buffer_draw_info;

static const Config conf(
	{"Iosevka Term Medium"},
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

Result<Unit> run(int argc, char* argv[]) {
	using namespace std::literals;
	try(auto font_path, get_closest_font_match(conf.fonts));

	if (argc < 2) {
		return "Missing file argument";
	}
	auto buffer = slurp_to_buffer(std::filesystem::path(argv[1]));

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

	VertShader point_vert_shdr(std::string{point_vert.begin(), point_vert.end()});
	FragShader point_frag_shdr(std::string{point_frag.begin(), point_frag.end()});
	Program const point_shdr(point_vert_shdr, point_frag_shdr);

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

	UniformGroup<
	 GlobalDrawingUniforms&,
	 TextureUniform&,
	 BufferTextureUniform&
	> always = {
		globals,
		font_map,
		char_to_uv_locations,
	};

	BufferDrawInfo draw_info{
		tex_pixel,
		text_shdr,
		point_shdr,
		always,
		window,
		char_map_data.char_to_metrics,
		char_map_data.md.space_width,
		conf.tab_size,
		char_map_data.md.line_height,
		true,
	};

	InputHandler::glfw_register_callbacks(window.handle);
	InputHandler handler{
		buffer,
		draw_info,
		{},
		{},
		{},
		{},
		{},
		{},
	};
	handler.make_active();

	while (!glfwWindowShouldClose(window.handle)) {
		glfwWaitEvents();
		if (draw_info.needs_redraw) {
			draw_info.draw(buffer);
			draw_info.needs_redraw = false;
		}
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
