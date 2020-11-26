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

#pragma once

#include <QObject>
#include "linenumberplaintextedit.h"

#ifdef Q_OS_MAC
  #define register
#endif

// slots defined in pythoni (object.h) clashes with the QT "slot" keyword
#pragma push_macro("slots")
#undef slots
  #ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG
  #else
    #include <Python.h>
  #endif
#pragma pop_macro("slots")


class PythonViewController: public LineNumberPlainTextEdit
{
  Q_OBJECT

public:
    PythonViewController(QWidget *parent);
    ~PythonViewController();
private:
  PyThreadState* _state;
  static PythonViewController* _currentInstance;
  static PyObject* log_CaptureStdout(PyObject* self, PyObject* pArgs);
  static PyObject* log_CaptureStderr(PyObject* self, PyObject* pArgs);
  void ExecutePythonCommand(QString command);

  PyObject *_catcher;
  PyObject *_sys;
protected:
  void keyPressEvent(QKeyEvent *event) override final;
};
