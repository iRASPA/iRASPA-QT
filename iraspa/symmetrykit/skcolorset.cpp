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

#include "skcolorset.h"
#include <iostream>

SKColorSet::SKColorSet(QString name, SKColorSet& from, bool editable): _displayName(name), _editable(editable)
{
  for(auto const [key, value]: from._colors)
  {
    this->_colors[key] = value;
  }
}

const QColor* SKColorSet::operator[] (QString colorName) const
{
  if(_colors.find(colorName) != _colors.end())
  {
    return &_colors.at(colorName);
  }
  return nullptr;
}

SKColorSet::SKColorSet(ColorScheme scheme)
{
  switch(scheme)
  {
    case ColorScheme::jmol:
      _displayName = "Jmol";
      _colors = SKColorSet::jMol;
      break;
    case ColorScheme::rasmol_modern:
      _displayName = "Rasmol modern";
      _colors = SKColorSet::rasmolModern;
      break;
    case ColorScheme::rasmol:
      _displayName = "Rasmol";
      _colors = SKColorSet::rasmol;
      break;
    case ColorScheme::vesta:
      _displayName = "Vesta";
      _colors = SKColorSet::vesta;
      break;
  default:
    break;
  }
}

QDataStream &operator<<(QDataStream &stream, const SKColorSet &colorSet)
{
  stream << colorSet._versionNumber;
  stream << colorSet._displayName;
  stream << colorSet._editable;
  stream << colorSet._colors;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, SKColorSet &colorSet)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > colorSet._versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKColorSet");
  }

  stream >> colorSet._displayName;
  stream >> colorSet._editable;
  stream >> colorSet._colors;

  return stream;
}


