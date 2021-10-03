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

#include "openglcrystalcylinderobjectpickingshader.h"
#include "glgeterror.h"

OpenGLCrystalCylinderObjectPickingShader::OpenGLCrystalCylinderObjectPickingShader(OpenGLObjectShader &objectShader):
      _crystalCylinderShader(objectShader._crystalCylinderObjectShader)
{
}


void OpenGLCrystalCylinderObjectPickingShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}


void OpenGLCrystalCylinderObjectPickingShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());
    glDeleteVertexArrays(size, _vertexArrayObject[i].data());
  }
}

void OpenGLCrystalCylinderObjectPickingShader::generateBuffers()
{
  _vertexArrayObject.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _vertexArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    int size = static_cast<GLsizei>(_renderStructures[i].size());

    glGenVertexArrays(size, _vertexArrayObject[i].data());
  }
}

void OpenGLCrystalCylinderObjectPickingShader::paintGL(GLuint structureUniformBuffer)
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
        if (RKRenderCrystalPrimitiveCylinderObjectsSource* object = dynamic_cast<RKRenderCrystalPrimitiveCylinderObjectsSource*>(_renderStructures[i][j].get()))
        {
          if(source->drawAtoms() && _renderStructures[i][j]->isVisible() && _crystalCylinderShader._numberOfIndices[i][j]>0 && _crystalCylinderShader._numberOfDrawnAtoms[i][j]>0)
          {
            glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, index * sizeof(RKStructureUniforms), sizeof(RKStructureUniforms));
            check_gl_error();

            glBindVertexArray(_vertexArrayObject[i][j]);
            check_gl_error();

            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(_crystalCylinderShader._numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_crystalCylinderShader._numberOfDrawnAtoms[i][j]));

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


void OpenGLCrystalCylinderObjectPickingShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLCrystalCylinderObjectPickingShader::initializeVertexArrayObject()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if (RKRenderCrystalPrimitiveCylinderObjectsSource* object = dynamic_cast<RKRenderCrystalPrimitiveCylinderObjectsSource*>(_renderStructures[i][j].get()))
      {
        glBindVertexArray(_vertexArrayObject[i][j]);
        check_gl_error();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _crystalCylinderShader._indexBuffer[i][j]);
        check_gl_error();

        glBindBuffer(GL_ARRAY_BUFFER, _crystalCylinderShader._vertexBuffer[i][j]);
        check_gl_error();
        glVertexAttribPointer(_vertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));
        check_gl_error();

        glBindBuffer(GL_ARRAY_BUFFER,_crystalCylinderShader._instancePositionBuffer[i][j]);
        check_gl_error();
        glVertexAttribPointer(_instancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,position)));
        check_gl_error();
        glVertexAttribDivisor(_instancePositionAttributeLocation, 1);
        check_gl_error();

        glVertexAttribIPointer(_tagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,tag)));
        check_gl_error();
        glVertexAttribDivisor(_tagAttributeLocation, 1);
        check_gl_error();

        glEnableVertexAttribArray(_vertexPositionAttributeLocation);
        glEnableVertexAttribArray(_instancePositionAttributeLocation);
        glEnableVertexAttribArray(_tagAttributeLocation);
        check_gl_error();

        glBindVertexArray(0);
      }
    }
  }
}

void OpenGLCrystalCylinderObjectPickingShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;
  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLCrystalCylinderObjectPickingShader::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLCrystalCylinderObjectPickingShader::_fragmentShaderSource.c_str());

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


const std::string  OpenGLCrystalCylinderObjectPickingShader::_vertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
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

const std::string  OpenGLCrystalCylinderObjectPickingShader::_fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
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
