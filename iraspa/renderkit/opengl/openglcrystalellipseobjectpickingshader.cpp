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

#include "openglcrystalellipseobjectpickingshader.h"
#include "glgeterror.h"
#include "opengluniformstringliterals.h"

OpenGLCrystalEllipseObjectPickingShader::OpenGLCrystalEllipseObjectPickingShader(OpenGLObjectShader &objectShader):
      _crystalEllipseShader(objectShader._crystalEllipseObjectShader)
{
}


void OpenGLCrystalEllipseObjectPickingShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}


void OpenGLCrystalEllipseObjectPickingShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());
  }
}

void OpenGLCrystalEllipseObjectPickingShader::generateBuffers()
{
  _vertexArrayObject.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _vertexArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _vertexArrayObject[i].data());

  }
}

void OpenGLCrystalEllipseObjectPickingShader::paintGL(GLuint structureUniformBuffer)
{
  glUseProgram(_program);
  check_gl_error();

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderPrimitiveObjectsSource* source = dynamic_cast<RKRenderPrimitiveObjectsSource*>(_renderStructures[i][j].get()))
      {
        if (RKRenderCrystalPrimitiveEllipsoidObjectsSource* object = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource*>(_renderStructures[i][j].get()))
        {
          if(source->drawAtoms() && _renderStructures[i][j]->isVisible() && _crystalEllipseShader._numberOfIndices[i][j]>0 && _crystalEllipseShader._numberOfDrawnAtoms[i][j]>0)
          {
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, index * sizeof(RKStructureUniforms), sizeof(RKStructureUniforms));
            glBindVertexArray(_vertexArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_crystalEllipseShader._numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_crystalEllipseShader._numberOfDrawnAtoms[i][j]));
            check_gl_error();
            glBindVertexArray(0);
          }
        }
      }
      index++;
    }
  }
  glUseProgram(0);

}


void OpenGLCrystalEllipseObjectPickingShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLCrystalEllipseObjectPickingShader::initializeVertexArrayObject()
{

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderCrystalPrimitiveEllipsoidObjectsSource* source = dynamic_cast<RKRenderCrystalPrimitiveEllipsoidObjectsSource*>(_renderStructures[i][j].get()))
      {
        glBindVertexArray(_vertexArrayObject[i][j]);

        glBindBuffer(GL_ARRAY_BUFFER, _crystalEllipseShader._vertexBuffer[i][j]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _crystalEllipseShader._indexBuffer[i][j]);

        glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));

        glBindBuffer(GL_ARRAY_BUFFER,_crystalEllipseShader._instancePositionBuffer[i][j]);
        glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,position)));
        glVertexAttribDivisor(_instancePositionAttributeLocation, 1);

        glVertexAttribIPointer(_tagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,tag)));
        glVertexAttribDivisor(_tagAttributeLocation, 1);

        glEnableVertexAttribArray(_vertexPositionAttributeLocation);
        glEnableVertexAttribArray(_instancePositionAttributeLocation);
        glEnableVertexAttribArray(_tagAttributeLocation);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
      }
    }
  }
}

void OpenGLCrystalEllipseObjectPickingShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;
  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLCrystalEllipseObjectPickingShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLCrystalEllipseObjectPickingShader::_fragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _program = glCreateProgram();
    check_gl_error();

    glAttachShader(_program, vertexShader);
    check_gl_error();
    glAttachShader(_program, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_program, 0, "vFragColor");

    linkProgram(_program);

    _vertexPositionAttributeLocation = glGetAttribLocation(_program, "vertexPosition");
    if (_vertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";

    _instancePositionAttributeLocation = glGetAttribLocation(_program, "instancePosition");
    if (_instancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";

    _tagAttributeLocation = glGetAttribLocation(_program, "instanceTag");
    if (_tagAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceTag' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}


const std::string  OpenGLCrystalEllipseObjectPickingShader::_vertexShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;
in vec4 instancePosition;
in int instanceTag;

// Inputs from vertex shader
out VS_OUT
{
  flat int instanceId;
} vs_out;

void main(void)
{
  vec4 pos = instancePosition + structureUniforms.transformationMatrix * vertexPosition;
  vs_out.instanceId = instanceTag;
  gl_Position = frameUniforms.mvpMatrix * structureUniforms.modelMatrix * pos;
}
)foo";

const std::string  OpenGLCrystalEllipseObjectPickingShader::_fragmentShaderSource =
OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
OpenGLUniformStringLiterals::OpenGLStructureUniformBlockStringLiteral +
R"foo(
// Input from vertex shader
in VS_OUT
{
  flat int instanceId;
} fs_in;

out uvec4 vFragColor;

void main(void)
{
  vFragColor = uvec4(1,structureUniforms.sceneIdentifier,structureUniforms.MovieIdentifier,fs_in.instanceId);
}
)foo";
