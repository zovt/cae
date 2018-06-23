# cae
cae is an experimental, mouse-based, integrating programming environment.

The rendering is done via Vulkan currently. The use of multiple graphics backends
is a consideration, but nothing has been implemented yet.

# Goals
1. cae should integrate with the surrounding system as best as possible
2. cae should be fast
3. cae should render text well
4. cae should be simple to configure

# Platform Support
Tier 1: Linux (where cae is primarily developed)

Tier 2: macOs (??? depending on if we can get vulkan running there)

Tier 3: Windows (untested, probably requires significant changes)

# Building
## Requirements
- C++17 compiler
- fontconfig (*nix)
- glfw3
- vulkan headers, [glslangValidator](https://github.com/KhronosGroup/glslang)

# Prior art
Some text editors that have inspired cae

- acme
- sam
- vi-family editors
- ed
