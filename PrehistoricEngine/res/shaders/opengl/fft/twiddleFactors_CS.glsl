#version 430 core
#define M_PI 3.1415926535897932384626433832795

layout (local_size_x = 1, local_size_y = 16) in;

layout (binding = 0, rgba32f) writeonly uniform image2D twiddleIndices;

layout (std430, binding = 1) buffer indices {
	int j[];
} bit_reversed;

uniform int N;

int reverseIndex(int i) { return bit_reversed.j[i]; }
#include "shared/water/fft/twiddleFactors.glsl"
