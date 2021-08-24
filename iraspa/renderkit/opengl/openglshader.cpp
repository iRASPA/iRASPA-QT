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

#include "openglshader.h"
#include "glgeterror.h"
#include <QDebug>
#include <iostream>

OpenGLShader::OpenGLShader()
{

}

OpenGLShader::~OpenGLShader()
{
}


GLuint OpenGLShader::compileShaderOfType(GLenum shaderType,const GLchar * shaderSourceCode)
{
  GLint status;
  GLuint shader;
  GLint length;

  shader = glCreateShader(shaderType);
  check_gl_error();
  length= static_cast<GLint>(strlen(shaderSourceCode));
  glShaderSource(shader, 1, &shaderSourceCode, &length);
  check_gl_error();
  glCompileShader(shader);
  check_gl_error();
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  check_gl_error();
  if (status == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<char> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog.data());
    std::string log;
    for (size_t i = 0; i < errorLog.size(); i++)
    {
      log += errorLog[i];
    }

    std::cout << "ERROR IN SHADER: " << log << std::endl;

    glDeleteShader(shader);
    check_gl_error();
  }
  return shader;
}

void OpenGLShader::linkProgram(GLuint program)
{
  GLint status;

  glLinkProgram(program);
  check_gl_error();

  glGetProgramiv(program, GL_LINK_STATUS, &status);
  check_gl_error();
  if (0 == status)
  {
    qDebug() <<"Failed to link shader program";
  }
}
