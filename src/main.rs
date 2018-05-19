extern crate cgmath;
#[macro_use]
extern crate clap;
extern crate freetype;
#[macro_use]
extern crate glium;
extern crate toml;
#[macro_use]
extern crate serde_derive;
extern crate serde;
#[macro_use]
extern crate cfg_if;
#[macro_use]
extern crate failure;

mod hb_raw;

use cgmath::One;
use freetype::freetype::*;
use hb_raw::*;
use failure::{Fail, Error, err_msg};
use serde::de;
use serde::ser;

use std::collections::HashMap;
use std::fs::{read_to_string, create_dir, File};
use std::io::prelude::*;
use std::ffi;
use std::fmt;
use std::path::PathBuf;

mod plat_s {
	use failure::{Fail, Error};
	use std::path::PathBuf;
	use std::convert::From;
	use std::fmt::Display;

	pub trait FontMatcher {
		fn lookup_font_name(&mut self, font_name: &str) -> Result<Option<PathBuf>, Error>;
	}
	
	cfg_if! {
		if #[cfg(unix)] {
			extern crate fontconfig as fc_base;
			extern crate xdg;

			use std::ffi;
			use self::fc_base::fontconfig;

			#[derive(Fail, Debug)]
			#[fail(display = "Error loading XDG directories: {:?}", error)]
			pub struct Wrapped {
				error: xdg::BaseDirectoriesError,
			}

			impl From<xdg::BaseDirectoriesError> for Wrapped {
				fn from(error: xdg::BaseDirectoriesError) -> Self {
					Wrapped { error }
				}
			}

			pub fn get_config_dir() -> Result<PathBuf, Error> {
				let xdg_dirs = xdg::BaseDirectories::with_prefix("cae")?;				

				Ok(xdg_dirs.get_config_home())
			}

			struct UnixFontMatcher {
				config: *mut fontconfig::FcConfig,
			}

			impl Drop for UnixFontMatcher {
				fn drop(&mut self) {
					unsafe {
						fontconfig::FcConfigDestroy(self.config);
					}
				}
			}

			impl FontMatcher for UnixFontMatcher {				
				fn lookup_font_name(&mut self, font_name: &str) -> Result<Option<PathBuf>, Error> {
					use std::ffi as ffi;
					use std;

					let mut fc_result = fontconfig::FcResultNoMatch;
					let mut pattern = unsafe {
						let mut pattern = fontconfig::FcNameParse(
							(ffi::CString::new(font_name)?).as_ptr() as *const u8
						);
						fontconfig::FcConfigSubstitute(self.config, pattern, fontconfig::FcMatchPattern);
						fontconfig::FcDefaultSubstitute(pattern);
						pattern
					};

					let res = unsafe { fontconfig::FcFontMatch(self.config, pattern, &mut fc_result) };
					let ret = if !res.is_null() {
						let mut file: *mut fontconfig::FcChar8 = std::ptr::null_mut();
						
						let ret = if unsafe {
							// FIXME: 2nd argument is manually pulled from source
							fontconfig::FcPatternGetString(
								res,
								(ffi::CString::new("file")?).as_ptr(),
								0,
								&mut file as *mut *mut u8
							)
						} == fontconfig::FcResultMatch {
							Ok(Some(PathBuf::from(unsafe {
								ffi::CString::from_raw(file as *mut i8).to_str()?
							})))
						} else {
							Ok(None)
						};

						unsafe { fontconfig::FcPatternDestroy(res) };

						ret
					} else {
						Ok(None)
					};

					unsafe { fontconfig::FcPatternDestroy(pattern) };
					
					ret
				}
			}
			
			pub fn get_font_matcher() -> impl FontMatcher {
				let config = unsafe { fontconfig::FcInitLoadConfigAndFonts() };
				
				UnixFontMatcher {
					config
				}
			}
		}
	}
}

use plat_s::FontMatcher;

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
struct HexColor {
	value: u32,
}

impl HexColor {
	pub fn red(self) -> u8 {
		((self.value >> 16) & 0xFF) as u8
	}
	pub fn green(self) -> u8 {
		((self.value >> 8) & 0xFF) as u8
	}
	pub fn blue(self) -> u8 {
		((self.value) & 0xFF) as u8
	}
}

impl From<u32> for HexColor {
	fn from(value: u32) -> Self {
		HexColor { value }
	}
}

struct HexColorVisitor;

impl<'de> de::Visitor<'de> for HexColorVisitor {
	type Value = HexColor;

	fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
		formatter.write_str("a color defined in hex like 0x000000")
	}

	fn visit_str<E>(self, value: &str) -> Result<HexColor, E>
		where E: de::Error {
		Ok(HexColor {
			value: u32::from_str_radix(
				if value.starts_with("0x") { &value[2..] }
				else if value.starts_with("#") { &value[1..] }
				else { return Err(de::Error::custom("Invalid prefix for hex string")) },
				16
			).map_err(de::Error::custom)?,
		})
	}
}

impl<'de> de::Deserialize<'de> for HexColor {
	fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
	where  D: de::Deserializer<'de> {
		deserializer.deserialize_str(HexColorVisitor)
	}
}

impl ser::Serialize for HexColor {
	fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
		where S: ser::Serializer {
    serializer.serialize_str(&format!("0x{:06X}", self.value))
  }
}

#[derive(Debug, Deserialize, Serialize, Clone)]
struct Config {
	pub fonts: Vec<String>, // can provide fallback fonts
	pub fg: HexColor,
	pub bg: HexColor,
	pub tab_size: u8,
	pub font_size: u8,
}

impl Default for Config {
	fn default() -> Self {
		Config {
			fonts: vec!("mono".into(), "Courier".into()),
			fg: 0xFFFFFF.into(),
			bg: 0x000000.into(),
			tab_size: 2,
			font_size: 16,
		}
	}
}

#[derive(Copy, Clone)]
struct Vertex {
	pos: [f32; 2],
	uv: [f32; 2],
}

implement_vertex!(Vertex, pos, uv);

const VERT_SRC: &'static str = include_str!("shaders/text.vert");
const FRAG_SRC: &'static str = include_str!("shaders/text.frag");

struct GlyphImg<'a> {
	pub data: &'a [u8],
	pub width: u32,
	pub height: u32,
}

struct GlyphDrawInfo {
	pub tex: glium::Texture2d,
	pub bm_top: i32,
	pub bm_left: i32,
}

impl<'a> glium::texture::Texture2dDataSource<'a> for GlyphImg<'a> {
	type Data = u8;

	fn into_raw(self) -> glium::texture::RawImage2d<'a, Self::Data> {
		glium::texture::RawImage2d {
			data: std::borrow::Cow::Borrowed(self.data),
			width: self.width,
			height: self.height,
			format: glium::texture::ClientFormat::U8,
		}
	}
}

