CXXFLAGS := -std=c++14 -Wall -Wextra -Wpedantic -fno-exceptions ${CXXFLAGS} -I./lib/cjson
LDFLAGS := ${LDFLAGS}

src = $(wildcard src/*.cc)
headers = $(wildcard src/*.hh)
obj = $(src:src/%.cc=build/%.o)

cae: build build/cae

build:
	mkdir build

build/%.o: src/%.cc $(headers)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

build/cae: $(obj) build/cjson.o
	$(CXX) -o $@ $^ $(LDFLAGS)

build/cjson.o:
	$(CXX) -c -o $@ lib/cjson/cJSON.c $(CXXFLAGS)

.PHONY: clean

clean:
	rm -r build
