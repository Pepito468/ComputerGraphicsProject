#version 450
#extension GL_ARB_separate_shader_objects : enable

// the default render pass has just one attchment of type vec4, representing the pixel on screen
layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 fragNorm;
layout (location = 2) in vec2 fragUV;
layout (location = 3) in vec4 fragTan;
layout (location = 4) in vec3 shadowPos;
layout (location = 5) in vec4 screenPos;


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
layout (binding = 2, set = 1) uniform sampler2D armTex;
layout (binding = 3, set = 1) uniform sampler2D normalTex;
layout (binding = 4, set = 1) uniform sampler2D shadowMap;
layout (binding = 5, set = 1) uniform sampler2D sceneDepth;
layout (binding = 6, set = 1) uniform sampler2D sceneColor;

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

    float time;
} gubo;


float fresnelSchlick(float dotVH) {
    float F0 = 0.02; //water value
    return F0 + (1.0 - F0) * pow(clamp(1.0 - dotVH, 0.0, 1.0), 5.0);
}

mat3 computeTBN(vec3 N, vec3 T, float tangentW) {
   vec3 n = normalize(N);
   vec3 t = normalize(T);
   t = normalize(t - dot(t,n) * n);
   vec3 b = cross(n, t) * tangentW;

   return mat3(t, b, n);
}

vec3 getTangentNormal(vec2 UV) {
    return texture(normalTex, UV).rgb * 2.0 - 1.0;
}

vec2 panningUV(vec2 uv,vec2 tiling,vec2 direction,float speed,vec2 offset,float time)
{
    return uv * tiling + direction * speed * time + offset;
}

vec3 blendedNormals(float normalSpeed, float normalScale, float normalStrength){
    vec2 panningUV1 = fragUV * (1.0/(0.5*normalScale)) + gubo.time * normalSpeed*(-0.5);
    vec2 panningUV2 = fragUV * (1.0/normalScale) + gubo.time * normalSpeed;

    vec3 n1 = getTangentNormal(panningUV1);
    vec3 n2 = getTangentNormal(panningUV2);

    //blending normals
    vec3 blend = normalize(vec3(n1.xy + n2.xy, n1.z * n2.z));
    blend *= normalStrength;
    return normalize(blend);
}

float blinn(vec3 L, vec3 N, vec3 V, float smoothness){
    vec3 H = normalize(V + L);
	return pow(max(dot(H, N), 0.0), smoothness);
}

float linearizeDepth(float depth)
{
    //TODO: should always be coherent with the current camera
    const float nearPlane = 0.01;
    const float farPlane = 100.0;

    return (nearPlane * farPlane) /
           (farPlane - depth * (farPlane - nearPlane));
}

void pointLight(vec3 pos, out vec3 direction, out vec3 color, int i) {
    vec3  delta = gubo.pointLightPos[i].xyz - pos;
    float dist  = length(delta);
    float beta  = gubo.pointLightParams[i].x;
    float g     = gubo.pointLightParams[i].y;

    direction = delta/dist;
    color     = gubo.pointLightColor[i].rgb * pow(g/dist, beta);
}


//from: https://ameye.dev/notes/stylized-water-shader/
void main()
{
    float t = gubo.time;

    //COLORS
    vec3 waterColor    = vec3(0.004, 0.018, 0.028);
    vec3 horizonColor  = vec3(0.07, 0.22, 0.30);
    vec3 specularColor = vec3(1.00, 0.96, 0.82);
    vec3 shallowColor  = vec3(0.5, 1, 1);
    vec3 deepColor     = vec3(0.015, 0.055, 0.075);
    vec3 foamColor     = vec3(0.82, 0.94, 0.92);

    //Lighting params
    float smoothness = 200;
    float lightingHardness = 0.35;
    float normalSpeed = 0.05;
    float normalScale = 0.1;
    float normalStrength = 0.5;

    //Foam params
    float intersectionFoamDepth = 0.1;
    float intersectionFoamCutoff = 0.3;
    vec2 intersectionFoamTiling = vec2(80);
    vec2 intersectionFoamDirection = vec2(1.0, 0.15);
    float intersectionFoamSpeed = 0.03;

    //Refraction params
    float refractionStrength = 0.005;

    //Computed params
	vec3 V = normalize(gubo.eyePos - fragPos);
	vec3 L = normalize(gubo.lightDir.xyz);
	mat3 TBN = computeTBN(fragNorm, fragTan.xyz, fragTan.w);
    vec3 tangentNormal = blendedNormals(normalSpeed, normalScale, normalStrength);
    vec3 N = normalize(TBN * tangentNormal);

    // Water Depth
    vec2 screenUV = screenPos.xy / screenPos.w;
    screenUV = screenUV * 0.5 + 0.5;
    float sceneDepthValue = texture(sceneDepth, screenUV).r;
    float waterDepth = linearizeDepth(sceneDepthValue) - screenPos.w;

    //Fading
    float fadeDistance = 1.5;
    float fade = clamp(waterDepth / fadeDistance, 0.0, 1.0);
    vec3 baseColor = mix(shallowColor, deepColor, fade);

    //Refraction
    vec2 refractedUV = screenUV + N.xz * refractionStrength;
    vec3 sceneColorValue = texture(sceneColor, refractedUV).rgb;
    vec3 underwaterColor = sceneColorValue * baseColor;

    // Lighting
    vec3 H = normalize(V + L);
    float dotNH = max(dot(N, H), 0.0);
    float fresnel = fresnelSchlick(dotNH);
	vec3 color = mix(underwaterColor, horizonColor, fresnel);

    //Direct Light
    float specular = blinn(L, N, V, smoothness);
    vec3 specular_ = mix(specular, step(0.5, specular), lightingHardness) * specularColor;
    float dotNL = max(dot(N,L), 0.0);
    vec3 directLight = gubo.lightColor.rgb * dotNL;
    color *= directLight;
    color += specular_;

    //Point Lights
    vec3 Lc;
    for (int i = 0; i < gubo.pointInstanceCount; i++){
        pointLight(fragPos, L, Lc, i);
        float specularSoft = blinn(L, N, V, smoothness*1.5);
        float specularHard = smoothstep (0.005,0.01, specularSoft);
        float specularTerm = mix(specularSoft, specularHard, lightingHardness);
        dotNL = max(dot(N,L), 0.0);
        color += Lc * (dotNL + specularTerm);
    }

    //Ambient Light
    vec3 ambient = mix(gubo.ambientLower.rgb,gubo.ambientUpper.rgb,max(dot(N, normalize(gubo.ambientDir.xyz)), 0.0));
    color += ambient * 0.02;

    //Foam
    float foam = 1.0 - smoothstep(0.0, intersectionFoamDepth, waterDepth);
    vec2 foamUV = panningUV(fragUV, intersectionFoamTiling, intersectionFoamDirection, intersectionFoamSpeed, vec2(0.0), gubo.time);
    float foamTex = texture(armTex, foamUV).r;
    float foamPattern = step(intersectionFoamCutoff, foamTex);
    foam *= foamPattern;
    color = mix(color, foamColor, foam);

    //Gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    outColor = vec4(color, 1.0);


}