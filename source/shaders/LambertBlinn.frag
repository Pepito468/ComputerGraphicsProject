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
} ubo;

// and also the global
layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    vec4 lightDir;
    vec4 lightColor;
    vec3 eyePos;

    // --- Hemispheric ambient ---
    vec4 ambientUpper;  // xyz = sky / upper  color  (lU)
    vec4 ambientLower;  // xyz = ground / lower color (lD)
    vec4 ambientDir;    // xyz = "up" direction for blending (d)
} gubo;

vec3 hemisphericAmbient(vec3 N, vec3 mA) {
    vec3 lU = gubo.ambientUpper.xyz;
    vec3 lD = gubo.ambientLower.xyz;
    vec3 d  = normalize(gubo.ambientDir.xyz);
    float dotNd = dot(N, d);

    vec3 lA = ((dotNd + 1.0)/2.0)*lU + ((1.0 - dotNd)/2.0)*lD;

    return lA * mA;
}

void main() {
	// returns a color computed with lambert + blinn
	vec3 N = normalize(fragNorm);
	vec3 V = normalize(gubo.eyePos - fragPos);
	vec3 L = gubo.lightDir.xyz;

	// lambert diffuse
	float kD = max(dot(N, L), 0.0);

	// blinn specular
	vec3 H = normalize(V + L);
	float kS = pow(max(dot(H, N), 0.0), ubo.specular.w);



	// final color
	vec3 color = (kD * ubo.diffuse + kS * ubo.specular.rgb) * gubo.lightColor.rgb;

	vec3 ambient = hemisphericAmbient(N, ubo.diffuse);
    color += ambient;
	
	outColor = vec4(color, 1.0f);
}