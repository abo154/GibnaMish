ifeq ($(OS), Windows_NT)
	uname_S := Windows
else
	uname_S := $(shell uname -s)
endif

ifeq ($(uname_S), Darwin)
	TARGET = -mcpu=apple-a14
else
	TARGET = -march=native
endif

CFLAGS = -Wall -std=c++20 -static -O3 -DNDEBUG -mtune=native -Wcast-qual -static-libgcc -static-libstdc++ -pie -lm -fno-exceptions -fno-rtti -Wextra -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-maybe-uninitialized -pedantic -funroll-loops -fPIC -flto

OBJ_DIR := obj
SRC_DIRS := Sources
SRCS := $(wildcard $(addsuffix /*.cpp, $(SRC_DIRS))) main.cpp
OBJS := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))

GibnaMish: $(OBJ_DIR) $(OBJS)
	g++ -o GibnaMish $(OBJS) $(CFLAGS)
ifeq ($(OS), Windows_NT)
	-rmdir /S /Q $(OBJ_DIR)
else
	rm -rf $(OBJ_DIR)
endif

$(OBJ_DIR):
ifeq ($(OS), Windows_NT)
	mkdir $(OBJ_DIR)
else
	mkdir -p $(OBJ_DIR)
endif

$(OBJ_DIR)/%.o: $(SRC_DIRS)/%.cpp
	g++ -c $(CFLAGS) -flto $(TARGET) -std=c++20 -Wall $< -o $@

$(OBJ_DIR)/main.o: main.cpp
	g++ -c $(CFLAGS) -flto $(TARGET) -std=c++20 -Wall $< -o $@

clean:
ifeq ($(OS), Windows_NT)
	-del /Q $(OBJ_DIR)\*.o GibnaMish.exe
	-rmdir /S /Q $(OBJ_DIR)
else
	rm -f $(OBJ_DIR)/*.o GibnaMish
	rm -rf $(OBJ_DIR)
endif
