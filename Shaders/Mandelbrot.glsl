#version 460

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

uniform float uTime;
uniform vec2 uResolution;
uniform vec2 uMouse;

// Mandelbrot set computation
vec3 mandelbrot(vec2 c) {
    vec2 z = vec2(0.0);
    int iterations = 0;
    const int max_iterations = 256;

    for (int i = 0; i < max_iterations; ++i) {
        if (dot(z, z) > 4.0) break;
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        iterations++;
    }

    if (iterations == max_iterations) {
        return vec3(0.0);// Inside the set - black
    }

    // Smooth coloring with time-based animation
    float smooth_iter = float(iterations) + 1.0 - log2(log2(dot(z, z)));
    float t = smooth_iter / float(max_iterations);

    // Animated color palette
    vec3 color = vec3(
    0.5 + 0.5 * sin(3.0 * t + uTime * 0.5),
    0.5 + 0.5 * sin(3.0 * t + uTime * 0.5 + 2.0),
    0.5 + 0.5 * sin(3.0 * t + uTime * 0.5 + 4.0)
    );

    return color;
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

    // Convert pixel coordinates to complex plane
    vec2 uv = vec2(pixel_coords) / uResolution;

    // Static center position (interesting point on the Mandelbrot set)
    // Some intersting points:
    // Misiurewicz Point: -0.77568377, 0.13646737 (good to ~10^-50)
    // Feigenbaum Point: -1.25066, 0.02012 (famous for period-doubling)
    // Deep Spiral: -0.7269, 0.1889 (beautiful spiral structures)
    // Ultra Deep: -0.74529, 0.11307 (suitable for extreme zooms)
    vec2 center = vec2(-0.74529, 0.11307);

    // Zoom in over time (exponential zoom for dramatic effect)
    float zoom = exp(-uTime * 0.33);// Starts at 1.0 and zooms in exponentially
    // Alternative linear zoom (slower):
    // float zoom = max(0.01, 2.0 - uTime * 0.1);

    // Map to complex plane
    vec2 c = (uv - 0.5) * 4.0 * zoom + center;

    vec3 color = mandelbrot(c);

    imageStore(img_output, pixel_coords, vec4(color, 1.0));
}