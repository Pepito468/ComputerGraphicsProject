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

// now we need to read the values in the uniforms
// in this shader, we need only the local uniforms
layout (binding = 0, set = 1) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	vec3 diffuse;
	vec4 specular;
} ubo;

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

layout(binding = 0, set = 2) uniform ShadowMapUniformBufferObject {
	mat4 mvpMat;
} subo;


vec3 GerstnerWave(vec3 position, float steepness, float wavelength, float speed, float direction, inout vec3 tangent, inout vec3 binormal)
{
    direction = direction * 2 - 1;
    vec2 d = normalize(vec2(cos(3.14 * direction), sin(3.14 * direction)));
    float k = 2 * 3.14 / wavelength;
    float f = k * (dot(d, position.xz) - speed * gubo.time);
    float a = steepness / k;

    tangent += vec3(
    -d.x * d.x * (steepness * sin(f)),
    d.x * (steepness * cos(f)),
    -d.x * d.y * (steepness * sin(f))
    );

    binormal += vec3(
    -d.x * d.y * (steepness * sin(f)),
    d.y * (steepness * cos(f)),
    -d.y * d.y * (steepness * sin(f))
    );

    return vec3(
    d.x * (a * cos(f)),
    a * sin(f),
    d.y * (a * cos(f))
    );
}

void GerstnerWaves(vec3 position, float steepness, float wavelength, float speed, vec4 directions, out vec3 Offset, out vec3 normal)
{
    Offset = vec3(0.0);
    vec3 tangent = vec3(1, 0, 0);
    vec3 binormal = vec3(0, 0, 1);

    Offset += GerstnerWave(position, steepness, wavelength, speed, directions.x, tangent, binormal);
    Offset += GerstnerWave(position, steepness, wavelength, speed, directions.y, tangent, binormal);
    Offset += GerstnerWave(position, steepness, wavelength, speed, directions.z, tangent, binormal);
    Offset += GerstnerWave(position, steepness, wavelength, speed, directions.w, tangent, binormal);

    normal = normalize(cross(binormal, tangent));
}

void main() {
	  float t = gubo.time;

      vec3 offset, normal;

      GerstnerWaves(inPos, 0.08, 0.1, 0.02, vec4(0.01, 0.0, 0.01, 0.0), offset, normal);

      vec3 outPos = inPos;
      outPos += offset;

      gl_Position = ubo.mvpMat * vec4(outPos, 1.0);
      fragPos = (ubo.mMat * vec4(outPos, 1.0)).xyz;

      fragNorm = normalize(mat3(ubo.nMat) * normal);
      fragUV = inUV;
      fragTan = vec4(normalize(mat3(ubo.mMat) * inTangent.xyz),inTangent.w);
      vec4 shadowPosPrj = subo.mvpMat * ubo.mMat * vec4(inPos, 1.0);
      shadowPos = shadowPosPrj.xyz / shadowPosPrj.w;
}
