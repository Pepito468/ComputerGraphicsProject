#version 450

layout(location = 0) out vec4 outColor;

// layout(location = 0) in vec3 fragPos;
// layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(binding = 1, set = 1) uniform sampler2D tex;

void main()
{
    outColor = texture(tex, fragUV);
}
