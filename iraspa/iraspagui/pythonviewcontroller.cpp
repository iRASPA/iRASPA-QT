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

#include "pythonviewcontroller.h"
#include <iostream>
#include <string>
#include <QString>


PythonViewController::PythonViewController(QWidget *parent): LineNumberPlainTextEdit(parent)
{
  appendPlainText("Python console ready");

  int cursorWidth = fontMetrics().boundingRect(QLatin1Char('9')).width();
  setCursorWidth(cursorWidth);

  #if defined(Q_OS_LINUX)
    static wchar_t name[] = L"iRASPA";
    Py_SetProgramName(name);
  #elif defined(Q_OS_WIN)
    #if PY_MAJOR_VERSION >= 3
    {
      static wchar_t name[] = L"iRASPA";
      Py_SetProgramName(name);
    }
    #else
    {
      static char name[] = "iRASPA";
      Py_SetProgramName(name);
    }
    #endif
  #else
    #if PY_MAJOR_VERSION >= 3
    {
      static wchar_t name[] = L"iRASPA";
      Py_SetProgramName(name);
    }
    #else
    {
      static char name[] = "iRASPA";
      Py_SetProgramName(name);
    }
    #endif
  #endif


  Py_NoSiteFlag = 1;
  Py_Initialize();

  

  // have a new interpreter per document
  _state = Py_NewInterpreter();

  PyThreadState_Swap(_state);

  static const std::string string =
"import sys\n\
import math\n\
class CatchOutErr:\n\
    def __init__(self):\n\
        self.value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
CatchOutErr = CatchOutErr()\n\
sys.stdout = CatchOutErr\n\
sys.stderr = CatchOutErr\n";


  PyObject *pModule = PyImport_AddModule("__main__");
  PyRun_SimpleString(string.c_str());

  _catcher = PyObject_GetAttrString(pModule,"CatchOutErr");
  PyErr_Print();

  PyObject *output = PyObject_GetAttrString(_catcher,"value");

  #if defined(Q_OS_LINUX)
    PyObject* utf8string = PyUnicode_AsUTF8String(output);
  #else
    PyObject* utf8string = PyObject_Str(output);
  #endif
  appendPlainText(QString::fromUtf8(PyBytes_AsString(utf8string)));

}

void PythonViewController::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Return)
  {
    ExecutePythonCommand(textCursor().block().text());
  }
  else
  {
    LineNumberPlainTextEdit::keyPressEvent(event);
  }
}

void PythonViewController::ExecutePythonCommand(QString command)
{
  PyObject *out = PyUnicode_FromString("");
  PyObject_SetAttrString(_catcher, "value", out);
  PyRun_SimpleString(command.toStdString().c_str());
  PyErr_Print();

  PyObject *output = PyObject_GetAttrString(_catcher,"value");


  #if defined(Q_OS_LINUX)
    PyObject* outputString = PyUnicode_AsUTF8String(output);
  #else
    PyObject* outputString = PyObject_Str(output);
  #endif
  appendPlainText(QString::fromUtf8(PyBytes_AsString(outputString)));

  Py_DECREF(output);
  Py_DECREF(out);
}

PythonViewController::~PythonViewController()
{
  PyThreadState_Swap(_state);
  Py_EndInterpreter(_state);
  PyThreadState_Swap(nullptr);
}
