#version 430 core
#define M_PI 3.1415926535897932384626433832795

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba32f) uniform readonly image2D twiddlesIndices;
layout (binding = 1, rgba32f) uniform image2D pingpong0;
layout (binding = 2, rgba32f) uniform image2D pingpong1;

uniform int stage;
uniform int pingpong;
uniform int direction;

struct Complex
{	
	float real;
	float im;
};

Complex Mul(Complex c0, Complex c1)
{
	Complex c;
	c.real = c0.real * c1.real - c0.im * c1.im;
	c.im   = c0.real * c1.im + c0.im * c1.real;
	return c;
}

Complex Add(Complex c0, Complex c1)
{
	Complex c;
	c.real = c0.real + c1.real;
	c.im   = c0.im   + c1.im;
	return c;
}


void HorizontalButterflies()
{
	Complex H;
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	
	if(pingpong == 0)
	{
		vec4 data = imageLoad(twiddlesIndices, ivec2(stage, x.x)).rgba;
		vec2 p_ = imageLoad(pingpong0, ivec2(data.z, x.y)).rg;
		vec2 q_ = imageLoad(pingpong0, ivec2(data.w, x.y)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w,q));
		
		imageStore(pingpong1, x, vec4(H.real, H.im, 0, 1));
	} 
	else if(pingpong == 1)
	{
		vec4 data = imageLoad(twiddlesIndices, ivec2(stage, x.x)).rgba;
		vec2 p_ = imageLoad(pingpong1, ivec2(data.z, x.y)).rg;
		vec2 q_ = imageLoad(pingpong1, ivec2(data.w, x.y)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w,q));
		
		imageStore(pingpong0, x, vec4(H.real, H.im, 0, 1));
	}
}

void VerticalButterflies()
{
	Complex H;
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	
	if(pingpong == 0)
	{
		vec4 data = imageLoad(twiddlesIndices, ivec2(stage, x.y)).rgba;
		vec2 p_ = imageLoad(pingpong0, ivec2(x.x, data.z)).rg;
		vec2 q_ = imageLoad(pingpong0, ivec2(x.x, data.w)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w,q));
		
		imageStore(pingpong1, x, vec4(H.real, H.im, 0, 1));
	} 
	else if(pingpong == 1)
	{
		vec4 data = imageLoad(twiddlesIndices, ivec2(stage, x.y)).rgba;
		vec2 p_ = imageLoad(pingpong1, ivec2(x.x, data.z)).rg;
		vec2 q_ = imageLoad(pingpong1, ivec2(x.x, data.w)).rg;
		vec2 w_ = vec2(data.x, data.y);
		
		Complex p = Complex(p_.x,p_.y);
		Complex q = Complex(q_.x,q_.y);
		Complex w = Complex(w_.x,w_.y);
		
		//Butterfly operation
		H = Add(p, Mul(w,q));
		
		imageStore(pingpong0, x, vec4(H.real, H.im, 0, 1));
	}
}

void main(void)
{
	if(direction == 0)
		HorizontalButterflies();
	else if(direction == 1)
		VerticalButterflies();
}