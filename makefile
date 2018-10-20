CXX = c++
CXX_FLAGS = -pedantic -g -std=c++17

SRC_DIR = src
TEST_DIR = misc
HDR_DIR = include
BUILD_DIR = build

HDRS = $(shell find $(HDR_DIR) -name "*.h")

SRCS = $(shell find $(SRC_DIR) -name "*.cpp")
TEST_SRCS = $(shell find $(TEST_DIR) -name "*.cpp")

OBJ = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(SRCS)))
OBJ_TEST = $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp,%.o,$(TEST_SRCS)))


all: $(patsubst %.cpp,%,$(TEST_SRCS))

$(patsubst %.cpp,%,$(TEST_SRCS)): $(OBJ) $(OBJ_TEST)
	@mkdir -p $(BUILD_DIR)/$(@D)
	$(CXX) $(CXX_FLAGS) -lpthread -I $(HDR_DIR) -o ./$(@F) $(OBJ) $(BUILD_DIR)/$@.o

$(OBJ): $(BUILD_DIR)/%.o : %.cpp $(HDRS)
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -c -I $(HDR_DIR) -o $@ $<

$(OBJ_TEST): $(BUILD_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -c -I $(HDR_DIR) -o $@ $<

.PHONY: clean
clean:
	rm -rf build/
	rm -f *.out

