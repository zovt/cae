# cae
cae is an experimental, mouse-based, integrating programming environment.

The rendering is done via OpenGL currently. The use of multiple graphics backends
is a consideration, but nothing has been implemented yet.

# Self-dogfooding
As of Mon, 9 Jul 2018, the primary author of cae (@zovt) is using it as his primary
text editor

# Goals
1. cae should integrate with the surrounding system as best as possible
2. cae should be fast
3. cae should render text well
4. cae should be simple to configure

# Platform Support
Tier 1: Linux (where cae is primarily developed)

Tier 2: macOs (@zovt does not have a Mac to test on, but it should probably work)

Tier 3: Windows (untested, probably requires significant changes)

# Building
## Requirements
- C++17 compiler (with std::filesystem support)
- fontconfig (*nix)
- glfw3
- glew
- glm

## Making
```
git submodule init
# set your CXXFLAGS if you'd like. -DCAE_DEBUG enables debug output
make
```

# Prior art
Some text editors that have inspired cae

- acme
- sam
- vi-family editors
- ed
