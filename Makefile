CXXFLAGS = -std=c++14 -Wunused -Wall -Wextra -Wpedantic -I src/ -O3 -march=native -msse4 -mfpmath=sse -ffast-math -g
# CXXFLAGS = -std=c++14 -Wunused -Wall -Wextra -Wpedantic -I src/ -g
LDFLAGS  = -lglfw -lGLEW -lGL

voxelator: src/main.o src/ext/stb/stb_image_pre.o
	$(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

all: voxelator

apitrace: clean voxelator
	apitrace trace ./voxelator
	qapitrace voxelator.trace

clean:
	find . -name '*.o' -type f -delete
	find . -name '*.trace' -type f -delete
	find . -name voxelator -type f -delete
