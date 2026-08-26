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
	vec4 param1;
	vec4 param2;
	vec4 param3;
	vec4 param4;
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



//from: https://github.com/MaxBittker/glsl-voronoi-noise
const mat2 myt = mat2(.12121212, .13131313, -.13131313, .12121212);
const vec2 mys = vec2(1e4, 1e6);

vec2 rhash(vec2 uv) {
  uv *= myt;
  uv *= mys;
  return fract(fract(uv / mys) * uv);
}

vec3 hash(vec3 p) {
  return fract(
      sin(vec3(dot(p, vec3(1.0, 57.0, 113.0)), dot(p, vec3(57.0, 113.0, 1.0)),
               dot(p, vec3(113.0, 1.0, 57.0)))) *
      43758.5453);
}

vec3 voronoi3d(const in vec3 x) {
  vec3 p = floor(x);
  vec3 f = fract(x);

  float id = 0.0;
  vec2 res = vec2(100.0);
  for (int k = -1; k <= 1; k++) {
    for (int j = -1; j <= 1; j++) {
      for (int i = -1; i <= 1; i++) {
        vec3 b = vec3(float(i), float(j), float(k));
        vec3 r = vec3(b) - f + hash(p + b);
        float d = dot(r, r);

        float cond = max(sign(res.x - d), 0.0);
        float nCond = 1.0 - cond;

        float cond2 = nCond * max(sign(res.y - d), 0.0);
        float nCond2 = 1.0 - cond2;

        id = (dot(p + b, vec3(1.0, 57.0, 113.0)) * cond) + (id * nCond);
        res = vec2(d, res.x) * cond + res * nCond;

        res.y = cond2 * d + nCond2 * res.y;
      }
    }
  }

  return vec3(sqrt(res), abs(id));
}


//from: https://www.shadertoy.com/view/XdXGW8
vec2 grad( ivec2 z )  // replace this anything that returns a random vector
{
    // 2D to 1D  (feel free to replace by some other)
    int n = z.x+z.y*11111;

    // Hugo Elias hash (feel free to replace by another one)
    n = (n<<13)^n;
    n = (n*(n*n*15731+789221)+1376312589)>>16;

#if 0

    // simple random vectors
    return vec2(cos(float(n)),sin(float(n)));

#else

    // Perlin style vectors
    n &= 7;
    vec2 gr = vec2(n&1,n>>1)*2.0-1.0;
    return ( n>=6 ) ? vec2(0.0,gr.x) :
           ( n>=4 ) ? vec2(gr.x,0.0) :
                              gr;
#endif
}

float noise( in vec2 p )
{
    ivec2 i = ivec2(floor( p ));
     vec2 f =       fract( p );

	vec2 u = f*f*(3.0-2.0*f); // feel free to replace by a quintic smoothstep instead

    return mix( mix( dot( grad( i+ivec2(0,0) ), f-vec2(0.0,0.0) ),
                     dot( grad( i+ivec2(1,0) ), f-vec2(1.0,0.0) ), u.x),
                mix( dot( grad( i+ivec2(0,1) ), f-vec2(0.0,1.0) ),
                     dot( grad( i+ivec2(1,1) ), f-vec2(1.0,1.0) ), u.x), u.y);
}

/*
small flame
 vec4 firstColor = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 secondColor = vec4(0.9, 0.9, 0.0, 1.0);
    float distortionAmount = 0.1;
    float distortionScale = 15;
    vec2 distortionSpeed = vec2(1,1);
    float dissolveAmount = 1.2;
    float dissolveScale = 5;
    float dissolveSpeed = 3;
*/


void main()
{
    float t = gubo.time;

    vec4 firstColor = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 secondColor = vec4(0.9, 0.9, 0.0, 1.0);
    float distortionAmount = 0.05;
    float distortionScale = 20;
    vec2 distortionSpeed = vec2(1,1);
    float dissolveAmount = 1.2;
    float dissolveScale = 5;
    float dissolveSpeed = 3;
    float intensity = 5;

    vec2 gradientNoiseUV = fragUV*dissolveScale + distortionSpeed*t;
    float gradientNoise = noise(gradientNoiseUV);

    vec2 voronoiUV = fragUV*dissolveScale;
    float voronoiNoise = pow(voronoi3d(vec3(voronoiUV, t*dissolveSpeed)).r, distortionAmount);
    voronoiNoise = mix(voronoiNoise, gradientNoise, 0.6);


    vec4 albedo = texture(tex, mix(fragUV, vec2(gradientNoise,gradientNoise), distortionAmount));
    albedo *= albedo*voronoiNoise*intensity;

    vec4 finalColor = mix(firstColor, secondColor, albedo.a);

    outColor = vec4(finalColor.rgb, albedo.a);
}