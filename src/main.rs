#[macro_use]
extern crate cgmath;
use cgmath::One;
#[macro_use]
extern crate glium;

use std::fs::File;
use std::io::prelude::*;

#[derive(Copy, Clone)]
struct Vertex {
	vert_coords: [f32; 2],
}

implement_vertex!(Vertex, vert_coords);

const VERT_SRC: &'static str = r#"
#version 330 core

in vec2 vert_coords;

uniform mat4 transform;
uniform mat4 world;
uniform mat4 proj;
uniform vec3 color;

out vec3 f_color;

void main() {
	gl_Position = proj * world * transform * vec4(vert_coords, 0.0, 1.0);
	f_color = color;
}
"#;

const FRAG_SRC: &'static str = r#"
#version 330 core

in vec3 f_color;
out vec4 color;

void main() {
	color = vec4(f_color, 0.0);
}
"#;

fn main() {
	use glium::Surface;
	use glium::glutin;

	let (win_w, win_h) = (800f32, 600f32);

	let mut events_loop = glutin::EventsLoop::new();
	let window = glutin::WindowBuilder::new()
		.with_title("yuga")
		.with_dimensions(win_w as u32, win_h as u32);
	let context = glutin::ContextBuilder::new()
		.with_gl(glutin::GlRequest::Specific(glutin::Api::OpenGl, (3, 3)))
		.with_gl_profile(glutin::GlProfile::Core);
	let display = glium::Display::new(window, context, &events_loop).unwrap();

	let proj: cgmath::Matrix4<f32> = cgmath::ortho(0f32, win_w, win_h, 0f32, 10f32, -10f32);
	let world: cgmath::Matrix4<f32> = cgmath::Matrix4::one();

	let vertex1 = Vertex {
		vert_coords: [1.0, 1.0],
	};
	let vertex2 = Vertex {
		vert_coords: [1.0, 0.0],
	};
	let vertex3 = Vertex {
		vert_coords: [0.0, 0.0],
	};
	let vertex4 = Vertex {
		vert_coords: [0.0, 1.0],
	};
	let px = vec![vertex1, vertex2, vertex3, vertex4];
	let px_indices = glium::index::IndexBuffer::new(
		&display,
		glium::index::PrimitiveType::TrianglesList,
		&[0, 1, 3, 1, 2, 3u8],
	).unwrap();
	let vertex_buffer = glium::VertexBuffer::new(&display, &px).unwrap();

	let program = glium::Program::from_source(&display, VERT_SRC, FRAG_SRC, None).unwrap();

	let proj_ref: [[f32; 4]; 4] = proj.into();
	let world_ref: [[f32; 4]; 4] = world.into();

	let mut f = File::open("src/main.rs").unwrap();
	let mut text = String::new();
	f.read_to_string(&mut text);
	let mut exit = false;
	while !exit {
		events_loop.poll_events(|ev| match ev {
			glutin::Event::WindowEvent { event, .. } => match event {
				glutin::WindowEvent::Closed => exit = true,
				_ => (),
			},
			_ => (),
		});

		let mut target = display.draw();
		target.clear_color(1.0, 1.0, 1.0, 1.0);

		let mut col = 0;
		let mut row = 0;
		for c in text.chars() {
			if c == '\n' {
				row = row + 1;
				col = 0;
			} else if c.is_whitespace() {
				col = col + 1;
			} else {
				let transform: cgmath::Matrix4<f32> = cgmath::Matrix4::from_translation(
					cgmath::Vector3::new(col as f32 * 10.0f32, row as f32 * 10f32, 0f32),
				) * cgmath::Matrix4::from_scale(10f32);
				let transform_ref: [[f32; 4]; 4] = transform.into();

				let uniforms = uniform! {
					proj: proj_ref,
					world: world_ref,
					transform: transform_ref,
					color: [1.0, 0.0, 0.0f32]
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

				col = col + 1;
			}
		}
		target.finish().unwrap();
	}
}
