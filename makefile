CXX := g++-9
CXXFLAGS := -std=c++17 -Wall -Werror -Wextra -pedantic-errors
LDFLAGS := -lm -lncurses
BUILD := ./build
OBJ_DIR := $(BUILD)/objects
APP_DIR := $(BUILD)/apps
TARGET := term_shapes
INCLUDE := -Iinclude/
SRC := $(wildcard src/*cc)

OBJECTS := $(SRC:%.cc=$(OBJ_DIR)/%.o)

all: main

main: src/main.cc
	$(CXX) $(CXXFLAGS) -o $(TARGET) $<

.PHONY: all build clean debug release

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O3
release: all

clean:
	-@rm -rvf $(BUILD)
	-@rm -rvf $(TARGET)
