vec3 underwaterColour(sampler2D scene, vec2 texCoord, vec2 resolution, vec3 sceneColour)
{
    vec2 stepUV = 2.0 / resolution;
    vec3 haze = (texture(scene, texCoord + vec2(stepUV.x, 0)).rgb +
             texture(scene, texCoord - vec2(stepUV.x, 0)).rgb +
             texture(scene, texCoord + vec2(0, stepUV.y)).rgb +
             texture(scene, texCoord - vec2(0, stepUV.y)).rgb) * 0.25;
    return mix(sceneColour, haze, 0.45) * vec3(0.45, 0.79, 0.84) + vec3(0.008, 0.035, 0.045);
}
