#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragNorm;
layout (location = 2) in vec2 fragUV;
layout (location = 3) in vec4 fragTan;
layout (location = 4) in vec3 shadowPos;

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
layout (binding = 2, set = 1) uniform sampler2D ambientOcclusionTex;
layout (binding = 3, set = 1) uniform sampler2D metallicTex;
layout (binding = 4, set = 1) uniform sampler2D normalTex;
layout (binding = 5, set = 1) uniform sampler2D roughnessTex;
layout (binding = 6, set = 1) uniform sampler2D shadowMap;

// and also the global
layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
    // --- Directional Light ---
    vec4 lightDir;          // xyz = direction TOWARDS the light (normalized)
    vec4 lightColor;        // xyz = color * intensity
    vec3 eyePos;


    // --- Hemispheric ambient ---
    vec4 ambientUpper;  // xyz = sky / upper  color  (lU)
    vec4 ambientLower;  // xyz = ground / lower color (lD)
    vec4 ambientDir;    // xyz = "up" direction for blending (d)


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

const float PI = 3.14159265359;

mat3 computeTBN(vec3 N, vec3 T, float tangentW) {
   vec3 n = normalize(N);
   vec3 t = normalize(T);
   t = normalize(t - dot(t,n) * n);
   vec3 b = cross(n, t) * tangentW;

   return mat3(t, b, n);
}

vec3 getNormalFromMap(mat3 TBN) {
    vec3 m = 2*texture(normalTex,fragUV).rgb - vec3(1.0, 1.0, 1.0);

    return normalize(TBN*m);
}

vec3 computeF0(vec3 albedo, float metallic) {
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    return F0;
}

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float dotVH, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - dotVH, 0.0, 1.0), 5.0);
}

// GGX Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2     = roughness * roughness;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = NdotH2 * (a2 - 1.0) + 1.0;
    denom = max(denom, 0.0001);
    return a2 / (PI * denom * denom);
}

// Schlick-GGX single-direction geometry term
float GeometrySchlickGGX(float NdotA, float roughness) {
    float k = (roughness + 1.0);
    k = (k * k) / 8.0;
    return NdotA / (NdotA * (1.0 - k) + k);
}

// Smith geometry function (combines view and light directions)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

vec3 lambertBRDF(vec3 L, vec3 N, vec3 albedo){
    return albedo*max(dot(L,N),0);
}

vec3 blinnBRDF(vec3 L, vec3 N, vec3 V){

    vec3 H = normalize(V + L);

	return ubo.specular.rgb*pow(max(dot(H, N), 0.0), ubo.specular.w);
}

vec3 applyBRDF(vec3 L, vec3 N, vec3 V, vec3 albedo){
    return lambertBRDF(L,N, albedo) + blinnBRDF(L, N, V);
}

vec3 cookTorranceBRDF(vec3 L, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness){
  vec3 H = normalize(V + L);

  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);

    vec3  F0 = computeF0(albedo, metallic);
    vec3  F  = fresnelSchlick(max(dot(V, H), 0.0), F0);
    float D  = DistributionGGX(N, H, roughness);
    float G  = GeometrySmith(N, V, L, roughness);

    vec3 f_diffuse  = albedo * NdotL;
    vec3 f_specular = (D * G * F) / max(4.0 * NdotV, 0.001);

    return (vec3(1.0)-F)*(1-metallic)*f_diffuse + f_specular;
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
	//vec3 N = normalize(fragNorm);

    vec3 albedo = texture(tex, fragUV).rgb;
    float ao = texture(ambientOcclusionTex, fragUV).r;
    float metallic  = texture(metallicTex, fragUV).r;
    float roughness = texture(roughnessTex, fragUV).r;

	vec3 color = vec3(0.0);
	vec3 L, Lc;
	vec3 V = normalize(gubo.eyePos - fragPos);


    // ---- Build TBN and decode the normal map ------
    mat3 TBN = computeTBN(fragNorm, fragTan.xyz, fragTan.w);
    vec3 N   = getNormalFromMap(TBN);

	directLight(L, Lc);

	float lightMapDist = shadowPos.z;
    vec2 lightMapUV = (shadowPos.xy + 1.0f) / 2.0f;

    float shadowBias = max(0.012f * (1.0f - dot(N, L)), 0.002f);
    const float shadowScale = 128.0f;
    vec2 texelSize = 1.0f / vec2(textureSize(shadowMap, 0));
    float notInShadow = 0.0f;

    float depth = texture(shadowMap, lightMapUV).r;
    notInShadow = depth + shadowBias >= lightMapDist ? 1.0f : 0.0f;

    /*
    for(int x = -1; x <= 1; x++) {
    	for(int y = -1; y <= 1; y++) {
    		float depth = texture(shadowMap, lightMapUV + vec2(x, y) * texelSize).r;
    		notInShadow += depth + shadowBias >= lightMapDist ? 1.0f : 0.0f;
    	}

    }
    notInShadow /= 9.0f;*/

    vec3 ambient = hemisphericAmbient(N, albedo) * ao;
	color += ambient;
	color += notInShadow * Lc * cookTorranceBRDF(L, N, V, albedo, metallic, roughness);

    for (int i = 0; i < gubo.pointInstanceCount; i++){

        pointLight(fragPos, L, Lc, i);
        color += Lc * cookTorranceBRDF(L, N, V, albedo, metallic, roughness);
	}

    for (int i = 0; i < gubo.spotInstanceCount; i++){
        spotLight(fragPos, L, Lc, i);
        color += Lc * cookTorranceBRDF(L, N, V, albedo, metallic, roughness);
	}

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

	outColor = vec4(color, 1.0f);
}