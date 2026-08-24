#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform sceneDepthUBO {
    mat4 mvpMat;
} sdubo;

layout(set = 1, binding = 0) uniform UBO {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    vec3 diffuse;
    vec4 specular;
} ubo;



void main()
{
    gl_Position = sdubo.mvpMat * ubo.mMat * vec4(inPosition, 1.0);
}