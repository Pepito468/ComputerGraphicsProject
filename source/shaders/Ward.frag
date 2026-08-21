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

 layout (binding = 1, set = 1) uniform WardUBO {
 	float alphaT;
 	float alphaB;
 } wubo;

 // and also the global
 layout(binding = 0, set = 0) uniform GlobalUniformBufferObject {
     vec3 lightDir;
     vec3 lightColor;
     vec3 eyePos;
 } gubo;

float saturate(float x) { return clamp(x, 0.0, 1.0); }

 void main() {
 	// returns a color computed with lambert + blinn
 	vec3 N = normalize(fragNorm);
 	vec3 V = normalize(gubo.eyePos - fragPos);
 	vec3 L = gubo.lightDir;

 	// lambert diffuse
 	float kD = max(dot(N, L), 0.0);

    //TODO: check denominators
 	// Ward specular
    // strongly anisotropic (alphaT < alphaB)
    float alphaT = wubo.alphaT;
    float alphaB = wubo.alphaB;
    vec3 dp1 = dFdx(fragPos);
    vec3 t   = normalize(dp1 - dot(dp1, N) * N);
    vec3 b   = normalize(cross(N, t));

    vec3 h = normalize(L + V);
    float dotHT = saturate(dot(h,t));
    float dotHB = saturate(dot(h,b));
    float dotHN = saturate(dot(h,N));

    float X = exp(-(pow(dotHT/alphaT,2) + pow(dotHB/alphaB,2))/pow(dotHN, 2));

    float dotVN = saturate(dot(V, N));
    float dotLN = saturate(dot(L, N));

    float Y = 4*3.14*alphaT*alphaB*sqrt(dotVN/dotLN);

    float kS = X/Y;

 	// final color
 	vec3 color = (kD * ubo.diffuse + kS * ubo.specular.rgb) * gubo.lightColor;

 	outColor = vec4(color, 1.0f);
 }

