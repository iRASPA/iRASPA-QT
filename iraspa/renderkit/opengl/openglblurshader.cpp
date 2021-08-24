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

#include "openglblurshader.h"
#include <iostream>
#include "glgeterror.h"
#include "rkrenderuniforms.h"
#include "quadgeometry.h"
#include <QtDebug>
#include <QtGlobal>
#include <QDebug>

OpenGLBlurShader::OpenGLBlurShader()
{
}


void OpenGLBlurShader::resizeGL(int width, int height)
{
  GLenum status;

  if(_blurDownSamplerTexture)
    glDeleteTextures(1, &_blurDownSamplerTexture);
  glGenTextures(1, &_blurDownSamplerTexture);

  if(_blurDownSamplerFrameBufferObject)
    glDeleteFramebuffers(1,&_blurDownSamplerFrameBufferObject);
  glGenFramebuffers(1,&_blurDownSamplerFrameBufferObject);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, _blurDownSamplerFrameBufferObject);
  check_gl_error();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _blurDownSamplerTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, std::max(width,16), std::max(height,16), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurDownSamplerTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("resize initializeHorizontalBlurFrameBuffer fatal error: framebuffer incomplete: %x",status);
    check_gl_error();
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if(_blurHorizontalTexture)
    glDeleteTextures(1, &_blurHorizontalTexture);
  glGenTextures(1, &_blurHorizontalTexture);

  if(_blurHorizontalFrameBufferObject)
    glDeleteFramebuffers(1,&_blurHorizontalFrameBufferObject);
  glGenFramebuffers(1,&_blurHorizontalFrameBufferObject);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, _blurHorizontalFrameBufferObject);
  check_gl_error();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _blurHorizontalTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, std::max(width,16), std::max(height,16), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurHorizontalTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("resize initializeHorizontalBlurFrameBuffer fatal error: framebuffer incomplete: %x", status);
    check_gl_error();
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  if(_blurVerticalTexture)
    glDeleteTextures(1, &_blurVerticalTexture);
  glGenTextures(1, &_blurVerticalTexture);
  if(_blurVerticalFrameBufferObject)
    glDeleteFramebuffers(1,&_blurVerticalFrameBufferObject);
  glGenFramebuffers(1,&_blurVerticalFrameBufferObject);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, _blurVerticalFrameBufferObject);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _blurVerticalTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, std::max(width,16), std::max(height,16), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurVerticalTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("resize initializeVerticalBlurFrameBuffer fatal error: framebuffer incomplete: %x", status);
    check_gl_error();
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLBlurShader::initializeFramebuffers()
{
  GLenum status;


  glGenFramebuffers(1, &_blurDownSamplerFrameBufferObject);
  glGenTextures(1, &_blurDownSamplerTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, _blurDownSamplerFrameBufferObject);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D, _blurDownSamplerTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurDownSamplerTexture, 0);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, 0);

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("init initializeHorizontalBlurFrameBuffer fatal error: framebuffer incomplete: %d", status);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(1, &_blurHorizontalFrameBufferObject);
  glGenTextures(1, &_blurHorizontalTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, _blurHorizontalFrameBufferObject);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D, _blurHorizontalTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurHorizontalTexture, 0);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, 0);

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("init horizontal initializeHorizontalBlurFrameBuffer fatal error: framebuffer incomplete");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);



  glGenFramebuffers(1, &_blurVerticalFrameBufferObject);
  glGenTextures(1, &_blurVerticalTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, _blurVerticalFrameBufferObject);

  glBindTexture(GL_TEXTURE_2D, _blurVerticalTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurVerticalTexture, 0);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, 0);

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("init vertical initializeVerticalBlurFrameBuffer fatal error: framebuffer incomplete: %d", status);
  }
  glBindFramebuffer(GLenum(GL_FRAMEBUFFER), 0);
}