std::map<QString, QColor> SKColorSet::jMol =
{
  {QString("H"), QColor(0xFFFFFF)},  {QString("He"), QColor(0xD9FFFF)}, {QString("Li"), QColor(0xCC80FF)}, {QString("Be"), QColor(0xC2FF00)},
  {QString("B"), QColor(0xFFB5B5)},  {QString("C"), QColor(0x909090)},  {QString("N"),  QColor(0x3050F8)}, {QString("O"),  QColor(0xFF0D0D)},
  {QString("F"), QColor(0x90E050)},  {QString("Ne"),QColor(0xB3E3F5)},  {QString("Na"), QColor(0xAB5CF2)}, {QString("Mg"), QColor(0x8AFF00)},
  {QString("Al"), QColor(0xBFA6A6)}, {QString("Si"), QColor(0xF0C8A0)}, {QString("P"), QColor(0xFF8000)},  {QString("S"), QColor(0xFFFF30)},
  {QString("Cl"), QColor(0x1FF01F)}, {QString("Ar"), QColor(0x80D1E3)}, {QString("K"), QColor(0x8F40D4)},  {QString("Ca"), QColor(0x3DFF00)},
  {QString("Sc"), QColor(0xE6E6E6)}, {QString("Ti"), QColor(0xBFC2C7)}, {QString("V"), QColor(0xA6A6AB)},  {QString("Cr"), QColor(0x8A99C7)},
  {QString("Mn"), QColor(0x9C7AC7)}, {QString("Fe"), QColor(0xE06633)}, {QString("Co"), QColor(0xF090A0)}, {QString("Ni"), QColor(0x50D050)},
  {QString("Cu"), QColor(0xC88033)}, {QString("Zn"), QColor(0x7D80B0)}, {QString("Ga"), QColor(0xC28F8F)}, {QString("Ge"), QColor(0x668F8F)},
  {QString("As"), QColor(0xBD80E3)}, {QString("Se"), QColor(0xFFA100)}, {QString("Br"), QColor(0xA62929)}, {QString("Kr"), QColor(0x5CB8D1)},
  {QString("Rb"), QColor(0x702EB0)}, {QString("Sr"), QColor(0x00FF00)}, {QString("Y"), QColor(0x94FFFF)},  {QString("Zr"), QColor(0x94E0E0)},
  {QString("Nb"), QColor(0x73C2C9)}, {QString("Mo"), QColor(0x54B5B5)}, {QString("Tc"), QColor(0x3B9E9E)}, {QString("Ru"), QColor(0x248F8F)},
  {QString("Rh"), QColor(0x0A7D8C)}, {QString("Pd"), QColor(0x006985)}, {QString("Ag"), QColor(0xC0C0C0)}, {QString("Cd"), QColor(0xFFD98F)},
  {QString("In"), QColor(0xA67573)}, {QString("Sn"), QColor(0x668080)}, {QString("Sb"), QColor(0x9E63B5)}, {QString("Te"), QColor(0xD47A00)},
  {QString("I"), QColor(0x940094)},  {QString("Xe"), QColor(0x429EB0)}, {QString("Cs"), QColor(0x57178F)}, {QString("Ba"), QColor(0x00C900)},
  {QString("La"), QColor(0x70D4FF)}, {QString("Ce"), QColor(0xFFFFC7)}, {QString("Pr"), QColor(0xD9FFC7)}, {QString("Nd"), QColor(0xC7FFC7)},
  {QString("Pm"), QColor(0xA3FFC7)}, {QString("Sm"), QColor(0x8FFFC7)}, {QString("Eu"), QColor(0x61FFC7)}, {QString("Gd"), QColor(0x45FFC7)},
  {QString("Tb"), QColor(0x30FFC7)}, {QString("Dy"), QColor(0x1FFFC7)}, {QString("Ho"), QColor(0x00FF9C)}, {QString("Er"), QColor(0x00E675)},
  {QString("Tm"), QColor(0x00D452)}, {QString("Yb"), QColor(0x00BF38)}, {QString("Lu"), QColor(0x00AB24)}, {QString("Hf"), QColor(0x4DC2FF)},
  {QString("Ta"), QColor(0x4DA6FF)}, {QString("W"), QColor(0x2194D6)}, {QString("Re"), QColor(0x267DAB)},  {QString("Os"), QColor(0x266696)},
  {QString("Ir"), QColor(0x175487)}, {QString("Pt"), QColor(0xD0D0E0)}, {QString("Au"), QColor(0xFFD123)}, {QString("Hg"), QColor(0xB8B8D0)},
  {QString("Tl"), QColor(0xA6544D)}, {QString("Pb"), QColor(0x575961)}, {QString("Bi"), QColor(0x9E4FB5)}, {QString("Po"), QColor(0xAB5C00)},
  {QString("At"), QColor(0x754F45)}, {QString("Rn"), QColor(0x428296)}, {QString("Fr"), QColor(0x420066)}, {QString("Ra"), QColor(0x007D00)},
  {QString("Ac"), QColor(0x70ABFA)}, {QString("Th"), QColor(0x00BAFF)}, {QString("Pa"), QColor(0x00A1FF)}, {QString("U"), QColor(0x008FFF)},
  {QString("Np"), QColor(0x0080FF)}, {QString("Pu"), QColor(0x006BFF)}, {QString("Am"), QColor(0x545CF2)}, {QString("Cm"), QColor(0x785CE3)},
  {QString("Bk"), QColor(0x8A4FE3)}, {QString("Cf"), QColor(0xA136D4)}, {QString("Es"), QColor(0xB31FD4)}, {QString("Fm"), QColor(0xB31FBA)},
  {QString("Md"), QColor(0xB30DA6)}, {QString("No"), QColor(0xBD0D87)}, {QString("Lr"), QColor(0xC70066)}, {QString("Rf"), QColor(0xCC0059)},
  {QString("Db"), QColor(0xD1004F)}, {QString("Sg"), QColor(0xD90045)}, {QString("Bh"), QColor(0xE00038)}, {QString("Hs"), QColor(0xE6002E)},
  {QString("Mt"), QColor(0xEB0026)}, {QString("Ds"), QColor(0xEB0026)}, {QString("Rg"), QColor(0xEB0026)}, {QString("Cn"), QColor(0xEB0026)},
  {QString("Uut"), QColor(0xEB0026)}, {QString("Uuq"), QColor(0xEB0026)}, {QString("Uup"), QColor(0xEB0026)}, {QString("Uuh"), QColor(0xEB0026)},
  {QString("Uus"), QColor(0xEB0026)}, {QString("Uuo"), QColor(0xEB0026)}
};