// FIXME: bitmaps are broken?
fn main() -> Result<(), Error> {
	let matches = clap_app!(cae =>
		(version: "cae 0.0.1")
		(author: "zovt <zovt@posteo.de>")
		(@arg file: +takes_value)
	).get_matches();

	let path = matches.value_of("file").expect("Missing file");

	// handle config
	let mut config_path = plat_s::get_config_dir()?;
	if !config_path.exists() {
		create_dir(&config_path)?;
	}
	config_path.push("cae.toml");
	if !config_path.exists() {
		let mut file = File::create(&config_path)?;
		file.write_all((toml::to_string_pretty(&Config::default())?).as_bytes())?;
	}
	let config: Config = toml::from_str(&read_to_string(config_path)?)?;
	
	let mut font_matcher = plat_s::get_font_matcher();
	let font_path: PathBuf = config.fonts.iter()
		.map(|name| font_matcher.lookup_font_name(name))
		.map(|res| { match res {
			Err(e) => { println!("{}", e); Err(e) },
			o => o,
		} })
		.filter_map(Result::ok)
		.filter_map(|o| o)
		.nth(0)
		.ok_or_else(|| err_msg("Failed to match any font names"))?;

	use glium::Surface;
	use glium::glutin;

	let (mut win_w, mut win_h) = (800f32, 600f32);

	// Font stuff
	let px_sz = config.font_size as u32;

	let mut ft_lib: FT_Library = std::ptr::null_mut();
	unsafe {
		FT_Init_FreeType(&mut ft_lib);
	};
	let mut ft_face: FT_Face = std::ptr::null_mut();
	unsafe {
		FT_New_Face(
			ft_lib,
			(ffi::CString::new(
				font_path.to_str()
					.ok_or(err_msg("Invalid characters in filename"))?
			)?).as_ptr(),
			0,
			&mut ft_face as *mut FT_Face,
		);
		FT_Set_Pixel_Sizes(ft_face, 0, px_sz);
	};
	let hb_font: *mut hb_font_t = unsafe { hb_ft_font_create(ft_face, None) };
	let hb_buf = unsafe { hb_buffer_create() };
	let eng = unsafe { hb_language_from_string(ffi::CString::new("en").unwrap().as_ptr(), -1) };

	// Graphics stuff
	let mut events_loop = glutin::EventsLoop::new();
	let window = glutin::WindowBuilder::new()
		.with_title("cae")
		.with_dimensions(win_w as u32, win_h as u32);
	let context = glutin::ContextBuilder::new()
		.with_gl(glutin::GlRequest::Specific(glutin::Api::OpenGl, (3, 3)))
		.with_gl_profile(glutin::GlProfile::Core);
	let display = glium::Display::new(window, context, &events_loop).unwrap();

	let proj: cgmath::Matrix4<f32> = cgmath::ortho(0f32, win_w, win_h, 0f32, 10f32, -10f32);
	let (mut x, mut y) = (0.0, 0.0);
	let mut world: cgmath::Matrix4<f32> = cgmath::Matrix4::one();

	let vertex1 = Vertex {
		pos: [1.0, 1.0],
		uv: [1.0, 1.0],
	};
	let vertex2 = Vertex {
		pos: [1.0, 0.0],
		uv: [1.0, 0.0],
	};
	let vertex3 = Vertex {
		pos: [0.0, 0.0],
		uv: [0.0, 0.0],
	};
	let vertex4 = Vertex {
		pos: [0.0, 1.0],
		uv: [0.0, 1.0],
	};
	let px = vec![vertex1, vertex2, vertex3, vertex4];
	let px_indices = glium::index::IndexBuffer::new(
		&display,
		glium::index::PrimitiveType::TrianglesList,
		&[0, 1, 3, 1, 2, 3u8],
	).unwrap();
	let vertex_buffer = glium::VertexBuffer::new(&display, &px).unwrap();

	let program = glium::Program::from_source(&display, VERT_SRC, FRAG_SRC, None).unwrap();

	let mut proj_ref: [[f32; 4]; 4] = proj.into();
	let mut world_ref: [[f32; 4]; 4] = world.into();

	let mut g_d_infos: HashMap<u32, GlyphDrawInfo> = HashMap::new();

	let mut f = File::open(path).unwrap();
	let mut text = String::new();
	f.read_to_string(&mut text).unwrap();

	// spacing constants
	let (space_width, tab_width) = unsafe {
		let g_idx = FT_Get_Char_Index(ft_face, ' ' as u64);
		FT_Load_Glyph(ft_face, g_idx, 0);
		let space_width = (*(*ft_face).glyph).metrics.horiAdvance;
		(space_width as f32 / 64.0, config.tab_size as f32 * (space_width as f32 / 64.0))
	};

	let mut exit = false;
	// TODO: Only render when necessary to avoid unneeded CPU usage. This should be
	// pretty stupid at first
	while !exit {
		events_loop.poll_events(|ev| match ev {
			glutin::Event::WindowEvent { event, .. } => match event {
				glutin::WindowEvent::Closed => exit = true,
				glutin::WindowEvent::Resized(w, h) => {
					win_w = w as f32;
					win_h = h as f32;
					proj_ref = cgmath::ortho(0f32, win_w, win_h, 0f32, 10f32, -10f32).into()
				}
				glutin::WindowEvent::MouseWheel { delta, .. } => match delta {
					// TODO: Put max bounds on scrolling
					glutin::MouseScrollDelta::LineDelta(h, v) => {
						x = (x + h * 2.0 * px_sz as f32).max(0.0);
						y = (y + -v * 2.0 * px_sz as f32).max(0.0);
						world =
							cgmath::Matrix4::from_translation(cgmath::Vector3::new(-x, -y, 0.0));
						world_ref = world.into();
					}
					glutin::MouseScrollDelta::PixelDelta(x_d, y_d) => {
						x = (x + x_d).max(0.0);
						y = (y + -y_d).max(0.0);
						world = world
							* cgmath::Matrix4::from_translation(cgmath::Vector3::new(-x, -y, 0.0));
						world_ref = world.into();
					}
				},
				_ => (),
			},
			_ => (),
		});

		let mut target = display.draw();
		target.clear_color(
			(config.bg.red() as f32) / 255.0,
			(config.bg.green() as f32) / 255.0,
			(config.bg.blue() as f32) / 255.0,
			1.0
		);

		let mut pen = (0.0, px_sz as f32);
		let mut idx = 0;
		// set pen y and idx to start at currently visible line
		while pen.1 <= y {
			match text[idx..].find('\n') {
				Some(nl_idx) => idx += nl_idx + 1,
				None => {
					break;
				},
			};
			
			pen.1 += px_sz as f32;
		}
			
		while idx < text.len() {
			if pen.1 > y + win_h {
				break;
			}
			let mut c = text.chars().nth(idx).unwrap();
			if c.is_whitespace() {
				match c {
					'\t' => pen.0 += tab_width,
					' ' => pen.0 += space_width,
					'\n' => {
						pen.0 = 0.0;
						pen.1 += px_sz as f32;
					}
					_ => (),
				};
				idx += 1;
				continue;
			}

			let start = idx;
			while {
				c = text.chars().nth(idx).unwrap();
				idx += 1;
				!c.is_whitespace() && idx < text.len()
			} {};
			let end = idx;

			// render previous "word"
			let word = &text[start..end];
			let (glyph_count, glyphs, glyphs_pos) = unsafe {
				hb_buffer_set_direction(hb_buf, HB_DIRECTION_LTR);
				hb_buffer_set_script(hb_buf, HB_SCRIPT_LATIN);
				hb_buffer_set_language(hb_buf, eng);
				hb_buffer_add_utf8(
					hb_buf,
					ffi::CString::new(word).unwrap().as_ptr(),
					word.len() as i32,
					0,
					word.len() as i32,
				);
				hb_shape(hb_font, hb_buf, std::ptr::null(), 0);
				let mut glyph_count = 0;
				let glyphs = hb_buffer_get_glyph_infos(hb_buf, &mut glyph_count);
				let glyphs_pos = hb_buffer_get_glyph_positions(hb_buf, &mut glyph_count);
				hb_buffer_clear_contents(hb_buf);

				(glyph_count, glyphs, glyphs_pos)
			};

			for i in 0..glyph_count {
				let glyph_pos = unsafe { *glyphs_pos.offset(i as isize) };
				let glyph = unsafe { *glyphs.offset(i as isize) };

				if !g_d_infos.contains_key(&glyph.codepoint) {
					unsafe {
						FT_Load_Glyph(ft_face, glyph.codepoint, 0);
						FT_Render_Glyph((*ft_face).glyph, FT_Render_Mode::FT_RENDER_MODE_NORMAL);
					};

					let ft_glyph = unsafe { (*(*ft_face).glyph) };
					let ft_bitmap = ft_glyph.bitmap;

					println!("{}", glyph.codepoint);
					for y in 0..ft_bitmap.rows {
						for x in 0..ft_bitmap.width {
							print!("{:06x} ", unsafe { *(ft_bitmap.buffer.offset(x as isize + y as isize * ft_bitmap.width as isize)) });
						}
						println!("");
					}

					let glyph_img = GlyphImg {
						data: unsafe {
							std::slice::from_raw_parts(
								ft_bitmap.buffer,
								(ft_bitmap.rows * ft_bitmap.width) as usize,
							)
						},
						width: ft_bitmap.width,
						height: ft_bitmap.rows,
					};

					g_d_infos.insert(
						glyph.codepoint,
						GlyphDrawInfo {
							tex: glium::texture::texture2d::Texture2d::new(&display, glyph_img)
								.unwrap(),
							bm_left: ft_glyph.bitmap_left,
							bm_top: ft_glyph.bitmap_top,
						},
					);
				};

				let g_d_info = &g_d_infos[&glyph.codepoint];

				let transform: cgmath::Matrix4<f32> =
					cgmath::Matrix4::from_translation(cgmath::Vector3::new(
						(pen.0 + (glyph_pos.x_offset as f32) / 64f32 + g_d_info.bm_left as f32)
							.round(),
						(pen.1 + (glyph_pos.y_offset as f32) / 64f32 - g_d_info.bm_top as f32)
							.round(),
						0f32,
					))
						* cgmath::Matrix4::from_nonuniform_scale(
							g_d_info.tex.width() as f32,
							g_d_info.tex.height() as f32,
							1.0f32,
						);
				let transform_ref: [[f32; 4]; 4] = transform.into();

				let uniforms = uniform! {
					proj: proj_ref,
					world: world_ref,
					transform: transform_ref,
					glyph: &g_d_info.tex,
					fg: [
						config.fg.red() as f32 / 255.0,
						config.fg.green() as f32 / 255.0,
						config.fg.blue() as f32 / 255.0
					],
					bg: [
						config.bg.red() as f32 / 255.0,
						config.bg.green() as f32 / 255.0,
						config.bg.blue() as f32 / 255.0
					],
				};
				target
					.draw(
						&vertex_buffer,
						&px_indices,
						&program,
						&uniforms,
						&Default::default(),
					)
					.unwrap();

				pen = (
					(pen.0 + (glyph_pos.x_advance as f32) / 64f32).round(),
					(pen.1 - (glyph_pos.y_advance as f32) / 64f32).round(),
				);
			}
		}
		target.finish().unwrap();
	}

	// Clean up
	unsafe {
		hb_buffer_destroy(hb_buf);
		FT_Done_Library(ft_lib);
	};

	Ok(())
}
