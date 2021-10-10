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

#include "glgeterror.h"
#include <QOpenGLFunctions_3_3_Core>
#include "mathkit.h"

void check_gl_error_local(const char *file, int line)
{
  QOpenGLContext *context = QOpenGLContext::currentContext();

  if(context)
  {
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if(f)
    {
      GLenum err (f->glGetError());

      while(err!=GL_NO_ERROR)
      {
        std::string error;

        switch(err) {
           case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
           case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
           case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
           case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
           case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
           default: error="UNKNOWN"; break;
        }

        qDebug() << "GL_" << error.c_str() << " - " << file <<":" << line;
        std::cout << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
        err=f->glGetError();
      }
    }
  }
}


