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

#include "characterset.h"

CharacterSet::CharacterSet()
{

}

CharacterSet::CharacterSet(QByteArray chars)
{
  _string = QString(chars);
}

// \x0A   \n    Linefeed (LF)
// \x0B   \e    Escape (ESC)
// \x0C   \f    Formfeed (FF)
// \x0D   \r    Carriage return (CR)
// \x85   …

CharacterSet CharacterSet::newlineCharacterSet()
{
  return CharacterSet(QByteArray("\x0A\x0B\x0C\x0D\x85",5));
}

CharacterSet CharacterSet::whitespaceAndNewlineCharacterSet()
{
  return CharacterSet(QByteArray("\x0A\x0B\x0C\x0D\x85\x0D\x85\x20\x09\xA0",8));
}


CharacterSet CharacterSet::whitespaceCharacterSet()
{
  return CharacterSet(QByteArray("\x20\x09\xA0",3));
}
