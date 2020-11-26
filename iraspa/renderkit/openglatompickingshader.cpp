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

#include "openglatompickingshader.h"
#include "openglatomshader.h"
#include "openglatomorthographicimpostershader.h"

OpenGLAtomPickingShader::OpenGLAtomPickingShader(OpenGLAtomShader &atomShader):
   _atomShader(atomShader._atomShader), _atomOrthographicImposterShader(atomShader._atomOrthographicImposterShader)
{
}


void OpenGLAtomPickingShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  deleteBuffers();
  _renderStructures = structures;
  generateBuffers();
}


void OpenGLAtomPickingShader::deleteBuffers()
{
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glDeleteVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _atomPickingVertexArrayObject[i].data());
  }
}

void OpenGLAtomPickingShader::generateBuffers()
{
  _atomPickingVertexArrayObject.resize(_renderStructures.size());

  for(size_t i=0l;i<_renderStructures.size();i++)
  {
    _atomPickingVertexArrayObject[i].resize(_renderStructures[i].size());
  }

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    glGenVertexArrays(static_cast<GLsizei>(_renderStructures[i].size()), _atomPickingVertexArrayObject[i].data());

  }
}

void OpenGLAtomPickingShader::paintGL(GLuint structureUniformBuffer)
{
  glUseProgram(_atomPickingProgram);
  check_gl_error();

  int index = 0;
  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      if(_renderStructures[i][j]->drawAtoms() && _renderStructures[i][j]->isVisible() && _atomOrthographicImposterShader._numberOfIndices[i][j]>0 && _atomShader._numberOfDrawnAtoms[i][j]>0)
      {
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, structureUniformBuffer, index * sizeof(RKStructureUniforms), sizeof(RKStructureUniforms));
        glBindVertexArray(_atomPickingVertexArrayObject[i][j]);
        check_gl_error();

        glDrawElementsInstanced(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_atomOrthographicImposterShader._numberOfIndices[i][j]), GL_UNSIGNED_SHORT, nullptr, static_cast<GLsizei>(_atomShader._numberOfDrawnAtoms[i][j]));
        check_gl_error();
        glBindVertexArray(0);
      }
      index++;
    }
  }
  glUseProgram(0);

}


void OpenGLAtomPickingShader::reloadData()
{
  initializeVertexArrayObject();
}

void OpenGLAtomPickingShader::initializeVertexArrayObject()
{

  for(size_t i=0;i<_renderStructures.size();i++)
  {
    for(size_t j=0;j<_renderStructures[i].size();j++)
    {
      glBindVertexArray(_atomPickingVertexArrayObject[i][j]);

      glBindBuffer(GL_ARRAY_BUFFER, _atomOrthographicImposterShader._vertexBuffer[i][j]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _atomOrthographicImposterShader._indexBuffer[i][j]);

      glVertexAttribPointer(_atomPickingVertexPositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), (GLvoid *)offsetof(RKVertex,position));

      glBindBuffer(GL_ARRAY_BUFFER,_atomShader._instancePositionBuffer[i][j]);
      glVertexAttribPointer(_atomPickingInstancePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,position)));
      glVertexAttribDivisor(_atomPickingInstancePositionAttributeLocation, 1);


      glVertexAttribPointer(_atomPickingScaleAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,scale)));
      glVertexAttribDivisor(_atomPickingScaleAttributeLocation, 1);

      glVertexAttribIPointer(_atomPickingTagAttributeLocation, 1, GL_INT, sizeof(RKInPerInstanceAttributesAtoms), reinterpret_cast<GLvoid*>(offsetof(RKInPerInstanceAttributesAtoms,tag)));
      glVertexAttribDivisor(_atomPickingTagAttributeLocation, 1);

      glEnableVertexAttribArray(_atomPickingVertexPositionAttributeLocation);
      glEnableVertexAttribArray(_atomPickingInstancePositionAttributeLocation);
      glEnableVertexAttribArray(_atomPickingScaleAttributeLocation);
      glEnableVertexAttribArray(_atomPickingTagAttributeLocation);


      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }
  }
}

void OpenGLAtomPickingShader::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;
  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLAtomPickingShader::_atomVertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLAtomPickingShader::_atomFragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _atomPickingProgram = glCreateProgram();
    check_gl_error();

    glAttachShader(_atomPickingProgram, vertexShader);
    check_gl_error();
    glAttachShader(_atomPickingProgram, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_atomPickingProgram, 0, "vFragColor");

    linkProgram(_atomPickingProgram);

    _atomPickingVertexPositionAttributeLocation = glGetAttribLocation(_atomPickingProgram, "vertexPosition");
    if (_atomPickingVertexPositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";

    _atomPickingInstancePositionAttributeLocation = glGetAttribLocation(_atomPickingProgram, "instancePosition");
    if (_atomPickingInstancePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";

    _atomPickingScaleAttributeLocation = glGetAttribLocation(_atomPickingProgram, "instanceScale");
    if (_atomPickingScaleAttributeLocation < 0) qDebug() << "Shader did not contain the 'vertexPosition' attribute.";

    _atomPickingTagAttributeLocation = glGetAttribLocation(_atomPickingProgram, "instanceTag");
    if (_atomPickingTagAttributeLocation < 0) qDebug() << "Shader did not contain the 'instanceTag' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}


const std::string  OpenGLAtomPickingShader::_atomVertexShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
R"foo(
in vec4 vertexPosition;

in vec4 instancePosition;
in vec4 instanceScale;
in int instanceTag;

// Inputs from vertex shader
out VS_OUT
{
  smooth vec4 eye_position;
  smooth vec2 texcoords;
  smooth vec3 frag_pos;
  flat vec3 frag_center;
  flat vec4 sphere_radius;
  flat int instanceId;
} vs_out;


void main(void)
{
  vs_out.instanceId = instanceTag;
  vec4 scale = structureUniforms.atomScaleFactor * instanceScale;

  vs_out.eye_position = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;

  vs_out.texcoords = vertexPosition.xy;
  vs_out.sphere_radius = scale;
  vec4 pos2 = frameUniforms.viewMatrix * structureUniforms.modelMatrix * instancePosition;
  pos2.xy += scale.xy * vec2(vertexPosition.x,vertexPosition.y);

  vs_out.frag_pos = pos2.xyz;

  gl_Position =  frameUniforms.projectionMatrix * pos2;
}
)foo";

const std::string  OpenGLAtomPickingShader::_atomFragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
OpenGLStructureUniformBlockStringLiteral +
R"foo(
// Inputs from vertex shader
in VS_OUT
{
  smooth vec4 eye_position;
  smooth vec2 texcoords;
  smooth vec3 frag_pos;
  flat vec3 frag_center;
  flat vec4 sphere_radius;
  flat int instanceId;
} fs_in;

out uvec4 vFragColor;

void main(void)
{
  float x = fs_in.texcoords.x;
  float y = fs_in.texcoords.y;
  float zz = 1.0 - x*x - y*y;

  if (zz <= 0.0)
    discard;

  float z = sqrt(zz);
  vec4 pos = fs_in.eye_position;
  pos.z += fs_in.sphere_radius.z*z;
  pos = frameUniforms.projectionMatrix * pos;
  gl_FragDepth = 0.5*(pos.z / pos.w)+0.5;

  vFragColor = uvec4(1,structureUniforms.sceneIdentifier,structureUniforms.MovieIdentifier,fs_in.instanceId);
}
)foo";