std::map<QString, QColor> SKColorSet::rasmol =
{
  {QString("H"), QColor(0xFFFFFF)},  {QString("He"), QColor(0xFFC0CB)}, {QString("Li"), QColor(0xB22222)}, {QString("Be"), QColor(0xFF1493)},
  {QString("B"), QColor(0x00FF00)},  {QString("C"), QColor(0xC8C8C8)},  {QString("N"), QColor(0x8F8FFF)},  {QString("O"), QColor(0xF00000)},
  {QString("F"), QColor(0xDAA520)},  {QString("Ne"), QColor(0xFF1493)}, {QString("Na"), QColor(0x0000FF)}, {QString("Mg"), QColor(0x228B22)},
  {QString("Al"), QColor(0x808090)}, {QString("Si"), QColor(0xDAA520)}, {QString("P"), QColor(0xFFA500)},  {QString("S"), QColor(0xFFC832)},
  {QString("Cl"), QColor(0x00FF00)}, {QString("Ar"), QColor(0xFF1493)}, {QString("K"), QColor(0xFF1493)},  {QString("Ca"), QColor(0x808090)},
  {QString("Sc"), QColor(0xFF1493)}, {QString("Ti"), QColor(0x808090)}, {QString("V"), QColor(0xFF1493)},  {QString("Cr"), QColor(0x808090)},
  {QString("Mn"), QColor(0x808090)}, {QString("Fe"), QColor(0xFFA500)}, {QString("Co"), QColor(0xFF1493)}, {QString("Ni"), QColor(0xA52A2A)},
  {QString("Cu"), QColor(0xA52A2A)}, {QString("Zn"), QColor(0xA52A2A)}, {QString("Ga"), QColor(0xFF1493)}, {QString("Ge"), QColor(0xFF1493)},
  {QString("As"), QColor(0xFF1493)}, {QString("Se"), QColor(0xFF1493)}, {QString("Br"), QColor(0xA52A2A)}, {QString("Kr"), QColor(0xFF1493)},
  {QString("Rb"), QColor(0xFF1493)}, {QString("Sr"), QColor(0xFF1493)}, {QString("Y"), QColor(0xFF1493)},  {QString("Zr"), QColor(0xFF1493)},
  {QString("Nb"), QColor(0xFF1493)}, {QString("Mo"), QColor(0xFF1493)}, {QString("Tc"), QColor(0xFF1493)}, {QString("Ru"), QColor(0xFF1493)},
  {QString("Rh"), QColor(0xFF1493)}, {QString("Pd"), QColor(0xFF1493)}, {QString("Ag"), QColor(0x808090)}, {QString("Cd"), QColor(0xFF1493)},
  {QString("In"), QColor(0xFF1493)}, {QString("Sn"), QColor(0xFF1493)}, {QString("Sb"), QColor(0xFF1493)}, {QString("Te"), QColor(0xFF1493)},
  {QString("I"), QColor(0xA020F0)},  {QString("Xe"), QColor(0xFF1493)}, {QString("Cs"), QColor(0xFF1493)}, {QString("Ba"), QColor(0xFFA500)},
  {QString("La"), QColor(0xFF1493)}, {QString("Ce"), QColor(0xFF1493)}, {QString("Pr"), QColor(0xFF1493)}, {QString("Nd"), QColor(0xFF1493)},
  {QString("Pm"), QColor(0xFF1493)}, {QString("Sm"), QColor(0xFF1493)}, {QString("Eu"), QColor(0xFF1493)}, {QString("Gd"), QColor(0xFF1493)},
  {QString("Tb"), QColor(0xFF1493)}, {QString("Dy"), QColor(0xFF1493)}, {QString("Ho"), QColor(0xFF1493)}, {QString("Er"), QColor(0xFF1493)},
  {QString("Tm"), QColor(0xFF1493)}, {QString("Yb"), QColor(0xFF1493)}, {QString("Lu"), QColor(0xFF1493)}, {QString("Hf"), QColor(0xFF1493)},
  {QString("Ta"), QColor(0xFF1493)}, {QString("W"),  QColor(0xFF1493)}, {QString("Re"), QColor(0xFF1493)}, {QString("Os"), QColor(0xFF1493)},
  {QString("Ir"), QColor(0xFF1493)}, {QString("Pt"), QColor(0xFF1493)}, {QString("Au"), QColor(0xDAA520)}, {QString("Hg"), QColor(0xFF1493)},
  {QString("Tl"), QColor(0xFF1493)}, {QString("Pb"), QColor(0xFF1493)}, {QString("Bi"), QColor(0xFF1493)}, {QString("Po"), QColor(0xFF1493)},
  {QString("At"), QColor(0xFF1493)}, {QString("Rn"), QColor(0xFF1493)}, {QString("Fr"), QColor(0xFF1493)}, {QString("Ra"), QColor(0xFF1493)},
  {QString("Ac"), QColor(0xFF1493)}, {QString("Th"), QColor(0xFF1493)}, {QString("Pa"), QColor(0xFF1493)}, {QString("U"),  QColor(0xFF1493)},
  {QString("Np"), QColor(0xFF1493)}, {QString("Pu"), QColor(0xFF1493)}, {QString("Am"), QColor(0xFF1493)}, {QString("Cm"), QColor(0xFF1493)},
  {QString("Bk"), QColor(0xFF1493)}, {QString("Cf"), QColor(0xFF1493)}, {QString("Es"), QColor(0xFF1493)}, {QString("Fm"), QColor(0xFF1493)},
  {QString("Md"), QColor(0xFF1493)}, {QString("No"), QColor(0xFF1493)}, {QString("Lr"), QColor(0xFF1493)}, {QString("Rf"), QColor(0xFF1493)},
  {QString("Db"), QColor(0xFF1493)}, {QString("Sg"), QColor(0xFF1493)}, {QString("Bh"), QColor(0xFF1493)}, {QString("Hs"), QColor(0xFF1493)},
  {QString("Mt"), QColor(0xFF1493)}, {QString("Ds"), QColor(0xFF1493)}, {QString("Rg"), QColor(0xFF1493)}, {QString("Cn"), QColor(0xFF1493)},
  {QString("Uut"), QColor(0xFF1493)}, {QString("Uuq"), QColor(0xFF1493)}, {QString("Uup"), QColor(0xFF1493)}, {QString("Uuh"), QColor(0xFF1493)},
  {QString("Uus"), QColor(0xFF1493)}, {QString("Uuo"), QColor(0xFF1493)}
};

