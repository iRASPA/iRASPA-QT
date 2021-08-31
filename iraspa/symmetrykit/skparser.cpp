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

#include "skparser.h"

#include <qmath.h>

SKParser::SKParser(): _a(20.0), _b(20.0), _c(20.0), _alpha(90.0 * M_PI / 180.0), _beta(90.0 * M_PI / 180.0), _gamma(90.0 * M_PI / 180.0)
{

}

SKParser::~SKParser()
{
  // Compulsory virtual destructor definition
}


std::vector<std::vector<std::shared_ptr<SKStructure>>> SKParser::movies()
{
  return _movies;
}

std::vector<std::tuple<double3, int, double> > SKParser::firstTestFrame()
{
  std::vector<std::tuple<double3, int, double> > atoms{};

  for(const std::vector<std::shared_ptr<SKStructure>> &movie: _movies)
  {
    for(const std::shared_ptr<SKStructure> &structure: movie)
    {
      for(const std::shared_ptr<SKAsymmetricAtom> &atom: structure->atoms)
      {
         std::tuple<double3, int, double> atomTuple = std::make_tuple(atom->position(), atom->elementIdentifier(), 1.0);
         atoms.push_back(atomTuple);
      }
    }
  }
  return atoms;
}
