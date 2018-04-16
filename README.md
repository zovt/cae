# cae
cae is an experimental, mouse-based, OpenGL-UI text editor

## Goals
### Be just a text editor
cae has no plugin system. Instead, you can send text, buffers, etc to other programs. In this way, cae integrates into the surrounding system instead of becoming another subsystem.

### Be efficient
cae is written in Rust and utilizes a custom UI toolkit written in OpenGL. The editor itself should never be the bottleneck when writing code.

We want to render every file at 60 FPS or more. See [Contributing](#contributing)

### Be simple
You should be thinking about the code you are writing when working in cae, instead of remembering what invocation you need to select the letter 'b' on line 5 in the file 'example.rs'.

### Render text well
It should not be hard to read text in cae.

cae will:
- support proportional-width fonts
- elegantly handle multiple cursors
- render ligatures
- support [elastic tabstops](http://nickgravgaard.com/elastic-tabstops/)

## Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md).

**IMPORTANT**: If a file does not render at 60FPS, please open an issue with the attached file!

## Prior Art
- acme
- sam
- ed
- vi