void OpenGLBlurShader::paintGL(GLuint texture, int width, int height)
{
   glViewport(0,0,width,height);
    glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), _blurDownSamplerFrameBufferObject);
    check_gl_error();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    check_gl_error();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(_blurDownSamplerProgram);
    check_gl_error();

    glBindVertexArray(_blurDownSamplerVertexArray);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    check_gl_error();
    glUniform1i(_blurDownSamplerInputTextureUniformLocation, 0);

    glDrawElements(GLenum(GL_TRIANGLE_STRIP),4,GLenum(GL_UNSIGNED_SHORT),nullptr);
    check_gl_error();

    glBindVertexArray(0);

    glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER),0);
    check_gl_error();

    /*
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
  glReadBuffer(GL_COLOR_ATTACHMENT1);

  check_gl_error();
  glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), _blurHorizontalFrameBufferObject);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  check_gl_error();

  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  check_gl_error();
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
*/


  glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), _blurHorizontalFrameBufferObject);
  check_gl_error();
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  check_gl_error();

  glUseProgram(_blurHorizontalProgram);
  check_gl_error();

  glBindVertexArray(_blurHorizontalVertexArray);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _blurDownSamplerTexture);
  check_gl_error();
  glUniform1i(_blurHorizontalInputTextureUniformLocation, 0);

  glDrawElements(GLenum(GL_TRIANGLE_STRIP),4,GLenum(GL_UNSIGNED_SHORT),nullptr);
  check_gl_error();

  glBindVertexArray(0);

  glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER),0);
  check_gl_error();

  glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), _blurVerticalFrameBufferObject);
  //check_gl_error();

  glUseProgram(_blurVerticalProgram);
  check_gl_error();

  glBindVertexArray(_blurVerticalVertexArray);
  check_gl_error();

  glActiveTexture(GLenum(GL_TEXTURE0));
  glBindTexture(GLenum(GL_TEXTURE_2D), _blurHorizontalTexture);
  check_gl_error();
  glUniform1i(GLint(_blurVerticalInputTextureUniformLocation), 0);
  check_gl_error();


  glDrawElements(GLenum(GL_TRIANGLE_STRIP),4,GLenum(GL_UNSIGNED_SHORT),nullptr);
  check_gl_error();

  glBindVertexArray(0);
  glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER),0);
  check_gl_error();
}

