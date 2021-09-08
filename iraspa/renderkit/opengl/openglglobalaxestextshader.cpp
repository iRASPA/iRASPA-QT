/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#include "openglglobalaxestextshader.h"
#include <QDebug>
#include <iostream>
#include "glgeterror.h"
#include "spheregeometry.h"
#include "cylindergeometry.h"
#include "glgeterror.h"
#include "opengltextrenderingshader.h"

#if defined (Q_OS_OSX)
  const QString OpenGLGlobalAxesTextShader::fontString = QString("Helvetica");
#elif defined(Q_OS_WIN)
  const QString OpenGLGlobalAxesTextShader::fontString = QString("Segoe UI");
#elif defined(Q_OS_LINUX)
  const QString OpenGLGlobalAxesTextShader::fontString = QString("Helvetica");
#endif

OpenGLGlobalAxesTextShader::OpenGLGlobalAxesTextShader()
{

}

OpenGLGlobalAxesTextShader::~OpenGLGlobalAxesTextShader()
{
}

void OpenGLGlobalAxesTextShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = source;
}


void OpenGLGlobalAxesTextShader::paintGL()
{
  if(_dataSource)
  {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(_program);
    check_gl_error();

    glBindVertexArray(_vertexArrayObject);
    check_gl_error();

    if(!OpenGLTextRenderingShader::_cachedFontAtlas.contains(fontString))
    {
      GLuint texture;
      glGenTextures(1, &texture);
      check_gl_error();

      glBindTexture(GL_TEXTURE_2D, texture);
      check_gl_error();
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      RKFontAtlas* fontAtlas = new RKFontAtlas(fontString, 256);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, fontAtlas->textureData.data());
      check_gl_error();
      fontAtlas->texture = texture;

      OpenGLTextRenderingShader::_cachedFontAtlas.insert(fontString, fontAtlas);
    }
    RKFontAtlas *atlas = OpenGLTextRenderingShader::_cachedFontAtlas.object(fontString);

    glActiveTexture(GL_TEXTURE0);
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D, atlas->texture);
    check_gl_error();
    glUniform1i(_fontAtlasTextureUniformLocation,0);
    check_gl_error();

    glDrawArraysInstanced(GL_POINTS, 0, 1, (GLsizei)3);
    check_gl_error();
    glBindVertexArray(0);

    glUseProgram(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
  }
}

void OpenGLGlobalAxesTextShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLGlobalAxesTextShader::initializeVertexArrayObject()
{
  if(_dataSource)
  {
    std::vector<float4> coords{float4(0.0, 0.0, 0.0, 1.0)};

    glBindVertexArray(_vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    check_gl_error();
    glBufferData(GL_ARRAY_BUFFER, coords.size()*sizeof(float4), coords.data(), GL_DYNAMIC_DRAW);
    check_gl_error();

    if(!OpenGLTextRenderingShader::_cachedFontAtlas.contains(fontString))
    {
      GLuint texture;
      glGenTextures(1, &texture);
      check_gl_error();

      glBindTexture(GL_TEXTURE_2D, texture);
      check_gl_error();
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      RKFontAtlas* fontAtlas = new RKFontAtlas(fontString, 256);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, fontAtlas->textureData.data());
      check_gl_error();
      fontAtlas->texture = texture;

      OpenGLTextRenderingShader::_cachedFontAtlas.insert(fontString, fontAtlas);
    }
    RKFontAtlas *atlas = OpenGLTextRenderingShader::_cachedFontAtlas.object(fontString);

    std::vector<RKInPerInstanceAttributesText> atomTextDataX = atlas->buildMeshWithString(float4(1.0,0.0,0.0,1.0), float4(3,3,3,3), "X", RKTextAlignment::center);
    std::vector<RKInPerInstanceAttributesText> atomTextDataY = atlas->buildMeshWithString(float4(0.0,1.0,0.0,1.0), float4(3,3,3,3), "Y", RKTextAlignment::center);
    std::vector<RKInPerInstanceAttributesText> atomTextDataZ = atlas->buildMeshWithString(float4(0.0,0.0,1.0,1.0), float4(3,3,3,3), "Z", RKTextAlignment::center);

    std::vector<RKInPerInstanceAttributesText> atomTextData;
    std::copy(atomTextDataX.begin(), atomTextDataX.end(), std::back_inserter(atomTextData));
    std::copy(atomTextDataY.begin(), atomTextDataY.end(), std::back_inserter(atomTextData));
    std::copy(atomTextDataZ.begin(), atomTextDataZ.end(), std::back_inserter(atomTextData));

    glBindBuffer(GL_ARRAY_BUFFER,_instanceBuffer);
    check_gl_error();
    if(atomTextData.size()>0)
    {
      glBufferData(GL_ARRAY_BUFFER, atomTextData.size()*sizeof(RKInPerInstanceAttributesText), atomTextData.data(), GL_DYNAMIC_DRAW);
    }

    glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesText), (GLvoid *)offsetof(RKInPerInstanceAttributesText,position));
    check_gl_error();
    glVertexAttribDivisor(_instancePositionAttributeLocation, 1);
    check_gl_error();

    glVertexAttribPointer(_instanceScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesText), (GLvoid *)offsetof(RKInPerInstanceAttributesText,scale));
    check_gl_error();
    glVertexAttribDivisor(_instanceScaleAttributeLocation, 1);
    check_gl_error();

    glVertexAttribPointer(_instanceVertexAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesText), (GLvoid *)offsetof(RKInPerInstanceAttributesText,vertexCoordinatesData));
    check_gl_error();
    glVertexAttribDivisor(_instanceVertexAttributeLocation, 1);
    check_gl_error();

    glVertexAttribPointer(_instanceTextureUVAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesText), (GLvoid *)offsetof(RKInPerInstanceAttributesText,textureCoordinatesData));
    check_gl_error();
    glVertexAttribDivisor(_instanceTextureUVAttributeLocation, 1);
    check_gl_error();

    glEnableVertexAttribArray(_instancePositionAttributeLocation);
    glEnableVertexAttribArray(_instanceScaleAttributeLocation);
    glEnableVertexAttribArray(_instanceVertexAttributeLocation);
    glEnableVertexAttribArray(_instanceTextureUVAttributeLocation);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
}

