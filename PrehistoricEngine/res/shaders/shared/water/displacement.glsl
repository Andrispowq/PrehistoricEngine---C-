void displaceWater(inout vec4 p, vec2 uv, out vec2 waveUV)
{
    waveUV = uv * tiling + wind * motion;
    float fade = clamp(1.0 - distance(p.xyz, cameraPosition) / max(float(displacementRange), 1.0), 0.0, 1.0);
    p.y = waterHeight + textureLod(Dy, uv, 0).r * scaleY;
    p.y += fade * textureLod(Dy, waveUV, 0).r * displacementScale;
    if (choppiness != 0.0)
    {
        p.xz -= fade * choppiness * vec2(textureLod(Dx, waveUV, 0).r, textureLod(Dz, waveUV, 0).r);
    }
}
