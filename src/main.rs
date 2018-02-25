#[macro_use]
extern crate cgmath;
#[macro_use]
extern crate clap;
extern crate freetype;
#[macro_use]
extern crate glium;

mod hb_raw;

use cgmath::One;
use freetype::freetype::*;
use hb_raw::*;
use std::ffi;
use std::fs::File;
use std::io::prelude::*;
use std::collections::HashMap;

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

fn main() {
	let matches = clap_app!(cae =>
		(version: "cae 0.0.1")
		(author: "zovt <zovt@posteo.de>")
		(@arg file: +takes_value)
	).get_matches();

	let path = matches.value_of("file").unwrap();

	use glium::Surface;
	use glium::glutin;

	let (win_w, win_h) = (800f32, 600f32);

	// Font stuff
	let px_sz = 18;

	let mut ft_lib: FT_Library = std::ptr::null_mut();
	unsafe {
		FT_Init_FreeType(&mut ft_lib);
	};
	let mut ft_face: FT_Face = std::ptr::null_mut();
	unsafe {
		FT_New_Face(
			ft_lib,
			ffi::CString::new("fonts/Hasklig-Regular.otf").unwrap().as_ptr(),
			0,
			&mut ft_face as *mut FT_Face,
		);
		FT_Set_Pixel_Sizes(ft_face, 0, px_sz);
	};
	let mut hb_font: *mut hb_font_t = unsafe { hb_ft_font_create(ft_face, None) };
	let mut hb_buf = unsafe { hb_buffer_create() };
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
	let window_id = display.gl_window().id();

	let mut proj: cgmath::Matrix4<f32> = cgmath::ortho(0f32, win_w, win_h, 0f32, 10f32, -10f32);
	let (mut x, mut y) = (0.0, 0.0);
	let mut world: cgmath::Matrix4<f32> = cgmath::Matrix4::one();

	let vertex1 = Vertex {
		pos: [1.0, 1.0],
		uv: [1.0, 1.0]
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
	f.read_to_string(&mut text);
	
	// spacing constants
	let (space_width, tab_width) = unsafe {
		let g_idx = FT_Get_Char_Index(ft_face, ' ' as u64);
		FT_Load_Glyph(ft_face, g_idx, 0);
		let space_width = (*(*ft_face).glyph).metrics.horiAdvance;
		(space_width as f32 / 64.0, 2.0 * (space_width as f32 / 64.0))
	};

	let mut exit = false;
	// TODO: Only render when necessary to avoid unneeded CPU usage. This should be
	// pretty stupid at first
	while !exit {
		events_loop.poll_events(|ev| match ev {
			glutin::Event::WindowEvent { event, .. } => match event {
				glutin::WindowEvent::Closed => exit = true,
				glutin::WindowEvent::Resized(w, h) => proj_ref = cgmath::ortho(0f32, w as f32, h as f32, 0f32, 10f32, -10f32).into(),
				glutin::WindowEvent::MouseWheel { delta, .. } => match delta {
					// TODO: Put max bounds on scrolling
					glutin::MouseScrollDelta::LineDelta(h, v) => {
						x = (x + h * 2.0 * px_sz as f32).max(0.0);
						y = (y + -v * 2.0 * px_sz as f32).max(0.0);
						world = cgmath::Matrix4::from_translation(cgmath::Vector3::new(-x, -y, 0.0));
						world_ref = world.into();
					},
					glutin::MouseScrollDelta::PixelDelta(x_d, y_d) => {
						x = (x + x_d).max(0.0);
						y = (y + -y_d).max(0.0);
						world = world * cgmath::Matrix4::from_translation(cgmath::Vector3::new(-x, -y, 0.0));
						world_ref = world.into();
					},
				},
				_ => (),
			},
			_ => (),
		});
	
		let mut target = display.draw();
		target.clear_color(1.0, 1.0, 1.0, 1.0);

		let mut pen = (0.0, px_sz as f32);
		let (mut start, mut end) = (0, 0);
		let mut idx = 0;
		while idx < text.len() {
			let mut c = text.chars().nth(idx).unwrap();
			if c.is_whitespace() {
				match c {
					'\t' => pen.0 += tab_width,
					' ' => pen.0 += space_width,
					'\n' => { pen.0 = 0.0; pen.1 += px_sz as f32; },
					_ => (),
				};
				idx += 1;
				continue;
			}
			
			start = idx;
			while !c.is_whitespace() && idx < text.len() {
				idx += 1;
				c = text.chars().nth(idx).unwrap();
			}
			end = idx;
			
			// render previous "word"
			let word = &text[start..end];
			let (glyph_count, glyphs, glyphs_pos) = unsafe {
				hb_buffer_set_direction(hb_buf, HB_DIRECTION_LTR);
				hb_buffer_set_script(hb_buf, HB_SCRIPT_LATIN);
				hb_buffer_set_language(
					hb_buf,
					eng,
				);
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
				
					let ft_glyph = unsafe{ (*(*ft_face).glyph) };
					let ft_bitmap = ft_glyph.bitmap;
				
					let glyph_img = GlyphImg {
						data: unsafe { std::slice::from_raw_parts(ft_bitmap.buffer, (ft_bitmap.rows * ft_bitmap.width) as usize) },
						width: ft_bitmap.width,
						height: ft_bitmap.rows,
					};
					
					g_d_infos.insert(glyph.codepoint, GlyphDrawInfo {
						tex: glium::texture::texture2d::Texture2d::new(
							&display,
							glyph_img,
						).unwrap(),
						bm_left: ft_glyph.bitmap_left,
						bm_top: ft_glyph.bitmap_top,
					});
				};

				let g_d_info = &g_d_infos[&glyph.codepoint];

				let transform: cgmath::Matrix4<f32> = cgmath::Matrix4::from_translation(
					cgmath::Vector3::new((pen.0 + (glyph_pos.x_offset as f32)/64f32 + g_d_info.bm_left as f32).round(), (pen.1 + (glyph_pos.y_offset as f32)/64f32 - g_d_info.bm_top as f32).round(), 0f32),
				) * cgmath::Matrix4::from_nonuniform_scale(g_d_info.tex.width() as f32, g_d_info.tex.height() as f32, 1.0f32);
				let transform_ref: [[f32; 4]; 4] = transform.into();
					
				let uniforms = uniform! {
					proj: proj_ref,
					world: world_ref,
					transform: transform_ref,
					glyph: &g_d_info.tex,
					color: [0.0, 0.0, 0.0f32]
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
	
				pen = ((pen.0 + (glyph_pos.x_advance as f32)/64f32).round(), (pen.1 - (glyph_pos.y_advance as f32)/64f32).round());
			}
		}
		target.finish().unwrap();
	}

	// Clean up
	unsafe {
		hb_buffer_destroy(hb_buf);
		FT_Done_Library(ft_lib);
	};
}
