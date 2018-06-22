CXXFLAGS := -std=c++17 -g -Wall -Wextra \
	-Wpedantic ${CXXFLAGS} \
	-fno-exceptions \
	$(shell pkg-config --cflags fontconfig) \
	$(shell pkg-config --cflags glfw3) \
	$(shell pkg-config --cflags vulkan)

LDFLAGS := ${LDFLAGS} $(shell pkg-config --static --libs fontconfig) \
	$(shell pkg-config --static --libs glfw3) \
	$(shell pkg-config --static --libs vulkan)

src = $(wildcard src/*.cc)
src += $(wildcard src/*/*.cc)
headers = $(wildcard src/*.hh)
headers += $(wildcard src/*/*.hh)
obj = $(src:src/%.cc=build/%.o)
obj += $(src:src/graphics/%)

cae: build build/cae

build:
	mkdir build && mkdir build/graphics

build/graphics/%.o: src/grapics/%.cc $(headers)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

build/%.o: src/%.cc $(headers)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

build/cae: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -r build
