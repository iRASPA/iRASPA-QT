#pragma once

#include <string>

class VulkanUniformStringLiterals
{
public:
  inline static const std::string VersionStringLiteral = R"foo(
  #version 450
  )foo";

  inline static const std::string FrameUniformBlockStringLiteral = R"foo(
  layout(std140, set = 0, binding = 0) uniform FrameUniformBlock
  {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 mvpMatrix;
    mat4 shadowMatrix;
    mat4 projectionMatrixInverse;
    mat4 viewMatrixInverse;
    mat4 normalMatrix;
    mat4 axesProjectionMatrix;
    mat4 axesViewMatrix;
    mat4 axesMvpMatrix;
    mat4 padMatrix;
    vec4 cameraPosition;
    vec4 padvector4;
    float numberOfMultiSamplePoints;
    float padInt1;
    float padInt2;
    float padInt3;
    float bloomLevel;
    float bloomPulse;
    float padFloat1;
    float padFloat2;
  } frameUniforms;
  )foo";
};
