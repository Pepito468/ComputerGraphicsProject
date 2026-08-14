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
} ubo;

// the shaders now receives also the texture in a separate set
layout (binding = 1, set = 1) uniform sampler2D tex;

// and also the global
layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    // --- Directional Light ---
    vec4 lightDir;          // xyz = direction TOWARDS the light (normalized)
    vec4 lightColor;        // xyz = color * intensity
    vec3 eyePos;

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

vec3 lambertBRDF(vec3 L, vec3 N, vec3 diffuseColor){
    return diffuseColor*max(dot(L,N),0);
}

vec3 blinnBRDF(vec3 L, vec3 N, vec3 V){

    vec3 H = normalize(V + L);

	return ubo.specular.rgb*pow(max(dot(H, N), 0.0), ubo.specular.w);
}

vec3 applyBRDF(vec3 L, vec3 N, vec3 V, vec3 diffuseColor){
    return lambertBRDF(L,N, diffuseColor) + blinnBRDF(L, N, V);
}

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


void main() {
	// returns a color computed with lambert + blinn
	vec3 N = normalize(fragNorm);
	vec3 V = normalize(gubo.eyePos - fragPos);

	vec3 color = vec3(0.0);
	vec3 L, Lc;

    vec3 diffuseColor = texture(tex, fragUV).rgb;

	directLight(L, Lc);

	color += Lc * applyBRDF(L, N, V, diffuseColor);

    for (int i = 0; i < gubo.pointInstanceCount; i++){

        pointLight(fragPos, L, Lc, i);
        color += Lc * applyBRDF(L, N, V, diffuseColor);
	}

    for (int i = 0; i < gubo.spotInstanceCount; i++){
        spotLight(fragPos, L, Lc, i);
        color += Lc * applyBRDF(L, N, V, diffuseColor);
	}

	outColor = vec4(color, 1.0f);
}