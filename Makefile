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

ifeq ($(build), release)
	CFLAGS = -Wall -std=c++20 -static -O3 -mtune=native -static-libgcc -static-libstdc++ -pie -lm -fstrict-aliasing -fno-exceptions -fno-rtti -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Ofast -fomit-frame-pointer -DIS_64BIT -DUSE_AVX2 -mavx2 -DUSE_SSE41 -DUSE_SSE3 -msse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse -fexceptions -fPIC
else
	CFLAGS = -Wall -std=c++20 -static -O3 -DNDEBUG -mtune=native -static-libgcc -static-libstdc++ -pie -lm -fstrict-aliasing -fno-exceptions -fno-rtti -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-maybe-uninitialized -Ofast -fomit-frame-pointer -DIS_64BIT -DUSE_AVX2 -mavx2 -DUSE_SSE41 -DUSE_SSE3 -msse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse -fexceptions -fPIC
endif

SRC_DIRS := Sources pystring nnue
SRCS := $(wildcard $(addsuffix /*.cpp, $(SRC_DIRS))) main.cpp
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

GibnaMish: $(OBJS)
	g++ -o GibnaMish $(OBJS) $(CFLAGS)

%.o: %.cpp
	g++ -c $(CFLAGS) -flto $(TARGET) -std=c++20 -Wall $< -o $@

clean:
	rm -f $(OBJS) GibnaMish