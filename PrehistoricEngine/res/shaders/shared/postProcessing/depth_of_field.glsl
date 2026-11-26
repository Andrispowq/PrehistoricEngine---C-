// Signed circle of confusion: positive radii lie behind the focal plane.
// apertureScale converts the entrance pupil diameter to a projected pixel radius.
vec3 depthOfFieldColour(vec2 uv, vec2 resolution, vec2 projectedDisk, float focusDistance, float apertureScale, vec3 center)
{
    const float maxRadius = 12.0;
    const int taps = 48;
    float focus = 1.0 / focusDistance;
    float centerDepth = inverseViewDepth(uv);
    float centerCoC = clamp(apertureScale * (focus - centerDepth), -maxRadius, maxRadius);
    vec3 sum = center;
    float total = 1.0;
    vec3 foreground = vec3(0);
    float coverage = 0.0;
    // The editor can display the backing texture at a different aspect ratio.
    // Both axes of the projected lens disk already include the camera projection scale.
    vec2 pixelStep = vec2(projectedDisk.x / projectedDisk.y, 1.0) / resolution.y;
    vec2 direction = vec2(1, 0);
    // Rotate through a disk by the golden angle. Weight each radial interval by its area.
    const mat2 turn = mat2(-0.7373689, 0.6754903, -0.6754903, -0.7373689);
    for (int i = 0; i < taps; ++i)
    {
        float radius = (float(i) + 0.5) * maxRadius / float(taps);
        vec2 pos = clamp(uv + direction * radius * pixelStep, 0.5 / resolution, 1.0 - 0.5 / resolution);
        direction = turn * direction;
        float depth = inverseViewDepth(pos);
        float coc = clamp(apertureScale * (focus - depth), -maxRadius, maxRadius);
        float area = 2.0 * radius * maxRadius / float(taps);
        if (coc < min(centerCoC - 0.5, -0.5)) {
            // Near objects spread over the background in proportion to their disk coverage.
            float weight = area * clamp(-coc - radius + 0.5, 0.0, 1.0) / (coc * coc);
            if (weight > 0.0) {
                foreground += sampleLensColour(pos) * weight;
                coverage += weight;
            }
        } else {
            float footprint = abs(centerCoC);
            if (depth > centerDepth) footprint = min(footprint, abs(coc));
            float weight = area * clamp(footprint - radius, 0.0, 1.0);
            if (weight > 0.0) {
                sum += sampleLensColour(pos) * weight;
                total += weight;
            }
        }
    }
    vec3 blurred = sum / total;
    return mix(blurred, foreground / max(coverage, 0.00001), min(coverage, 1.0));
}
