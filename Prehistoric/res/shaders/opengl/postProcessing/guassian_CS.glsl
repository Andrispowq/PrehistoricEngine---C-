#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColour;

uniform sampler2D scene;
uniform vec2 screenSize;

#define KERNEL_SIZE 5
#define SIGMA 10

uniform int horizontal;
uniform float weight[KERNEL_SIZE] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

/*shared float weight[KERNEL_SIZE];

float erf(float x)
{
    const float a1 = 0.254829592;
    const float a2 = -0.284496736;
    const float a3 = 1.421413741;
    const float a4 = -1.453152027;
    const float a5 = 1.061405429;
    const float p = 0.3275911;

    // A&S formula 7.1.26
    float t = 1.0 / (1.0 + p * abs(x));
    float y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);

    return sign(x) * y;
}

float def_int_gaussian(float x, float mu, float sigma) 
{
    return 0.5 * erf((x - mu) / (sqrt(2) * sigma));
}

void generate_kernel()
{
    float end = 0.5 * KERNEL_SIZE;
    float start = -end;
    int index = 0;

    float sum = 0;
    float x = start;
    float last_int = def_int_gaussian(x, 0, SIGMA);
    float acc = 0;

    while (x < end)
    {
        x += step;
        float new_int = def_int_gaussian(x, 0, SIGMA);
        float c = new_int - last_int;
        weight[index++] = c;
        sum += c;
        last_int = new_int;
    }

    //normalize
    sum = 1 / sum;
    for (int i = 0; i < KERNEL_SIZE; i++)
    {
        weight[i] *= sum;
    }
}*/

void main()
{

    ivec2 x = ivec2(gl_GlobalInvocationID.xy);
    vec2 texCoord = gl_GlobalInvocationID.xy / screenSize;
	vec2 texelSize = 1.0 / screenSize * 3;
	
	vec3 combined = vec3(0);
	combined = texture(scene, texCoord).rgb * weight[0];

    /*if (gl_GlobalInvocationID == 0)
        generate_kernel();

    barrier();*/
	
	if(horizontal == 1)
	{	
        for(int i = 1; i < KERNEL_SIZE; ++i)
        {
            combined += texture(scene, texCoord + vec2(texelSize.x * i, 0.0)).rgb * weight[i];
            combined += texture(scene, texCoord - vec2(texelSize.x * i, 0.0)).rgb * weight[i];
        }
	}
	else
	{
		for(int i = 1; i < 5; ++i)
        {
            combined += texture(scene, texCoord + vec2(0.0, texelSize.y * i)).rgb * weight[i];
            combined += texture(scene, texCoord - vec2(0.0, texelSize.y * i)).rgb * weight[i];
        }
	}

    imageStore(outColour, x, vec4(combined, 1.0));
}