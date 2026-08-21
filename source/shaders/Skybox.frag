#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragDir;

layout(location = 0) out vec4 outColor;

layout(binding = 1, set = 0) uniform samplerCube envMap;

void main() {
    outColor = texture(envMap, fragDir);
}
