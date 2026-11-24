#version 430 core
#define M_PI 3.1415926535897932384626433832795

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) writeonly uniform image2D tilde_h0k;
layout (binding = 1, rgba32f) writeonly uniform image2D tilde_h0minusk;

layout (binding = 2) uniform sampler2D noise_r0;
layout (binding = 3) uniform sampler2D noise_i0;
layout (binding = 4) uniform sampler2D noise_r1;
layout (binding = 5) uniform sampler2D noise_i1;

uniform	int N;
uniform int L;
uniform float amplitude;
uniform float intensity;
uniform vec2 direction;
uniform float l;
uniform float alignment;

#include "shared/water/fft/h0k.glsl"
