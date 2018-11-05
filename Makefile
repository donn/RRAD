CXX= c++
CXX_FLAGS= -pedantic -std=c++17 -fPIC
LINKER_FLAGS = -shared

SRC_DIR = src
BUILD_DIR = build

LIB_HEADERS = $(shell find libinclude -name "*.h")
PRECOMPILED_HEADERS = $(addprefix build/, $(patsubst %,%.pch,$(LIB_HEADERS)))
# We're gonna have to agree on one compiler to use this, so put it to the side for now

HEADERS = $(shell find include -name "*.h") $(LIB_HEADERS)
SOURCES = $(shell find src -name "*.cpp")
OBJECTS = $(addprefix build/, $(patsubst %.cpp,%.o,$(SOURCES)))

OS = $(shell uname)
ifeq ($(OS),Darwin)
	LIB = rrad.dylib
else
	LIB = rrad.so
endif
all: debug

debug: CXX_FLAGS += -g
debug: $(LIB)
release: $(LIB)

$(OBJECTS): build/%.o : %.cpp $(HEADERS)
	mkdir -p $(@D)
	$(CXX) -Iinclude -Ilibinclude $(CXX_FLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	$(CXX) $(LINKER_FLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -rf build/
	rm -rf *.dSYM/
	rm -f $(LIB)
	rm -f *.out
