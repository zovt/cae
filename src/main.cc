#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <fstream>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <functional>

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
#define DEBUG_NAMESPACE "main"
#include "debug.hh"
#include "buffer.hh"
#include "input_handling.hh"
#include "resources/shaders/opengl/text.vert.hh"
#include "resources/shaders/opengl/text.frag.hh"
#include "resources/shaders/opengl/point.vert.hh"
#include "resources/shaders/opengl/point.frag.hh"
#include "plat.hh"

using namespace config;
using namespace color;
using namespace fonts;
using namespace err;
using namespace defer;
using namespace window;
using namespace buffer;
using namespace input_handling;
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
	RGBColor(0, 0, 0),
	RGBColor(255, 255, 234),
	2,
	20
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

Buffer get_initial_buffer(int argc, char* argv[]) {
	using namespace std::literals;

	std::string path{"/dev/null"};
	if (argc == 2) {
		std::string_view path_str{argv[1]};
		if (path_str == "-"sv) {
			Buffer::Contents stdin_contents{};
			std::string line{};
			while (std::getline(std::cin, line)) {
				stdin_contents.insert(stdin_contents.end(), line.begin(), line.end());
				stdin_contents.push_back('\n');
			}
	
			Buffer buf{};
			buf.path = path;
			buf.contents = stdin_contents;
			return buf;
		} else {
			path = std::string{argv[1]};
			if (plat_file_exists(path)) {
				return slurp_to_buffer(path);
			}
			Buffer buf{};
			buf.path = path;
		}
	}

	Buffer buf{};
	buf.path = path;
	return buf;
}

Result<Unit> run(int argc, char* argv[]) {
	using namespace std::literals;
	try(auto font_path, get_closest_font_match(conf.fonts));
	auto buffer = get_initial_buffer(argc, argv);

	auto path_hash = std::hash<std::string>{}(font_path);
	auto size_hash = std::hash<int>{}(conf.font_size);
	auto hash_name = std::to_string(path_hash + (size_hash * 13));
	auto home_dir = std::getenv("HOME");
	if (home_dir == nullptr) {
		return "Home dir does not exist"sv;
	}
	auto data_dir = std::string(home_dir) + "/.local/share/cae";
	if (!plat_file_exists(data_dir)) {
		plat_mkdir(data_dir);
	}
	auto font_data_folder = data_dir + "/" + hash_name;

	auto tex_path = font_data_folder + "/tex.bmp";
	auto uv_path = font_data_folder + "/uv.dat";
	auto metrics_path = font_data_folder + "/metrics.dat";
	auto md_path = font_data_folder + "/meta.dat";

	CharMapData char_map_data;
	if (!(
		plat_file_exists(font_data_folder)
		&& plat_file_exists(tex_path)
		&& plat_file_exists(uv_path)
		&& plat_file_exists(metrics_path)
		&& plat_file_exists(md_path)
	)) {
		plat_mkdir(font_data_folder);

		char_map_data = get_char_map_data(font_path, conf.font_size);

		stbi_write_bmp(tex_path.c_str(), char_map_data.md.image_width, char_map_data.md.image_height, 1, char_map_data.pixel_data.data());

		std::ofstream uv(uv_path, std::ios::binary);
		uv.write((char const*)char_map_data.char_to_uv_locations.data(), char_map_data.char_to_uv_locations.size() * sizeof(UVLocation));

		std::ofstream metrics(metrics_path, std::ios::binary);
		metrics.write((char const*)char_map_data.char_to_metrics.data(), char_map_data.char_to_metrics.size() * sizeof(Metrics));

		std::ofstream md(md_path, std::ios::binary);
		md.write((char const*)(&char_map_data.md), sizeof(CharMapData::Metadata));
	} else {
		int tex_bmp_width;
		int tex_bmp_height;
		int tex_bmp_n;
		auto data = stbi_load(tex_path.c_str(), &tex_bmp_width, &tex_bmp_height, &tex_bmp_n, 1);
		char_map_data.pixel_data.assign((PixelData*)data, (PixelData*)(data + (tex_bmp_width * tex_bmp_height)));
		stbi_image_free(data);

		auto metrics_size = plat_get_file_size(metrics_path);
		std::ifstream metrics(metrics_path, std::ios::binary);
		char_map_data.char_to_metrics.resize(metrics_size / sizeof(Metrics));
		metrics.read((char*)char_map_data.char_to_metrics.data(), metrics_size);

		auto uv_size = plat_get_file_size(uv_path);
		std::ifstream uv(uv_path, std::ios::binary);
		char_map_data.char_to_uv_locations.resize(uv_size / sizeof(UVLocation));
		uv.read((char*)char_map_data.char_to_uv_locations.data(), uv_size);

		std::ifstream md(md_path, std::ios::binary);
		md.read((char*)(&char_map_data.md), sizeof(CharMapData::Metadata));
	}

	std::string window_name{"cae - " + buffer.path};
	Window window(800, 600, window_name.c_str());

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

	VecUniform<GLuint, 3> text_fg = {{conf.fg.red, conf.fg.green, conf.fg.blue}, "text_fg"};
	VecUniform<GLuint, 3> text_bg = {{conf.bg.red, conf.bg.green, conf.bg.blue}, "text_bg"};
	// FIXME: Make this a config option
	VecUniform<GLuint, 3> selection_color = {{202, 255, 255}, "text_bg"};

	UniformGroup<
		GlobalDrawingUniforms&,
		TextureUniform&,
		BufferTextureUniform&,
		VecUniform<GLuint, 3>&,
		VecUniform<GLuint, 3>&
	> always = {
		globals,
		font_map,
		char_to_uv_locations,
		text_fg,
		text_bg
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
		conf.bg,
		selection_color
	};

	PersistentState state{};

	glfw_register_callbacks(window.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!glfwWindowShouldClose(window.handle)) {
		glfwWaitEvents();
		while (!event_queue.empty()) {
			dbg_println("Event queue");
			auto needs_redraw = handle_event(
				event_queue_pop(),
				buffer,
				draw_info,
				state
			);
			dbg_printval(needs_redraw);
			draw_info.needs_redraw = draw_info.needs_redraw | needs_redraw;
		}
	
		if (draw_info.needs_redraw) {
			dbg_println("Needs redraw");
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
