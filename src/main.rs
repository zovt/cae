#[macro_use]
extern crate glium;

#[derive(Copy, Clone)]
struct Vertex {
	position: [f32; 2],
}

implement_vertex!(Vertex, position);

const VERT_SRC: &'static str = r#"
#version 330 core
in vec2 position;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
}
"#;

const FRAG_SRC: &'static str = r#"
#version 330 core
out vec4 color;

void main() {
	color = vec4(1.0, 0.0, 0.0, 1.0);
}
"#;

fn main() {
	use glium::Surface;
	use glium::glutin;

	let mut events_loop = glutin::EventsLoop::new();
	let window = glutin::WindowBuilder::new();
	let context = glutin::ContextBuilder::new();
	let display = glium::Display::new(window, context, &events_loop).unwrap();

	let program = glium::Program::from_source(&display, VERT_SRC, FRAG_SRC, None).unwrap();

	let mut exit = false;
	let mut d = 0f32;
	while !exit {
		events_loop.poll_events(|ev| match ev {
			glutin::Event::WindowEvent { event, .. } => match event {
				glutin::WindowEvent::Closed => exit = true,
				_ => (),
			},
			_ => (),
		});
		
		d += 0.1;

		let vertex1 = Vertex {
			position: [-0.5 + d.sin(), -0.5],
		};
		let vertex2 = Vertex {
			position: [0.0 + d.cos(), 0.5],
		};
		let vertex3 = Vertex {
			position: [0.5 + d.cos(), -0.25],
		};
		let shape = vec![vertex1, vertex2, vertex3];

		let vertex_buffer = glium::VertexBuffer::new(&display, &shape).unwrap();
		let indices = glium::index::NoIndices(glium::index::PrimitiveType::TrianglesList);

		let mut target = display.draw();
		target.clear_color(1.0, 1.0, 1.0, 1.0);
		target
			.draw(
				&vertex_buffer,
				&indices,
				&program,
				&glium::uniforms::EmptyUniforms,
				&Default::default(),
			)
			.unwrap();
		target.finish().unwrap();
	}
}
