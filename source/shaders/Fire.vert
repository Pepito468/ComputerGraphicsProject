#version 450
#extension GL_ARB_separate_shader_objects : enable

// this time, positions and colors are sent by the application
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec4 inTangent;

layout (location = 0) out vec3 fragPos;
layout (location = 1) out vec3 fragNorm;
layout (location = 2) out vec2 fragUV;
layout (location = 3) out vec4 fragTan;
layout (location = 4) out vec3 shadowPos;
layout (location = 5) out vec4 screenPos;

// now we need to read the values in the uniforms
// in this shader, we need only the local uniforms
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
	// now the shader becomes more serious:
	// it computes the normalized screen coordinates with the world-view-projection matrix
	// it computes the world coordinates of the point with the world matrix
	// it computes the normal direction with the corresponding matrix
    gl_Position = ubo.mvpMat * vec4(inPos, 1.0f);
	fragPos     = (ubo.mMat * vec4(inPos, 1.0f)).xyz;
	fragNorm    =  mat3(ubo.nMat) * inNorm;
	fragUV		= inUV;
    fragTan = vec4(normalize(mat3(ubo.mMat) * inTangent.xyz), inTangent.w);
	vec4 shadowPosPrj = subo.mvpMat * ubo.mMat * vec4(inPos, 1.0);
	shadowPos = shadowPosPrj.xyz / shadowPosPrj.w;
    screenPos = ubo.mvpMat * vec4(inPos, 1.0);
}
