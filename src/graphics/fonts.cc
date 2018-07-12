#include "fonts.hh"

#include <iostream>
#include <ios>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace graphics::fonts;

CharMapData graphics::fonts::get_char_map_data(std::string const& path, int px_sz) {
	CharMapData result{};

	FT_Library lib = nullptr;
	FT_Init_FreeType(&lib);

	FT_Face face = nullptr;
	FT_New_Face(lib, path.c_str(), 0, &face);
	FT_Set_Char_Size(face, px_sz * 64, px_sz * 64, 72, 72);

	result.md.line_height = (float)face->height / 64.f;

	// get all chars in face
	FT_UInt glyph_idx;
	auto chr = FT_Get_First_Char(face, &glyph_idx);
	while (glyph_idx) {
		auto chr_c = chr;
		chr = FT_Get_Next_Char(face, chr, &glyph_idx);
		if (chr == 0) {
			chr = chr_c;
		}
	}
	result.char_to_uv_locations.resize((size_t)chr);
	result.char_to_metrics.resize((size_t)chr);

	std::vector<std::vector<unsigned char>> glyph_bitmaps((size_t)chr);
	size_t max_width = 0;
	size_t max_height = 0;
	chr = FT_Get_First_Char(face, &glyph_idx);
	while (glyph_idx) {
		auto chr_c = chr;
		FT_Load_Glyph(face, glyph_idx, 0);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		auto bitmap = face->glyph->bitmap;
		glyph_bitmaps[chr].reserve(bitmap.width * bitmap.rows);
		glyph_bitmaps[chr].assign(bitmap.buffer, bitmap.buffer + (bitmap.width * bitmap.rows));
		result.char_to_metrics[chr].width = bitmap.width;
		result.char_to_metrics[chr].height = bitmap.rows;
		auto metrics = face->glyph->metrics;
		result.char_to_metrics[chr].offset_x = (float)metrics.horiBearingX / 64.f;
		result.char_to_metrics[chr].offset_y = (float)metrics.horiBearingY / 64.f;
		result.char_to_metrics[chr].advance = (float)metrics.horiAdvance / 64.f;

		max_width = std::max(max_width, (size_t)bitmap.width);
		max_height = std::max(max_height, (size_t)bitmap.rows);

		chr = FT_Get_Next_Char(face, chr, &glyph_idx);
		if (chr == 0) {
			chr = chr_c;
		}
	}
	auto n_glyphs = face->num_glyphs;

	uint32_t tex_px = n_glyphs * max_height * max_width;
	uint32_t tex_size_px = 2;
	while (
		(tex_size_px * tex_size_px) < tex_px
		&& (((tex_size_px / max_width) * (tex_size_px / max_height)) < n_glyphs)
	) {
		tex_size_px = tex_size_px << 1;
	}

	result.md.image_width = tex_size_px;
	result.md.image_height = tex_size_px;

	result.pixel_data.resize((size_t)(result.md.image_width * result.md.image_height));
	auto images_per_line = tex_size_px / max_width;

	chr = FT_Get_First_Char(face, &glyph_idx);
	size_t current_image_col = 0;
	size_t current_image_row = 0;
	while (glyph_idx) {
		auto bitmap = glyph_bitmaps[chr];
		auto metrics = result.char_to_metrics[chr];
		float u_min = ((float)current_image_col * (float)max_width);
		float v_min = ((float)current_image_row * (float)max_height);
		result.char_to_uv_locations[chr] = {
			u_min / tex_size_px,
			v_min / tex_size_px,
			(u_min + (float)metrics.width) / (float)tex_size_px,
			(v_min + (float)metrics.height) / (float)tex_size_px
		};
		auto cursor = result.pixel_data.data()
			+ (current_image_col * max_width)
			+ (current_image_row * tex_size_px * max_height);
		for (int row = 0; row < metrics.height; row++) {
			std::memcpy(cursor, bitmap.data() + (row * metrics.width), metrics.width);
			cursor = cursor + tex_size_px;
		}

		current_image_col += 1;
		if (current_image_col >= images_per_line) {
			current_image_col = 0;
			current_image_row += 1;
		}
		chr = FT_Get_Next_Char(face, chr, &glyph_idx);
	}

	auto space_idx = FT_Get_Char_Index(face, 'n');
	FT_Load_Glyph(face, space_idx, 0);
	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	result.md.space_width = face->glyph->bitmap.width;

	return result;
}

