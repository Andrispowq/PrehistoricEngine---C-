#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform image2D displacement;
layout (binding = 1, rgba32f) uniform image2D pingpong0;
layout (binding = 2, rgba32f) uniform image2D pingpong1;

uniform int pingpong;
uniform int N;

#include "shared/water/fft/inversion.glsl"
