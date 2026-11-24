#version 430 core
#define M_PI 3.1415926535897932384626433832795

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) writeonly uniform image2D tilde_hkt_dy; //height displacement
layout (binding = 1, rgba32f) writeonly uniform image2D tilde_hkt_dx; //choppy-x displacement
layout (binding = 2, rgba32f) writeonly uniform image2D tilde_hkt_dz; //choppy-z displacement
layout (binding = 3, rgba32f) readonly uniform image2D tilde_h0k;
layout (binding = 4, rgba32f) readonly uniform image2D tilde_h0minusk;

uniform int N;
uniform int L;
uniform float t;

#define readSpectrum(img, uv) imageLoad(img, uv)
#include "shared/water/fft/hkt.glsl"
