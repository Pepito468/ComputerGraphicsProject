#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 2) in vec2 inUV;

layout(location = 2) out vec2 fragUV;

layout(binding = 0, set = 0) uniform SceneColorUBO {
    mat4 mvpMat;
} scubo;

layout(set = 1, binding = 0) uniform UBO {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    vec3 diffuse;
    vec4 specular;
} ubo;

void main()
{
    gl_Position = scubo.mvpMat * ubo.mMat * vec4(inPos, 1.0);
    fragUV = inUV;
}