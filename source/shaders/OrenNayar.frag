#version 450
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.14159265359

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
} ubo;

layout (binding = 1, set = 1) uniform OrenNayarUBO{
    float sigma;
} nubo;

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

    //values between 0 and PI/2
    //sigma = 0 means lambert diffuse
	float sigma = nubo.sigma;

	// OrenNayar diffuse
	float theata_i = acos(dot(L,N));
    float theata_r = acos(dot(V,N));

    float alpha = max(theata_i, theata_r);
    float beta = min (theata_i, theata_r);

    float s2 = sigma*sigma;

    float A = 1 - 0.5*(s2 /(s2 + 0.33));
    float B = 0.45*(s2 /(s2 + 0.09));

    vec3 vi = normalize(L - N*dot(L, N));
    vec3 vr = normalize(V - N*dot(V, N));

    float G = max(0.0, dot(vi, vr));

    float kD = clamp(dot(L,N),0.0,1.0) * ( A + B * G * sin(alpha) * tan(beta));

	// blinn specular
	vec3 H = normalize(V + L);
	float kS = pow(max(dot(H, N), 0.0), ubo.specular.w);

	// final color
	vec3 color = (kD * ubo.diffuse + kS * ubo.specular.rgb) * gubo.lightColor;

	outColor = vec4(color, 1.0f);
}