std::map<QString, QColor> SKColorSet::rasmolModern =
{
  {QString("H"),  QColor(0xFFFFFF)}, {QString("He"), QColor(0xFFC0CB)}, {QString("Li"), QColor(0xB22121)}, {QString("Be"), QColor(0xFA1691)},
  {QString("B"),  QColor(0x00FF00)}, {QString("C"),  QColor(0xD3D3D3)}, {QString("N"), QColor(0x87CEE6)},  {QString("O"),  QColor(0xFF0000)},
  {QString("F"),  QColor(0xDAA520)}, {QString("Ne"), QColor(0xFA1691)}, {QString("Na"), QColor(0x0000FF)}, {QString("Mg"), QColor(0x228B22)},
  {QString("Al"), QColor(0x696969)}, {QString("Si"), QColor(0xDAA520)}, {QString("P"), QColor(0xFFAA00)},  {QString("S"),  QColor(0xFFFF00)},
  {QString("Cl"), QColor(0x00FF00)}, {QString("Ar"), QColor(0xFA1691)}, {QString("K"), QColor(0xFA1691)},  {QString("Ca"), QColor(0x696969)},
  {QString("Sc"), QColor(0xFA1691)}, {QString("Ti"), QColor(0x696969)}, {QString("V"), QColor(0xFA1691)},  {QString("Cr"), QColor(0x696969)},
  {QString("Mn"), QColor(0x696969)}, {QString("Fe"), QColor(0xFFAA00)}, {QString("Co"), QColor(0xFA1691)}, {QString("Ni"), QColor(0x802828)},
  {QString("Cu"), QColor(0x802828)}, {QString("Zn"), QColor(0x802828)}, {QString("Ga"), QColor(0xFA1691)}, {QString("Ge"), QColor(0xFA1691)},
  {QString("As"), QColor(0xFA1691)}, {QString("Se"), QColor(0xFA1691)}, {QString("Br"), QColor(0x802828)}, {QString("Kr"), QColor(0xFA1691)},
  {QString("Rb"), QColor(0xFA1691)}, {QString("Sr"), QColor(0xFA1691)}, {QString("Y"), QColor(0xFA1691)},  {QString("Zr"), QColor(0xFA1691)},
  {QString("Nb"), QColor(0xFA1691)}, {QString("Mo"), QColor(0xFA1691)}, {QString("Tc"), QColor(0xFA1691)}, {QString("Ru"), QColor(0xFA1691)},
  {QString("Rh"), QColor(0xFA1691)}, {QString("Pd"), QColor(0xFA1691)}, {QString("Ag"), QColor(0x696969)}, {QString("Cd"), QColor(0xFA1691)},
  {QString("In"), QColor(0xFA1691)}, {QString("Sn"), QColor(0xFA1691)}, {QString("Sb"), QColor(0xFA1691)}, {QString("Te"), QColor(0xFA1691)},
  {QString("I"),  QColor(0xFA1691)}, {QString("Xe"), QColor(0xFA1691)}, {QString("Cs"), QColor(0xFA1691)}, {QString("Ba"), QColor(0xFFAA00)},
  {QString("La"), QColor(0xFA1691)}, {QString("Ce"), QColor(0xFA1691)}, {QString("Pr"), QColor(0xFA1691)}, {QString("Nd"), QColor(0xFA1691)},
  {QString("Pm"), QColor(0xFA1691)}, {QString("Sm"), QColor(0xFA1691)}, {QString("Eu"), QColor(0xFA1691)}, {QString("Gd"), QColor(0xFA1691)},
  {QString("Tb"), QColor(0xFA1691)}, {QString("Dy"), QColor(0xFA1691)}, {QString("Ho"), QColor(0xFA1691)}, {QString("Er"), QColor(0xFA1691)},
  {QString("Tm"), QColor(0xFA1691)}, {QString("Yb"), QColor(0xFA1691)}, {QString("Lu"), QColor(0xFA1691)}, {QString("Hf"), QColor(0xFA1691)},
  {QString("Ta"), QColor(0xFA1691)}, {QString("W"),  QColor(0xFA1691)}, {QString("Re"), QColor(0xFA1691)}, {QString("Os"), QColor(0xFA1691)},
  {QString("Ir"), QColor(0xFA1691)}, {QString("Pt"), QColor(0xFA1691)}, {QString("Au"), QColor(0xDAA520)}, {QString("Hg"), QColor(0xFA1691)},
  {QString("Tl"), QColor(0xFA1691)}, {QString("Pb"), QColor(0xFA1691)}, {QString("Bi"), QColor(0xFA1691)}, {QString("Po"), QColor(0xFA1691)},
  {QString("At"), QColor(0xFA1691)}, {QString("Rn"), QColor(0xFA1691)}, {QString("Fr"), QColor(0xFA1691)}, {QString("Ra"), QColor(0xFA1691)},
  {QString("Ac"), QColor(0xFA1691)}, {QString("Th"), QColor(0xFA1691)}, {QString("Pa"), QColor(0xFA1691)}, {QString("U"),  QColor(0xFA1691)},
  {QString("Np"), QColor(0xFA1691)}, {QString("Pu"), QColor(0xFA1691)}, {QString("Am"), QColor(0xFA1691)}, {QString("Cm"), QColor(0xFA1691)},
  {QString("Bk"), QColor(0xFA1691)}, {QString("Cf"), QColor(0xFA1691)}, {QString("Es"), QColor(0xFA1691)}, {QString("Fm"), QColor(0xFA1691)},
  {QString("Md"), QColor(0xFA1691)}, {QString("No"), QColor(0xFA1691)}, {QString("Lr"), QColor(0xFA1691)}, {QString("Rf"), QColor(0xFA1691)},
  {QString("Db"), QColor(0xFA1691)}, {QString("Sg"), QColor(0xFA1691)}, {QString("Bh"), QColor(0xFA1691)}, {QString("Hs"), QColor(0xFA1691)},
  {QString("Mt"), QColor(0xFA1691)}, {QString("Ds"), QColor(0xFA1691)}, {QString("Rg"), QColor(0xFA1691)}, {QString("Cn"), QColor(0xFA1691)},
  {QString("Uut"), QColor(0xFA1691)}, {QString("Uuq"), QColor(0xFA1691)}, {QString("Uup"), QColor(0xFA1691)}, {QString("Uuh"), QColor(0xFA1691)},
  {QString("Uus"), QColor(0xFA1691)},{QString("Uuo"), QColor(0xFA1691)}
};

