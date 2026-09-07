#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

layout (binding = 0, set = 1) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    vec3 color;
    vec4 specular;
    vec4 param1;
} ubo;

void main() {
    outColor = vec4(1, 1, 1, 1);
}
