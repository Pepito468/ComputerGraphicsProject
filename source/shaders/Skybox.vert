#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0, set = 0) uniform SkyboxUBO {
    mat4 mvpMat;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragDir;

void main() {

    gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);
    fragDir     = inPosition;
}
