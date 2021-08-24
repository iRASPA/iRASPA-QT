#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 mvpMatrix;
    mat4 shadowMatrix;
    mat4 projectionMatrixInverse;
    mat4 viewMatrixInverse;
    mat4 normalMatrix;

    // moved 'numberOfMultiSamplePoints' to here (for downsampling when no structures are present)
    float numberOfMultiSamplePoints;
    float intPad1;
    float intPad2;
    float intPad3;
    float bloomLevel;
    float bloomPulse;
    float padFloat1;
    float padFloat2;
    vec4 padVector3;
    vec4 padvector4;
} ubo;

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 pos;

layout(location = 0) out vec4 outColor;

float saturate(float x) 			{ return clamp(x, 0.0, 1.0); }

void main() 
{
    vec3 N = normalize(cross(dFdy(pos), dFdx(pos)));
    vec3 V = normalize(pos);
    vec3 H = normalize(V);

    float n_dot_h = saturate( dot(N, H) );

    float diffuse = 0.2;
    float spec = pow(n_dot_h, 8);
    
    outColor.rgb = fragColor.rgb * (diffuse + 0.5) * 0.5 + vec3(0.1, 0.1, 0.1) * spec;

    // gamma correction
    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));
    outColor.a = fragColor.a;
}
