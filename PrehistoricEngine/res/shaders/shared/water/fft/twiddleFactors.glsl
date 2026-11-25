void main(void)
{
	vec2 x = gl_GlobalInvocationID.xy;
	float k = mod(x.y * (float(N) / pow(2, x.x + 1)), N);
    vec2 twiddle = vec2(cos(2.0 * M_PI * k / float(N)), sin(2.0 * M_PI * k / float(N)));
	
	int butterflyspan = int(pow(2, x.x));
	
	int butterflywing;
	
	if (mod(x.y, pow(2, x.x + 1)) < pow(2, x.x))
		butterflywing = 1;
	else butterflywing = 0;

	// first stage, bit reversed indices
	if (x.x == 0) 
	{
		// top butterfly wing
		if (butterflywing == 1)
        imageStore(twiddleIndices, ivec2(x), vec4(twiddle, reverseIndex(int(x.y)), reverseIndex(int(x.y + 1))));
		// bot butterfly wing
		else
        imageStore(twiddleIndices, ivec2(x), vec4(twiddle, reverseIndex(int(x.y - 1)), reverseIndex(int(x.y))));
	}
	// second to log2(N) stage
	else 
	{
		// top butterfly wing
		if (butterflywing == 1)
        imageStore(twiddleIndices, ivec2(x), vec4(twiddle, x.y, x.y + butterflyspan));
		// bot butterfly wing
		else
        imageStore(twiddleIndices, ivec2(x), vec4(twiddle, x.y - butterflyspan, x.y));
	}
}