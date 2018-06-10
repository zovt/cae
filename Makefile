CXXFLAGS := -std=c++14 -g -Wall -Wextra \
	-Wpedantic ${CXXFLAGS} -I./lib/cpptoml/include \
	$(shell pkg-config --cflags fontconfig)
LDFLAGS := ${LDFLAGS} $(shell pkg-config --libs fontconfig)

src = $(wildcard src/*.cc)
headers = $(wildcard src/*.hh)
obj = $(src:src/%.cc=build/%.o)

cae: build build/cae

build:
	mkdir build

build/%.o: src/%.cc $(headers)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

build/cae: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -r build
