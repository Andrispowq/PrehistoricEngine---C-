#version 430

layout(location = 0) out vec4 outBloom;

in vec2 texture_FS;

uniform sampler2D scene;

uniform float threshold;

void main()
{
    vec3 bloomColour = texture(scene, texture_FS).rgb;
	float intensity = dot(bloomColour, vec3(0.2126, 0.7152, 0.0722));
	if(intensity < threshold)
	{
		bloomColour = vec3(0.0);
	}

	bloomColour = clamp(bloomColour, 0.0, 500.0);

	outBloom = vec4(bloomColour, 1.0);
}