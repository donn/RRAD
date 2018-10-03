CXX= c++
CXX_FLAGS= -pedantic -g -std=c++14
LINKER_FLAGS = -fPIC -shared

SRC_DIR = src
BUILD_DIR = build

HEADERS = $(shell find include -name "*.h")
SOURCES = $(shell find src -name "*.cpp")
OBJECTS = $(addprefix build/, $(patsubst %.cpp,%.o,$(SOURCES)))

OS = $(shell uname)
ifeq ($(OS),Darwin)
	LIB = rrac.dylib
else
	LIB = rrac.so
endif
all: debug

debug: CXX_FLAGS += -g
debug: $(LIB)
release: $(LIB)

$(OBJECTS): build/%.o : %.cpp $(HEADERS)
	@mkdir -p $(@D)
	@$(CXX) -Iinclude $(CXX_FLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	@$(CXX) $(LINKER_FLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -rf build/
	rm -f $(LIB)