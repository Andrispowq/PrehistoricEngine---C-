void main(void)
{
	ivec2 x = ivec2(gl_GlobalInvocationID.xy);
	
	float perms[] = { 1.0, -1.0 };
	int index = int(mod((int(x.x + x.y)), 2));
	float perm = perms[index];
	
	if(pingpong == 0)
	{
		float h = imageLoad(pingpong0, x).r;
		imageStore(displacement, x, vec4(perm * (h / float(N * N)), perm * (h / float(N * N)), perm * (h / float(N * N)), 1));
	}
	else if(pingpong == 1)
	{
		float h = imageLoad(pingpong1, x).r;
		imageStore(displacement, x, vec4(perm * (h / float(N * N)), perm * (h / float(N * N)), perm * (h / float(N * N)), 1));
	}
}