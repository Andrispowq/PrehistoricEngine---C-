layout(set=2, binding=2, std140) uniform WaterParameters
{
    vec4 tessellation; // factor, slope, shift, wireframe flag
    vec4 waves; // vertical scale, detail displacement, choppiness, fade range
    vec4 animation; // wind XY, phase, tiling
    vec4 optics; // reflection distortion, refraction distortion, absorption, water height
    vec4 waterDetail; // strength, normal tiling, distortion tiling, refraction depth scale
    vec4 tint; // RGB, underwater flag
    vec4 sunlight; // direction, specular exponent
    vec4 radiance;
};
