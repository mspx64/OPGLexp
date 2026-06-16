#shader Vertex
#version 450 core

// Input attributes
layout(location = 0) in vec3 a_Position;

// Uniform matrices (updated naming convention)
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

// Optional animation uniforms
uniform float u_time;
uniform bool u_enableAnimation;
uniform float u_waveAmplitude;
uniform float u_waveFrequency;

// Outputs to fragment shader
layout(location = 0) out vec3 v_WorldPos;
layout(location = 1) out vec3 v_LocalPos;
layout(location = 2) out float v_WaveHeight;

void main()
{
    vec3 displaced = a_Position;
    v_LocalPos = a_Position;
    
    // Optional wave animation for space-time distortion
    if (u_enableAnimation) {
        float wave1 = sin(a_Position.x * u_waveFrequency + u_time) * u_waveAmplitude;
        float wave2 = cos(a_Position.z * u_waveFrequency * 0.7 + u_time * 1.3) * u_waveAmplitude;
        displaced.y += wave1 + wave2 * 0.5;
        v_WaveHeight = wave1 + wave2 * 0.5;
    } else {
        v_WaveHeight = 0.0;
    }
    
    // Transform to world space
    v_WorldPos = vec3(u_model * vec4(displaced, 1.0));
    
    // Final position
    gl_Position = u_projection * u_view * vec4(v_WorldPos, 1.0);
}

#shader Fragment
#version 450 core



// Inputs from vertex shader
layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_LocalPos;
layout(location = 2) in float v_WaveHeight;

// Output
layout(location = 0) out vec4 FragColor;

// Uniforms
uniform vec3 u_viewPos;
uniform vec3 u_baseColor;
uniform float u_fadeDistance;
uniform float u_gridIntensity;
uniform float u_time;
uniform bool u_enableGrid;
uniform bool u_enableGradient;
uniform vec3 u_gradientColor;

// Grid calculation function
float getGrid(vec2 pos, float scale) {
    vec2 grid = abs(fract(pos * scale - 0.5) - 0.5) / fwidth(pos * scale);
    float line = min(grid.x, grid.y);
    return 1.0 - min(line, 1.0);
}

// Smooth distance-based fading
float smoothFade(float dist, float maxDist, float falloff) {
    float normalizedDist = dist / maxDist;
    return pow(1.0 - clamp(normalizedDist, 0.0, 1.0), falloff);
}

void main()
{
    vec3 finalColor = u_baseColor;
    
    // Calculate distance-based fade
    float dist = distance(u_viewPos, v_WorldPos);
    float fade = smoothFade(dist, u_fadeDistance, 2.0);
    
    // Optional grid overlay
    if (u_enableGrid) {
        float grid1 = getGrid(v_LocalPos.xz, 1.0) * 0.3;
        float grid2 = getGrid(v_LocalPos.xz, 0.1) * 0.1;
        float gridEffect = (grid1 + grid2) * u_gridIntensity;
        finalColor += gridEffect;
    }
    
    // Optional gradient effect based on height/wave
    if (u_enableGradient) {
        float heightFactor = (v_WaveHeight + 1.0) * 0.5; // Normalize to 0-1
        vec3 gradientContrib = u_gradientColor * heightFactor * 0.3;
        finalColor += gradientContrib;
    }
    
    // Enhance fade with wave animation
    float animatedFade = fade;
    if (v_WaveHeight != 0.0) {
        animatedFade *= (1.0 + abs(v_WaveHeight) * 0.2);
    }
    
    // Distance-based color shift (closer = brighter)
    float proximity = 1.0 - clamp(dist / (u_fadeDistance * 0.5), 0.0, 1.0);
    finalColor += proximity * 0.1;
    
    // Apply final fade
    FragColor = vec4(finalColor, animatedFade);
}