void OpenGLGlobalAxesTextShader::generateBuffers()
{
  glGenVertexArrays(1, &_vertexArrayObject);
  glGenBuffers(1, &_vertexBuffer);
  glGenBuffers(1, &_instanceBuffer);
}

void OpenGLGlobalAxesTextShader::deleteBuffers()
{

}

void OpenGLGlobalAxesTextShader::generateTextures()
{

}

void OpenGLGlobalAxesTextShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint geometryShader;
  GLuint fragmentShader;

  qDebug() << "OpenGLGlobalAxesTextShader::loadShader";

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLGlobalAxesTextShader::_vertexShaderSource.c_str());
  geometryShader=compileShaderOfType(GL_GEOMETRY_SHADER,OpenGLGlobalAxesTextShader::_geometryShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLGlobalAxesTextShader::_fragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _program = glCreateProgram();
    check_gl_error();

    glAttachShader(_program, vertexShader);
    check_gl_error();
    glAttachShader(_program, geometryShader);
    check_gl_error();
    glAttachShader(_program, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_program, 0, "vFragColor");

    linkProgram(_program);

    _instancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");
    _instanceVertexAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    _instanceScaleAttributeLocation = glGetAttribLocation(_program, "instanceScale");
    _instanceTextureUVAttributeLocation = glGetAttribLocation(_program, "instanceTexCoords");
    _fontAtlasTextureUniformLocation   = glGetUniformLocation(_program, "fontAtlasTexture");


    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'instancePosition' attribute.";
    if (_instanceVertexAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";
    if (_instanceScaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceScale' attribute.";
    if (_instanceTextureUVAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceTexCoords' attribute.";
    if (_fontAtlasTextureUniformLocation < 0) qDebug() << "Shader did not contain the 'fontAtlasTexture' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}


const std::string  OpenGLGlobalAxesTextShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLGlobalAxesUniformBlockStringLiteral +
R"foo(

in vec4 instancePosition;
in vec4 instanceScale;
in vec4 vertexPosition;
in vec4 instanceTexCoords;

out VertexData
{
  flat vec4 eye_position;
  flat vec4 sphere_radius;
  flat vec4 vertexPosition;
  flat vec4 texcoords;
  flat int instanceID;
} vs_out;

void main(void)
{
  vs_out.sphere_radius =  globalAxesUniforms.textScale[gl_InstanceID] * instanceScale;
  vs_out.vertexPosition =  instanceScale * vertexPosition;
  vs_out.texcoords = instanceTexCoords;
  vec4 textPosition = instancePosition;

  float scale = globalAxesUniforms.axesScale  + 2.0*globalAxesUniforms.centerScale + globalAxesUniforms.textOffset + globalAxesUniforms.textScale[gl_InstanceID];
  textPosition.xyz *= scale;
  vec4 pos = frameUniforms.axesViewMatrix * textPosition;
  vs_out.eye_position = pos;
  gl_Position = vec4(pos.xyz,1.0);
  vs_out.instanceID = gl_InstanceID;
}
)foo";

const std::string  OpenGLGlobalAxesTextShader::_geometryShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLGlobalAxesUniformBlockStringLiteral +
R"foo(
layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in VertexData
{
  flat vec4 eye_position;
  flat vec4 sphere_radius;
  flat vec4 vertexPosition;
  flat vec4 texcoords;
  flat int instanceID;
} vs_in[];

out GS_OUT
{
  smooth vec4 eye_position;
  smooth vec2 texcoords;
  flat vec4 sphere_radius;
  flat int instanceID;
} gs_out;

void main(void)
{
  vec4 pos = gl_in[0].gl_Position;
  gs_out.eye_position = vs_in[0].eye_position;
  gs_out.sphere_radius = vs_in[0].sphere_radius;
  vec4 c = globalAxesUniforms.textScale[vs_in[0].instanceID] *  vs_in[0].vertexPosition;
  vec4 d = vs_in[0].texcoords;
  gs_out.instanceID = vs_in[0].instanceID;


  vec4 pos1 = pos;
  pos1.xy += vec2(c.x, -c.y);
  pos1.xy += globalAxesUniforms.textDisplacement[vs_in[0].instanceID].xy;
  gl_Position = frameUniforms.axesProjectionMatrix * pos1;
  gs_out.texcoords = vec2(d.x, d.y);
  EmitVertex();

  vec4 pos2 = pos;
  pos2.xy += vec2(c.x, -c.y -c.w);
  pos2.xy += globalAxesUniforms.textDisplacement[vs_in[0].instanceID].xy;
  gl_Position = frameUniforms.axesProjectionMatrix * pos2;
  gs_out.texcoords = vec2(d.x, d.y + d.w);
  EmitVertex();

  vec4 pos3 = pos;
  pos3.xy += vec2(c.x + c.z,-c.y);
  pos3.xy += globalAxesUniforms.textDisplacement[vs_in[0].instanceID].xy;
  gl_Position = frameUniforms.axesProjectionMatrix * pos3;
  gs_out.texcoords = vec2(d.x + d.z, d.y);
  EmitVertex();

  vec4 pos4 = pos;
  pos4.xy += vec2(c.x + c.z, -c.y -c.w);
  pos4.xy += globalAxesUniforms.textDisplacement[vs_in[0].instanceID].xy;
  gl_Position = frameUniforms.axesProjectionMatrix * pos4;
  gs_out.texcoords = vec2(d.x + d.z, d.y + d.w);
  EmitVertex();

  EndPrimitive();
}
 )foo";

const std::string  OpenGLGlobalAxesTextShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLGlobalAxesUniformBlockStringLiteral +
R"foo(
// Input from vertex shader
in GS_OUT
{
  smooth vec4 eye_position;
  smooth vec2 texcoords;
  flat vec4 sphere_radius;
  flat int instanceID;
} fs_in;

out  vec4 vFragColor;
uniform sampler2D fontAtlasTexture;

void main(void)
{
  vec4 pos = fs_in.eye_position;
  pos.z += fs_in.sphere_radius.z + globalAxesUniforms.textDisplacement[fs_in.instanceID].z;
  pos = frameUniforms.axesProjectionMatrix * pos;
  gl_FragDepth = 0.5 * (pos.z / pos.w) + 0.5;

  vec4 color = globalAxesUniforms.textColor[fs_in.instanceID];

  // Outline of glyph is the isocontour with value 50%
  float edgeDistance = 0.5;

  // Sample the signed-distance field to find distance from this fragment to the glyph outline
  float sampleDistance = texture(fontAtlasTexture, fs_in.texcoords).r;

  // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
  float edgeWidth = length(vec2(dFdx(sampleDistance), dFdy(sampleDistance)));

  // Smooth the glyph edge by interpolating across the boundary in a band with the width determined above
  float insideness = smoothstep(edgeDistance - edgeWidth, edgeDistance + edgeWidth, sampleDistance);
  vFragColor = vec4(color.r * insideness, color.g * insideness, color.b * insideness, insideness);
}
)foo";


