#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragNorm;
layout (location = 2) in vec2 fragUV;

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
    // --- Directional Light ---
        vec4 lightDir;          // xyz = direction TOWARDS the light (normalized)
        vec4 lightColor;        // xyz = color * intensity
        vec4 eyePos;

        // --- Point Light ---
        vec4 pointLightPos[8];     // xyz = world position
        vec4 pointLightColor[8];   // xyz = color * intensity
        vec4 pointLightParams[8];  // x = beta (decay exponent), y = g (target distance)
        int pointInstanceCount;

        // --- Spotlight ---
        vec4 spotLightPos[8];      // xyz = world position
        vec4 spotLightDir[8];      // xyz = cone direction (normalized)
        vec4 spotLightColor[8];    // xyz = color * intensity
        vec4 spotLightParams[8];   // x = cIN = cos(alpha_IN/2), y = cOUT = cos(alpha_OUT/2)
        int  spotInstanceCount;
} gubo;


void directLight(out vec3 direction, out vec3 color) {
    direction = gubo.lightDir.xyz;
    color     = gubo.lightColor.rgb;
}

void pointLight(vec3 pos, out vec3 direction, out vec3 color, int i) {
    vec3  delta = gubo.pointLightPos[i].xyz - pos;
    float dist  = length(delta);
    float beta  = gubo.pointLightParams[i].x;
    float g     = gubo.pointLightParams[i].y;

    direction = delta/dist;
    color     = gubo.pointLightColor[i].rgb * pow(g/dist, beta);
}

void spotLight(vec3 pos, out vec3 direction, out vec3 color, int i) {
    vec3  delta = gubo.spotLightPos[i].xyz - pos;
    float dist  = length(delta);
    float cIN   = gubo.spotLightParams[i].x;
    float cOUT  = gubo.spotLightParams[i].y;


    direction = delta/dist;

    vec3 l = gubo.spotLightColor[i].rgb;
    float cosAlpha = dot(direction,normalize(gubo.spotLightDir[i].xyz));
    float dimming = clamp((cosAlpha-cOUT)/(cIN-cOUT), 0.0, 1.0);

    color     = l*dimming;
}

vec3 lambertBRDF(vec3 L, vec3 N){
    float tD = ubo.param1.x;
    float mD1 = ubo.param1.y;
    float mD2 = ubo.param1.z;


	float kD = max(dot(L,N),0.0) >= tD ? mD1 : mD2;
	return kD * ubo.diffuse.rgb;
}

vec3 specularBRDF(vec3 L, vec3 N, vec3 V){

    float tS = ubo.param2.x;
    float mS1 = ubo.param2.y;
    float mS2 = ubo.param2.z;


	vec3 R = 2*N*dot(L,N) - L;
	float kS = max(dot(V, R), 0.0) >= tS ? mS1 : mS2;

	return kS*ubo.specular.rgb;
}

vec3 applyBRDF(vec3 L, vec3 N, vec3 V){
    return lambertBRDF(L, N) + specularBRDF(L, N, V);
}

void main() {
	// returns a color computed with lambert + blinn
	vec3 N = normalize(fragNorm);
	vec3 V = normalize(gubo.eyePos.xyz - fragPos);

	vec3 color = vec3(0.0);
    vec3 L, Lc;

    directLight(L, Lc);

    color += Lc * applyBRDF(L, N, V);

    for (int i = 0; i < gubo.pointInstanceCount; i++){
        pointLight(fragPos, L, Lc, i);
        color += Lc * applyBRDF(L, N, V);
    }

    for (int i = 0; i < gubo.spotInstanceCount; i++){
        spotLight(fragPos, L, Lc, i);
        color += Lc * applyBRDF(L, N, V);
    }

	outColor = vec4(color, 1.0f);
}