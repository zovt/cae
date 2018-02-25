# Rendering
## Current status
As of 2706a393442b309a0ef6fc51dd236d3d23ea99a2, cae renders src/main.rs at about 20 FPS (this might be influenced by GNOME vsync on zovt's machine) in release mode according to glxosd.

Some things we can do to increase the render speed:
- Move glyphs into a texture atlas
	- Benefits:
		- This should prevent needless texture swapping
		- We can pass atlas coordinates
	- Considerations:
		- Do we render all glyphs in the fonts at once?
		- How to determine the pixel size of the atlas
		- Should we cache the atlas to disk for a given font?
		- Multiple fonts?

- Render only the part of the file we are currently displaying
	- Benefits:
		- This will probably give a very significant speedup
	- Considerations:
		- Determining which part of the file we are viewing
			- This can be done with the y index / px_sz for lines (x are a little more complicated due to proportional fonts)
				- What about visual line wrapping?
		- Scrolling
		- Line wrapping

- Only render when necessary
	- Benefits:
		- Only rendering on changes means we don't need to focus on "global" framerate as much
	- Considerations:
		- Animations?
		- Caching unchanged pieces

It's probably the best idea to combine all of these
