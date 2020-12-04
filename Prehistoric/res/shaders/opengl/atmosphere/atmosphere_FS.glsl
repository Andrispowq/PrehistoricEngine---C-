#version 430

in vec3 position_FS;

layout(location = 0) out vec4 out_Colour;

const vec3 baseColor = vec3(0.18, 0.27, 0.47);  //original color : 0.18, 0.27, 0.47

void main()
{
	float red = -0.00022 * (abs(position_FS.y) - 2800) + baseColor.x;
	float green = -0.00025 * (abs(position_FS.y) - 2800) + baseColor.y;
	float blue = -0.00019 * (abs(position_FS.y) - 2800) + baseColor.z;
	
	out_Colour = vec4(red, green, blue, 1);
}