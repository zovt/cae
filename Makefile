CXXFLAGS:=-Wall -Wextra -Wpedantic ${CXXFLAGS}
LDFLAGS:=${LDFLAGS}

src = $(wildcard src/*.cc)
obj = $(src:src/%.c=build/%.o)

cae: build build/cae

build:
	mkdir build

build/%.o: src/%.c
	$(CXX) -c -o $@ $< $(CXXFLAGS)

build/cae: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -r build
