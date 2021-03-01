GCC = g++ -std=c++17 
#MPICC = mpicxx  -std=c++17 -g  
MPICC = /home/jie.yang/test/mpich/build/_inst/bin/mpicxx -std=c++17 -g
CXX      := -c++
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror
MPICCFLAGS = -O0 -Wall -Wfatal-errors -pthread 
LIBRA = lib/libshp.a lib/librtree.a

# LDFLAGS  := -lgeos-3.8.1 -I/opt/geos/include
# LDFLAGS  := -L/home/jie.yang/.local/lib -lgeos
LDFLAGS  := -L/home/jie.yang/.local/lib -lgeos-3.8.1
MAKE_DIR = $(PWD)
OBJ_DIR  := $(MAKE_DIR)/bin
APP_DIR  := $(MAKE_DIR)
TARGET   := prog
INCLUDE  := -I/usr/local/include -I/home/jie.yang/.local/include
SRC      :=                      \
   $(wildcard src/*.cpp)         \

#   $(wildcard src/module1/*.cpp) \

OBJECTS := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(MPICC) $(MPICCFLAGS) $(INCLUDE) -o $@ -c $<

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(MPICC) $(MPICCFLAGS) $(INCLUDE) $(LDFLAGS) -o $(APP_DIR)/$(TARGET) $(OBJECTS)

.PHONY: all build clean debug release

build:
#	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

debug: MPICCFLAGS += -DDEBUG -g
debug: all

release: MPICCFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/$(TARGET)
