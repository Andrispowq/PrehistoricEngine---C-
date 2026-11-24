#version 430 core
#define M_PI 3.1415926535897932384626433832795

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform readonly image2D twiddlesIndices;
layout (binding = 1, rgba32f) uniform image2D pingpong0;
layout (binding = 2, rgba32f) uniform image2D pingpong1;

uniform int stage;
uniform int pingpong;
uniform int direction;

#include "shared/water/fft/butterfly.glsl"