void OpenGLBlurShader::initializeVertexArrayObject()
{
  QuadGeometry quad = QuadGeometry();

  glGenVertexArrays(1, &_blurDownSamplerVertexArray);
  glGenBuffers(1, &_blurDownSamplerVertexBuffer);
  glGenBuffers(1, &_blurDownSamplerIndexBuffer);

  glBindVertexArray(_blurDownSamplerVertexArray);

  glBindBuffer(GL_ARRAY_BUFFER, _blurDownSamplerVertexBuffer);
  if(quad.vertices().size()>0)
  {
    glBufferData(GL_ARRAY_BUFFER, quad.vertices().size()*sizeof(RKVertex), quad.vertices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _blurDownSamplerIndexBuffer);
  if(quad.indices().size()>0)
  {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad.indices().size() * sizeof(GLushort), quad.indices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glVertexAttribPointer(_blurDownSamplerVertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex),  reinterpret_cast<GLvoid*>(offsetof(RKVertex,position)));
  glEnableVertexAttribArray(_blurDownSamplerVertexPositionAttributeLocation);
  check_gl_error();

  glBindVertexArray(0);

  glGenVertexArrays(1, &_blurHorizontalVertexArray);
  glGenBuffers(1, &_blurHorizontalVertexBuffer);
  glGenBuffers(1, &_blurHorizontalIndexBuffer);

  glBindVertexArray(_blurHorizontalVertexArray);

  glBindBuffer(GL_ARRAY_BUFFER, _blurHorizontalVertexBuffer);
  if(quad.vertices().size()>0)
  {
    glBufferData(GL_ARRAY_BUFFER, quad.vertices().size()*sizeof(RKVertex), quad.vertices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _blurHorizontalIndexBuffer);
  if(quad.indices().size()>0)
  {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad.indices().size() * sizeof(GLushort), quad.indices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glVertexAttribPointer(_blurHorizontalVertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex),  reinterpret_cast<GLvoid*>(offsetof(RKVertex,position)));
  glEnableVertexAttribArray(_blurHorizontalVertexPositionAttributeLocation);
  check_gl_error();

  glBindVertexArray(0);

  glGenVertexArrays(1, &_blurVerticalVertexArray);
  glGenBuffers(1, &_blurVerticalVertexBuffer);
  glGenBuffers(1, &_blurVerticalIndexBuffer);

  glBindVertexArray(_blurVerticalVertexArray);
  check_gl_error();

  glBindBuffer(GL_ARRAY_BUFFER, _blurVerticalVertexBuffer);
  if(quad.vertices().size()>0)
  {
    glBufferData(GL_ARRAY_BUFFER, quad.vertices().size()*sizeof(RKVertex), quad.vertices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glBindBuffer(GLenum(GL_ELEMENT_ARRAY_BUFFER), _blurVerticalIndexBuffer);
  if(quad.indices().size()>0)
  {
    glBufferData(GLenum(GL_ELEMENT_ARRAY_BUFFER), quad.indices().size() * sizeof(GLushort), quad.indices().data(), GL_STATIC_DRAW);
    check_gl_error();
  }

  glVertexAttribPointer(_blurVerticalVertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex,position)));
  glEnableVertexAttribArray(_blurVerticalVertexPositionAttributeLocation);
  check_gl_error();

  glBindVertexArray(0);
}

void OpenGLBlurShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLBlurShader::_downSamplerVertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLBlurShader::_downSamplerFragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _blurDownSamplerProgram = glCreateProgram();
    check_gl_error();

    glAttachShader(_blurDownSamplerProgram, vertexShader);
    check_gl_error();
    glAttachShader(_blurDownSamplerProgram, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_blurDownSamplerProgram, 0, "vFragColor");

    linkProgram(_blurDownSamplerProgram);


    _blurDownSamplerInputTextureUniformLocation = glGetUniformLocation(_blurDownSamplerProgram, "originalTexture");
    _blurDownSamplerVertexPositionAttributeLocation = glGetAttribLocation(_blurDownSamplerProgram, "position");

    if (_blurDownSamplerInputTextureUniformLocation < 0) qDebug() << "Shader did not contain the 'originalTexture' uniform.";
    if (_blurDownSamplerVertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'position' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLBlurShader::_blurHorizontalVertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLBlurShader::_fragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _blurHorizontalProgram = glCreateProgram();
    check_gl_error();

    glAttachShader(_blurHorizontalProgram, vertexShader);
    check_gl_error();
    glAttachShader(_blurHorizontalProgram, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_blurHorizontalProgram, 0, "vFragColor");

    linkProgram(_blurHorizontalProgram);


    _blurHorizontalInputTextureUniformLocation = glGetUniformLocation(_blurHorizontalProgram, "s_texture");
    _blurHorizontalVertexPositionAttributeLocation = glGetAttribLocation(_blurHorizontalProgram, "position");

    if (_blurHorizontalInputTextureUniformLocation < 0) qDebug() << "Shader did not contain the 's_texture' uniform.";
    if (_blurHorizontalVertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'position' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLBlurShader::_blurVerticalVertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLBlurShader::_fragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _blurVerticalProgram = glCreateProgram();
    check_gl_error();

    glAttachShader(_blurVerticalProgram, vertexShader);
    check_gl_error();
    glAttachShader(_blurVerticalProgram, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_blurVerticalProgram, 0, "vFragColor");

    linkProgram(_blurVerticalProgram);


    _blurVerticalInputTextureUniformLocation = glGetUniformLocation(_blurVerticalProgram, "s_texture");
    _blurVerticalVertexPositionAttributeLocation = glGetAttribLocation(_blurVerticalProgram, "position");

    if (_blurVerticalInputTextureUniformLocation < 0) qDebug() << "Shader did not contain the 's_texture' uniform.";
    if (_blurVerticalVertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'position' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string OpenGLBlurShader::_downSamplerVertexShaderSource =
OpenGLVersionStringLiteral +
std::string(R"foo(
in vec4 position;

out vec2 texcoord;

void main()
{
  gl_Position = position;
  texcoord = position.xy * vec2(0.5) + vec2(0.5);
}
)foo");

const std::string OpenGLBlurShader::_downSamplerFragmentShaderSource =
OpenGLVersionStringLiteral +
//OpenGLFrameUniformBlockStringLiteral +
std::string(R"foo(
in vec2 texcoord;
out vec4 vFragColor;

uniform sampler2DMS originalTexture;

void main()
{
  vec4 sampleValue;

  // Calculate un-normalized texture coordinates
  vec2 tmp = floor(textureSize(originalTexture) * texcoord);

  vFragColor = texelFetch(originalTexture, ivec2(tmp), 0);
}
)foo");

const std::string OpenGLBlurShader::_blurHorizontalVertexShaderSource =
OpenGLVersionStringLiteral +
std::string(R"foo(
in vec4 position;

out vec2 v_texCoord;
out vec2 v_blurTexCoords[14];

void main()
{
  gl_Position = position;
  v_texCoord = position.xy * vec2(0.5) + vec2(0.5);
  v_blurTexCoords[ 0] = v_texCoord + vec2(-0.028, 0.0);
  v_blurTexCoords[ 1] = v_texCoord + vec2(-0.024, 0.0);
  v_blurTexCoords[ 2] = v_texCoord + vec2(-0.020, 0.0);
  v_blurTexCoords[ 3] = v_texCoord + vec2(-0.016, 0.0);
  v_blurTexCoords[ 4] = v_texCoord + vec2(-0.012, 0.0);
  v_blurTexCoords[ 5] = v_texCoord + vec2(-0.008, 0.0);
  v_blurTexCoords[ 6] = v_texCoord + vec2(-0.004, 0.0);
  v_blurTexCoords[ 7] = v_texCoord + vec2( 0.004, 0.0);
  v_blurTexCoords[ 8] = v_texCoord + vec2( 0.008, 0.0);
  v_blurTexCoords[ 9] = v_texCoord + vec2( 0.012, 0.0);
  v_blurTexCoords[10] = v_texCoord + vec2( 0.016, 0.0);
  v_blurTexCoords[11] = v_texCoord + vec2( 0.020, 0.0);
  v_blurTexCoords[12] = v_texCoord + vec2( 0.024, 0.0);
  v_blurTexCoords[13] = v_texCoord + vec2( 0.028, 0.0);
}
)foo");



const std::string OpenGLBlurShader::_blurVerticalVertexShaderSource =
OpenGLVersionStringLiteral +
std::string(R"foo(
in vec4 position;

out vec2 v_texCoord;
out vec2 v_blurTexCoords[14];

void main()
{
  gl_Position = position;
  v_texCoord = position.xy * vec2(0.5) + vec2(0.5);
  v_blurTexCoords[ 0] = v_texCoord + vec2(0.0, -0.028);
  v_blurTexCoords[ 1] = v_texCoord + vec2(0.0, -0.024);
  v_blurTexCoords[ 2] = v_texCoord + vec2(0.0, -0.020);
  v_blurTexCoords[ 3] = v_texCoord + vec2(0.0, -0.016);
  v_blurTexCoords[ 4] = v_texCoord + vec2(0.0, -0.012);
  v_blurTexCoords[ 5] = v_texCoord + vec2(0.0, -0.008);
  v_blurTexCoords[ 6] = v_texCoord + vec2(0.0, -0.004);
  v_blurTexCoords[ 7] = v_texCoord + vec2(0.0,  0.004);
  v_blurTexCoords[ 8] = v_texCoord + vec2(0.0,  0.008);
  v_blurTexCoords[ 9] = v_texCoord + vec2(0.0,  0.012);
  v_blurTexCoords[10] = v_texCoord + vec2(0.0,  0.016);
  v_blurTexCoords[11] = v_texCoord + vec2(0.0,  0.020);
  v_blurTexCoords[12] = v_texCoord + vec2(0.0,  0.024);
  v_blurTexCoords[13] = v_texCoord + vec2(0.0,  0.028);
}
)foo");


const std::string OpenGLBlurShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
std::string(R"foo(
uniform sampler2D s_texture;

in vec2 v_texCoord;
in vec2 v_blurTexCoords[14];

out vec4 vFragColor;

void main()
{
  vFragColor = vec4(0.0);
  vFragColor += texture(s_texture, v_blurTexCoords[ 0])*0.0044299121055113265;
  vFragColor += texture(s_texture, v_blurTexCoords[ 1])*0.00895781211794;
  vFragColor += texture(s_texture, v_blurTexCoords[ 2])*0.0215963866053;
  vFragColor += texture(s_texture, v_blurTexCoords[ 3])*0.0443683338718;
  vFragColor += texture(s_texture, v_blurTexCoords[ 4])*0.0776744219933;
  vFragColor += texture(s_texture, v_blurTexCoords[ 5])*0.115876621105;
  vFragColor += texture(s_texture, v_blurTexCoords[ 6])*0.147308056121;
  vFragColor += texture(s_texture, v_texCoord         )*0.159576912161;
  vFragColor += texture(s_texture, v_blurTexCoords[ 7])*0.147308056121;
  vFragColor += texture(s_texture, v_blurTexCoords[ 8])*0.115876621105;
  vFragColor += texture(s_texture, v_blurTexCoords[ 9])*0.0776744219933;
  vFragColor += texture(s_texture, v_blurTexCoords[10])*0.0443683338718;
  vFragColor += texture(s_texture, v_blurTexCoords[11])*0.0215963866053;
  vFragColor += texture(s_texture, v_blurTexCoords[12])*0.00895781211794;
  vFragColor += texture(s_texture, v_blurTexCoords[13])*0.0044299121055113265;
}
)foo");
