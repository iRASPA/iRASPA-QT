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

// Per-vertex:
layout(location = 0) in vec3 inPosition;

// Per-instance:
//layout(location = 1) in mat4 inTransform;
// workaround for mac
layout(location = 1) in vec4 inTransform0;
layout(location = 2) in vec4 inTransform1;
layout(location = 3) in vec4 inTransform2;
layout(location = 4) in vec4 inTransform3;

layout(location = 5) in vec4 inColor;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 pos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 inTransform = mat4(inTransform0, inTransform1, inTransform2, inTransform3);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    vec4 pos4 = vec4(inPosition, 1);
    pos = vec3(pos4) / pos4.w;
}
