CXX      := -c++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -O3 -pthread -march=native
LDFLAGS  := -L/usr/lib -lstdc++ -lm -lSDL2 -lSDL2_ttf -ltbb
OBJ_DIR  := ./build
BIN_DIR  := ./bin
TARGET   := tifo
INCLUDE  := -I include/
SRC      := $(wildcard src/*.cc)

OBJECTS  := $(SRC:%.cc=$(OBJ_DIR)/%.o)
DEPENDENCIES \
         := $(OBJECTS:.o=.d)

all: build $(BIN_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -MMD -o $@

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(TARGET) $^ $(LDFLAGS)

-include $(DEPENDENCIES)

.PHONY: all build clean debug release info

build:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

exe: all
	./$(BIN_DIR)/$(TARGET)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(BIN_DIR)/*
