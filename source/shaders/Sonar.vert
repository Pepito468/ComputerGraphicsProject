#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 fragPos;

layout (binding = 0, set = 1) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	vec3 diffuse;
	vec4 specular;
} ubo;

layout(binding = 0, set = 2) uniform ShadowMapUniformBufferObject {
	mat4 mvpMat;
} subo;

void main() {
    // get world pos again
    vec4 worldPos = ubo.mMat * vec4(inPos, 1.0);
    fragPos = worldPos.xyz;

    // get mvp pos
    gl_Position = ubo.mvpMat * vec4(inPos, 1.0);
}
