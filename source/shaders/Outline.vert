#version 450
#extension GL_ARB_separate_shader_objects : enable

// this time, positions and colors are sent by the application
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;

layout (binding = 0, set = 1) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    vec3 color;
    vec4 specular;
    vec4 param1;
} ubo;

void main() {
    float thickness = ubo.param1.x;
    vec3 extended = inPos + normalize(inNorm) *  thickness;
    gl_Position = ubo.mvpMat * vec4(extended, 1.0f);
}