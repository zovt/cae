CXXFLAGS := -std=c++17 -g -Wall -Wextra \
	-Wpedantic ${CXXFLAGS} \
	-Ibuild/include \
	-fno-exceptions \
	$(shell pkg-config --cflags fontconfig) \
	$(shell pkg-config --cflags glfw3) \
	$(shell pkg-config --cflags glew)

LDFLAGS := ${LDFLAGS} $(shell pkg-config --static --libs fontconfig) \
	$(shell pkg-config --static --libs glfw3) \
	$(shell pkg-config --static --libs glew)

src = $(shell find src/ -name *.cc)
headers = $(shell find src/ -name *.hh)
obj = $(shell echo $(src) | sed 's/src/build/g; s/\.cc/\.o/g')
build_dirs = $(shell find src/ -type d | sed 's/src/build/g')
resources = $(shell ./build_scripts/get_resource_headers.sh)

cae: build build/cae

build:
	mkdir $(build_dirs) build/include

$(resources):
	./build_scripts/compile_resources.sh $(subst build/include/,,$(subst .hh,,$@))

$(obj):
	$(CXX) -c -o $@ $(subst build,src,$(subst .o,.cc,$@)) $(CXXFLAGS)

build/cae: $(resources) $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean

clean:
	rm -r build