std::map<QString, QColor> SKColorSet::vesta =
{
  {QString("H"), QColor(0xFFCCCC)},  {QString("He"), QColor(0xFCE9CF)}, {QString("Li"), QColor(0x86E074)}, {QString("Be"), QColor(0x5FD87B)},
  {QString("B"), QColor(0x20A20F)},  {QString("C"),  QColor(0x814929)}, {QString("N"),  QColor(0xB0BAE6)}, {QString("O"), QColor(0xFF0300)},
  {QString("F"), QColor(0xB0BAE6)},  {QString("Ne"), QColor(0xFF38B5)}, {QString("Na"), QColor(0xFADD3D)}, {QString("Mg"), QColor(0xFC7C16)},
  {QString("Al"), QColor(0x81B3D6)}, {QString("Si"), QColor(0x1B3BFA)}, {QString("P"),  QColor(0xC19CC3)}, {QString("S"), QColor(0xFFFA00)},
  {QString("Cl"), QColor(0x32FC03)}, {QString("Ar"), QColor(0xCFFEC5)}, {QString("K"),  QColor(0xA122F7)}, {QString("Ca"), QColor(0x5B96BE)},
  {QString("Sc"), QColor(0xB663AC)}, {QString("Ti"), QColor(0x78CAFF)}, {QString("V"),  QColor(0xE51A00)}, {QString("Cr"), QColor(0x00009E)},
  {QString("Mn"), QColor(0xA9099E)}, {QString("Fe"), QColor(0xB57200)}, {QString("Co"), QColor(0x0000AF)}, {QString("Ni"), QColor(0xB8BCBE)},
  {QString("Cu"), QColor(0x2247DD)}, {QString("Zn"), QColor(0x8F9082)}, {QString("Ga"), QColor(0x9FE474)}, {QString("Ge"), QColor(0x7E6FA6)},
  {QString("As"), QColor(0x75D057)}, {QString("Se"), QColor(0x9AEF10)}, {QString("Br"), QColor(0x7F3103)}, {QString("Kr"), QColor(0xFAC1F3)},
  {QString("Rb"), QColor(0xFF0099)}, {QString("Sr"), QColor(0x00FF27)}, {QString("Y"),  QColor(0x67988E)}, {QString("Zr"), QColor(0x00FF00)},
  {QString("Nb"), QColor(0x4CB376)}, {QString("Mo"), QColor(0xB486B0)}, {QString("Tc"), QColor(0xCDAFCB)}, {QString("Ru"), QColor(0xCFB8AE)},
  {QString("Rh"), QColor(0xCED2AB)}, {QString("Pd"), QColor(0xC2C4b9)}, {QString("Ag"), QColor(0xB8BCBE)}, {QString("Cd"), QColor(0xF31FDC)},
  {QString("In"), QColor(0xD781BB)}, {QString("Sn"), QColor(0x9B8FBA)}, {QString("Sb"), QColor(0xD88350)}, {QString("Te"), QColor(0xADA252)},
  {QString("I"),  QColor(0x8F1F8B)}, {QString("Xe"), QColor(0x9BA1F8)}, {QString("Cs"), QColor(0x0FFFB9)}, {QString("Ba"), QColor(0x1AF02D)},
  {QString("La"), QColor(0x5AC449)}, {QString("Ce"), QColor(0xD1FD06)}, {QString("Pr"), QColor(0xFDE206)}, {QString("Nd"), QColor(0xFC8E07)},
  {QString("Pm"), QColor(0x0000F5)}, {QString("Sm"), QColor(0xFD067D)}, {QString("Eu"), QColor(0xFB08D5)}, {QString("Gd"), QColor(0xC004FF)},
  {QString("Tb"), QColor(0x7104FE)}, {QString("Dy"), QColor(0x3106FD)}, {QString("Ho"), QColor(0x0742FB)}, {QString("Er"), QColor(0x49733B)},
  {QString("Tm"), QColor(0x0000E0)}, {QString("Yb"), QColor(0x27FDF4)}, {QString("Lu"), QColor(0x26FDB5)}, {QString("Hf"), QColor(0xB4B459)},
  {QString("Ta"), QColor(0xB79B56)}, {QString("W"),  QColor(0x8E8A80)}, {QString("Re"), QColor(0xB3b18E)}, {QString("Os"), QColor(0xC9B179)},
  {QString("Ir"), QColor(0xC9CF73)}, {QString("Pt"), QColor(0xCCC6BF)}, {QString("Au"), QColor(0xFEB338)}, {QString("Hg"), QColor(0xD3B8CC)},
  {QString("Tl"), QColor(0x96896D)}, {QString("Pb"), QColor(0x53535B)}, {QString("Bi"), QColor(0xD230F8)}, {QString("Po"), QColor(0x0000FF)},
  {QString("At"), QColor(0x0000FF)}, {QString("Rn"), QColor(0xFFFF00)}, {QString("Fr"), QColor(QRgb(0x000000))}, {QString("Ra"), QColor(0x6eAA59)},
  {QString("Ac"), QColor(0x649E73)}, {QString("Th"), QColor(0x26FE78)}, {QString("Pa"), QColor(0x29FB35)}, {QString("U"),  QColor(0x7aA2AA)},
  {QString("Np"), QColor(0x4C4C4C)}, {QString("Pu"), QColor(0x4C4C4C)}, {QString("Am"), QColor(0x4C4C4C)}, {QString("Cm"), QColor(0x4C4C4C)},
  {QString("Bk"), QColor(0x4C4C4C)}, {QString("Cf"), QColor(0x4C4C4C)}, {QString("Es"), QColor(0x4C4C4C)}, {QString("Fm"), QColor(0x4C4C4C)},
  {QString("Md"), QColor(0x4C4C4C)}, {QString("No"), QColor(0x4C4C4C)}, {QString("Lr"), QColor(0x4C4C4C)}, {QString("Rf"), QColor(0x4C4C4C)},
  {QString("Db"), QColor(0x4C4C4C)}, {QString("Sg"), QColor(0x4C4C4C)}, {QString("Bh"), QColor(0x4C4C4C)}, {QString("Hs"), QColor(0x4C4C4C)},
  {QString("Mt"), QColor(0x4C4C4C)}, {QString("Ds"), QColor(0x4C4C4C)}, {QString("Rg"), QColor(0x4C4C4C)}, {QString("Cn"), QColor(0x4C4C4C)},
  {QString("Uut"), QColor(0x4C4C4C)}, {QString("Uuq"), QColor(0x4C4C4C)}, {QString("Uup"), QColor(0x4C4C4C)}, {QString("Uuh"), QColor(0x4C4C4C)},
  {QString("Uus"), QColor(0x4C4C4C)},{QString("Uuo"), QColor(0x4C4C4C)}
};

