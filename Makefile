CXXFLAGS := -std=c++17 -g -Wall -Wextra \
	-Wpedantic ${CXXFLAGS} \
	-Ibuild/include \
	-Ilib/stb \
	-Ilib/GSL/include \
	-fno-exceptions \
	$(shell pkg-config --cflags fontconfig glfw3 glew)

LDFLAGS := ${LDFLAGS} $(shell pkg-config --static --libs fontconfig glew glfw3) -lstdc++fs

src = $(shell find src/ -name *.cc)
headers = $(shell find src/ -name *.hh)
obj = $(shell echo $(src) | sed 's/src/build/g; s/\.cc/\.o/g')
build_dirs = $(shell find src/ -type d | sed 's/src/build/g')
resources = $(shell ./build_scripts/get_resource_headers.sh)

cae: build build/cae

build:
	mkdir $(build_dirs) build/include

$(resources): build/include/%.hh: %
	./build_scripts/compile_resources.sh $<

$(obj): build/%.o: src/%.cc $(headers) $(resources)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

build/cae: $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean

clean:
	rm -r build
