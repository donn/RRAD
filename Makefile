CXX= c++
CXX_FLAGS= -pedantic -std=c++17 -fPIC
CXX_FLAGS_LIB= -fpermissive -std=c++17 -fPIC 
LINKER_FLAGS = -shared

LIB_HEADERS = $(shell find libinclude -name "*.h")
LIB_SOURCES = $(shell find libsrc -name "*.cpp")
LIB_OBJECTS = $(addprefix build/, $(patsubst %.cpp,%.o,$(LIB_SOURCES)))
PRECOMPILED_HEADERS = $(addprefix build/, $(patsubst %,%.pch,$(LIB_HEADERS)))

# We're gonna have to agree on one compiler to use this, so put it to the side for now

HEADERS = $(shell find include -name "*.h") $(LIB_HEADERS)
SOURCES = $(shell find src -name "*.cpp")
OBJECTS = $(addprefix build/, $(patsubst %.cpp,%.o,$(SOURCES)))

OS = $(shell uname)
ifeq ($(OS),Darwin)
	LIB = librrad.dylib
else
	LIB = librrad.so
endif
all: debug

debug: CXX_FLAGS += -g
debug: $(LIB)
release: $(LIB)

$(LIB_OBJECTS): build/%.o : %.cpp $(HEADERS)
	mkdir -p $(@D)
	$(CXX) -Ilibinclude $(CXX_FLAGS_LIB) -c -o $@ $<

$(OBJECTS): build/%.o : %.cpp $(HEADERS)
	mkdir -p $(@D)
	$(CXX) -Iinclude -Ilibinclude $(CXX_FLAGS) -c -o $@ $<

$(LIB): $(OBJECTS) $(LIB_OBJECTS)
	$(CXX) $(LINKER_FLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -rf build/
	rm -rf *.dSYM/
	rm -f $(LIB)
	rm -f *.out
