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

#include "sksecondarystructure.h"
#include "skstridesecondarystructureassigner.h"
#include "skdsssecondarystructureassigner.h"
#include "skdsspsecondarystructureassigner.h"
#include "skpseasecondarystructureassigner.h"
#include "sksequoiasecondarystructureassigner.h"
#include "sksegnosecondarystructureassigner.h"

std::vector<SKSecondaryStructureType> SKSecondaryStructureAssigner::assign(const SKStrideBackboneChain &chain,
                                                                           SKSecondaryStructureAssignmentMethod method)
{
  switch (method)
  {
  case SKSecondaryStructureAssignmentMethod::stride:
    return SKStrideSecondaryStructureAssigner::assign(chain);
  case SKSecondaryStructureAssignmentMethod::dss:
    return SKDssSecondaryStructureAssigner::assign(chain);
  case SKSecondaryStructureAssignmentMethod::dssp:
    return SKDsspSecondaryStructureAssigner::assign(chain);
  case SKSecondaryStructureAssignmentMethod::psea:
    return SKPSeaSecondaryStructureAssigner::assign(chain);
  case SKSecondaryStructureAssignmentMethod::sequoia:
    return SKSequoiaSecondaryStructureAssigner::assign(chain);
  case SKSecondaryStructureAssignmentMethod::segno:
    return SKSegnoSecondaryStructureAssigner::assign(chain);
  }
  return {};
}
