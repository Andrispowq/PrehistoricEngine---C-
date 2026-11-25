vec2 multiplyComplex(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

void main(void)
{
	vec2 x = ivec2(gl_GlobalInvocationID.xy) - float(N)/2.0;
	
	vec2 k = vec2(2.0 * M_PI * x.x / L, 2.0 * M_PI * x.y / L);
	
	float magnitude = length(k);
	if (magnitude < 0.00001) magnitude = 0.00001;
	
	float w = sqrt(9.81 * magnitude);

    ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
    vec2 fourier_amp = readSpectrum(tilde_h0k, texel).rg;
    vec2 fourier_amp_conj = readSpectrum(tilde_h0minusk, texel).rg * vec2(1.0, -1.0);

	float cosinus = cos(w*t);
	float sinus   = sin(w*t);
		
	// euler formula
    vec2 exp_iwt = vec2(cosinus, sinus);
    vec2 exp_iwt_inv = vec2(cosinus, -sinus);
	
	// dy
    vec2 h_k_t_dy = multiplyComplex(fourier_amp, exp_iwt) + multiplyComplex(fourier_amp_conj, exp_iwt_inv);
	
	// dx
    vec2 dx = vec2(0.0, -k.x / magnitude);
    vec2 h_k_t_dx = multiplyComplex(dx, h_k_t_dy);
	
	// dz
    vec2 dy = vec2(0.0, -k.y / magnitude);
    vec2 h_k_t_dz = multiplyComplex(dy, h_k_t_dy);

    imageStore(tilde_hkt_dy, ivec2(gl_GlobalInvocationID.xy), vec4(h_k_t_dy, 0, 1));
    imageStore(tilde_hkt_dx, ivec2(gl_GlobalInvocationID.xy), vec4(h_k_t_dx, 0, 1));
    imageStore(tilde_hkt_dz, ivec2(gl_GlobalInvocationID.xy), vec4(h_k_t_dz, 0, 1));
}