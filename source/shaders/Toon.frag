#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragNorm;

// now we need to read the values in the uniforms
// in this shader, we need the local uniforms
layout (binding = 0, set = 1) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	vec3 diffuse;
	vec4 specular;
	vec3 param1;
	vec3 param2;
} ubo;

// and also the global
layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    vec3 lightDir;
    vec3 lightColor;
    vec3 eyePos;
} gubo;



void main() {
	// returns a color computed with lambert + blinn
	vec3 N = normalize(fragNorm);
	vec3 V = normalize(gubo.eyePos - fragPos);
	vec3 L = gubo.lightDir;


    float tD = ubo.param1.x;
	float mD1 = ubo.param1.y;
    float mD2 = ubo.param1.z;

    float tS = ubo.param2.x;
    float mS1 = ubo.param2.y;
    float mS2 = ubo.param2.z;


	// lambert diffuse
	float kD = max(dot(L,N),0.0) >= tD ? mD1 : mD2;

	// blinn specular
	vec3 R = 2*N*dot(L,N) - L;
	float kS = max(dot(V, R), 0.0) >= tS ? mS1 : mS2;

	// final color
	vec3 color = (kD * ubo.diffuse + kS * ubo.specular.rgb) * gubo.lightColor;

	outColor = vec4(color, 1.0f);
}