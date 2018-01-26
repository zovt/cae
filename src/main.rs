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
uniform float t;

void main() {
	gl_Position = vec4(position.x + t, position.y, 0.0, 1.0);
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
	let window = glutin::WindowBuilder::new()
		.with_title("yuga")
		.with_dimensions(800, 600);
	let context = glutin::ContextBuilder::new();
	let display = glium::Display::new(window, context, &events_loop).unwrap();

	let program = glium::Program::from_source(&display, VERT_SRC, FRAG_SRC, None).unwrap();

	let mut exit = false;
	let mut d = 0f32;
	let mut t = 0f32;
	while !exit {
		events_loop.poll_events(|ev| match ev {
			glutin::Event::WindowEvent { event, .. } => match event {
				glutin::WindowEvent::Closed => exit = true,
				_ => (),
			},
			_ => (),
		});

		d += 0.01;
		t = d.sin();

		let vertex1 = Vertex {
			position: [-0.5, -0.5],
		};
		let vertex2 = Vertex {
			position: [0.0, 0.5],
		};
		let vertex3 = Vertex {
			position: [0.5, -0.25],
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
				&uniform! { t: t },
				&Default::default(),
			)
			.unwrap();
		target.finish().unwrap();
	}
}
