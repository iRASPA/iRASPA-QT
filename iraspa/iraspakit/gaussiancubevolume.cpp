#include "gaussiancubevolume.h"
#include <QDebug>

GaussianCubeVolume::GaussianCubeVolume(): Structure()
{

}

GaussianCubeVolume::GaussianCubeVolume(std::shared_ptr<SKStructure> frame): Structure(frame)
{
    expandSymmetry();
    _atomsTreeController->setTags();

    _dimensions = frame->dimensions;
    _spacing = frame->spacing;
    _origin = frame->origin;
    _cell = frame->cell;
    _data = frame->byteData;
    _range = std::pair(frame->range.first, frame->range.second);
    qDebug() << "RANGE: " << _range;
    _average = frame->average;
    _variance = frame->variance;

    _average = frame->average;
    _variance = frame->variance;
    _adsorptionSurfaceIsoValue = frame->average;
    qDebug() << "_adsorptionSurfaceIsoValue" << _adsorptionSurfaceIsoValue;
    _adsorptionSurfaceRenderingMethod = RKEnergySurfaceType::isoSurface;
    _adsorptionVolumeTransferFunction = RKPredefinedVolumeRenderingTransferFunction::CoolWarmDiverging;
    _drawAdsorptionSurface = true;

    int largestSize = std::max({_dimensions.x,_dimensions.y,_dimensions.z});
    int k = 1;
    while(largestSize > pow(2,k))
    {
      k += 1;
    }
    _encompassingPowerOfTwoCubicGridSize = k;
    _adsorptionVolumeStepLength = 0.5 / pow(2,k);
}

GaussianCubeVolume::GaussianCubeVolume(const std::shared_ptr<Object> object): Structure(object)
{
  if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(object))
  {
    if(!atomViewer->isFractional())
    {
      convertAsymmetricAtomsToFractional();
      expandSymmetry();
    }
    _atomsTreeController->setTags();
    reComputeBoundingBox();
    computeBonds();
  }
}

std::shared_ptr<Object> GaussianCubeVolume::shallowClone()
{
  return std::make_shared<GaussianCubeVolume>(static_cast<const GaussianCubeVolume&>(*this));
}

// MARK: Bounding box
// =====================================================================

SKBoundingBox GaussianCubeVolume::boundingBox() const
{
  double3 minimumReplica = _cell->minimumReplicas();
  double3 maximumReplica = _cell->maximumReplicas();

  double3 c0 = _cell->unitCell() * double3(minimumReplica);
  double3 c1 = _cell->unitCell() * double3(maximumReplica.x+1, minimumReplica.y,   minimumReplica.z);
  double3 c2 = _cell->unitCell() * double3(maximumReplica.x+1, maximumReplica.y+1, minimumReplica.z);
  double3 c3 = _cell->unitCell() * double3(minimumReplica.x,   maximumReplica.y+1, minimumReplica.z);
  double3 c4 = _cell->unitCell() * double3(minimumReplica.x,   minimumReplica.y,   maximumReplica.z+1);
  double3 c5 = _cell->unitCell() * double3(maximumReplica.x+1, minimumReplica.y,   maximumReplica.z+1);
  double3 c6 = _cell->unitCell() * double3(maximumReplica.x+1, maximumReplica.y+1, maximumReplica.z+1);
  double3 c7 = _cell->unitCell() * double3(minimumReplica.x,   maximumReplica.y+1, maximumReplica.z+1);

  double valuesX[8] = {c0.x, c1.x, c2.x, c3.x, c4.x, c5.x, c6.x, c7.x};
  double valuesY[8] = {c0.y, c1.y, c2.y, c3.y, c4.y, c5.y, c6.y, c7.y};
  double valuesZ[8] = {c0.z, c1.z, c2.z, c3.z, c4.z, c5.z, c6.z, c7.z};

  double3 minimum = double3(*std::min_element(valuesX,valuesX+8),
                            *std::min_element(valuesY,valuesY+8),
                            *std::min_element(valuesZ,valuesZ+8));

  double3 maximum = double3(*std::max_element(valuesX,valuesX+8),
                            *std::max_element(valuesY,valuesY+8),
                            *std::max_element(valuesZ,valuesZ+8));

  return SKBoundingBox(minimum,maximum);
}

void GaussianCubeVolume::reComputeBoundingBox()
{
  SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  _cell->setBoundingBox(boundingBox);
}

// MARK: Rendering
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> GaussianCubeVolume::renderAtoms() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  std::vector<RKInPerInstanceAttributesAtoms> atomData = std::vector<RKInPerInstanceAttributesAtoms>();

  uint32_t asymmetricAtomIndex = 0;
  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      bool isVisible = atom->isVisible();
      double w = isVisible ? 1.0 : -1.0;

      QColor color = atom->color();

      float4 ambient = float4(color.redF(),color.greenF(),color.blueF(),color.alphaF());
      float4 diffuse = float4(color.redF(),color.greenF(),color.blueF(),color.alphaF());

      float4 specular = float4(1.0,1.0,1.0,1.0);

      double radius = atom->drawRadius(); // * atom->occupancy();
      float4 scale = float4(radius,radius,radius,1.0);

      for(std::shared_ptr<SKAtomCopy> copy : atom->copies())
      {
        if(copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          double3 copyPosition = double3::flip(copy->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
          for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
          {
            for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
            {
              for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
              {
                float4 position = float4(_cell->unitCell() * (copyPosition + double3(k1,k2,k3)), w);

                RKInPerInstanceAttributesAtoms atom1 = RKInPerInstanceAttributesAtoms(position, ambient, diffuse, specular, scale, asymmetricAtomIndex);
                atomData.push_back(atom1);
              }
            }
          }
        }
      }
    }
    asymmetricAtomIndex++;
  }

  return atomData;
}

std::vector<RKInPerInstanceAttributesBonds> GaussianCubeVolume::renderInternalBonds() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<RKInPerInstanceAttributesBonds> data =  std::vector<RKInPerInstanceAttributesBonds>();

  const std::vector<std::shared_ptr<SKAsymmetricBond>> asymmetricBonds = _bondSetController->arrangedObjects();

  uint32_t asymmetricBondIndex = 0;
  for(std::shared_ptr<SKAsymmetricBond> asymmetricBond : asymmetricBonds)
  {
    bool isVisble = asymmetricBond->isVisible() && asymmetricBond->atom1()->isVisible()  && asymmetricBond->atom2()->isVisible();

    if(isVisble)
    {
      const std::vector<std::shared_ptr<SKBond>> bonds = asymmetricBond->copies();
      for(std::shared_ptr<SKBond> bond : bonds)
      {
        if(bond->atom1()->type() == SKAtomCopy::AtomCopyType::copy && bond->atom2()->type() == SKAtomCopy::AtomCopyType::copy && bond->boundaryType() == SKBond::BoundaryType::internal)
        {
          QColor color1 = bond->atom1()->parent()->color();
          QColor color2 = bond->atom2()->parent()->color();

          double3 copyPosition1 = double3::flip(bond->atom1()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
          double3 copyPosition2 = double3::flip(bond->atom2()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

          for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
          {
            for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
            {
              for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
              {
                double3 pos1 = _cell->unitCell() * (copyPosition1 + double3(k1,k2,k3));
                double3 pos2 = _cell->unitCell() * (copyPosition2 + double3(k1,k2,k3));
                double bondLength = (pos2-pos1).length();
                double drawRadius1 = bond->atom1()->parent()->drawRadius()/bondLength;
                double drawRadius2 = bond->atom2()->parent()->drawRadius()/bondLength;

                RKInPerInstanceAttributesBonds bondData = RKInPerInstanceAttributesBonds(
                            float4(pos1,1.0),
                            float4(pos2,1.0),
                            float4(color1.redF(),color1.greenF(),color1.blueF(),color1.alphaF()),
                            float4(color2.redF(),color2.greenF(),color2.blueF(),color2.alphaF()),
                            float4(drawRadius1, std::min(bond->atom1()->parent()->occupancy(),bond->atom2()->parent()->occupancy()), drawRadius2, drawRadius1/drawRadius2),
                            asymmetricBondIndex,
                            static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType()));
                data.push_back(bondData);
              }
            }
          }
        }
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}

std::vector<RKInPerInstanceAttributesBonds> GaussianCubeVolume::renderExternalBonds() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 contentShift = _cell->contentShift();
  bool3 contentFlip = _cell->contentFlip();

  std::vector<RKInPerInstanceAttributesBonds> data =  std::vector<RKInPerInstanceAttributesBonds>();

  const std::vector<std::shared_ptr<SKAsymmetricBond>> asymmetricBonds = _bondSetController->arrangedObjects();

  uint32_t asymmetricBondIndex = 0;
  for(std::shared_ptr<SKAsymmetricBond> asymmetricBond : asymmetricBonds)
  {
    bool isVisible = asymmetricBond->isVisible() && asymmetricBond->atom1()->isVisible()  && asymmetricBond->atom2()->isVisible();

    if(isVisible)
    {
      const std::vector<std::shared_ptr<SKBond>> bonds = asymmetricBond->copies();
      for(std::shared_ptr<SKBond> bond : bonds)
      {
        if(bond->boundaryType() == SKBond::BoundaryType::external)
        {
          QColor color1 = bond->atom1()->parent()->color();
          QColor color2 = bond->atom2()->parent()->color();

          double3 copyPosition1 = double3::flip(bond->atom1()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;
          double3 copyPosition2 = double3::flip(bond->atom2()->position(), contentFlip, double3(1.0,1.0,1.0)) + contentShift;

          for(int k1=minimumReplicaX;k1<=maximumReplicaX;k1++)
          {
            for(int k2=minimumReplicaY;k2<=maximumReplicaY;k2++)
            {
              for(int k3=minimumReplicaZ;k3<=maximumReplicaZ;k3++)
              {
                double3 frac_pos1 = copyPosition1 + double3(k1,k2,k3);
                double3 frac_pos2 = copyPosition2 + double3(k1,k2,k3);

                double3 dr = frac_pos2 - frac_pos1;

                dr.x -= rint(dr.x);
                dr.y -= rint(dr.y);
                dr.z -= rint(dr.z);

                double3 pos1 = _cell->unitCell() * frac_pos1;  //+ bond.atom1.asymmetricParentAtom.displacement
                double3 pos2 = _cell->unitCell() * frac_pos2;  // + bond.atom2.asymmetricParentAtom.displacement

                dr = _cell->unitCell() * dr;
                double bondLength = dr.length();

                double drawRadius1 = bond->atom1()->parent()->drawRadius()/bondLength;
                double drawRadius2 = bond->atom2()->parent()->drawRadius()/bondLength;

                RKInPerInstanceAttributesBonds bondData = RKInPerInstanceAttributesBonds(
                              float4(pos1,1.0),
                              float4(pos1 + dr,1.0),
                              float4(color1.redF(),color1.greenF(),color1.blueF(),color1.alphaF()),
                              float4(color2.redF(),color2.greenF(),color2.blueF(),color2.alphaF()),
                              float4(drawRadius1, std::min(bond->atom1()->parent()->occupancy(),bond->atom2()->parent()->occupancy()), drawRadius2, drawRadius1/drawRadius2),
                              asymmetricBondIndex,
                              static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType()));
                data.push_back(bondData);

                RKInPerInstanceAttributesBonds bondData2 = RKInPerInstanceAttributesBonds(
                              float4(pos2,1.0),
                              float4(pos2 - dr,1.0),
                              float4(color2.redF(),color2.greenF(),color2.blueF(),color2.alphaF()),
                              float4(color1.redF(),color1.greenF(),color1.blueF(),color1.alphaF()),
                              float4(drawRadius2, std::min(bond->atom2()->parent()->occupancy(),bond->atom1()->parent()->occupancy()), drawRadius1, drawRadius2/drawRadius1),
                              asymmetricBondIndex,
                              static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType()));
                data.push_back(bondData2);
              }
            }
          }
        }
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}

// MARK: Symmetry
// =====================================================================

void GaussianCubeVolume::expandSymmetry()
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

  for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom = node->representedObject())
    {
      std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};

      std::shared_ptr<SKAtomCopy> newAtom = std::make_shared<SKAtomCopy>(asymmetricAtom, asymmetricAtom->position());
      newAtom->setType(SKAtomCopy::AtomCopyType::copy);
      atomCopies.push_back(newAtom);

      asymmetricAtom->setCopies(atomCopies);
    }
  }
}

void GaussianCubeVolume::expandSymmetry(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom)
{
  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = std::vector<std::shared_ptr<SKAtomCopy>>{};

  std::shared_ptr<SKAtomCopy> newAtom = std::make_shared<SKAtomCopy>(asymmetricAtom, asymmetricAtom->position());
  newAtom->setType(SKAtomCopy::AtomCopyType::copy);
  atomCopies.push_back(newAtom);

  asymmetricAtom->setCopies(atomCopies);
}

std::vector<RKInPerInstanceAttributesAtoms> GaussianCubeVolume::renderUnitCellSpheres() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 boundingBoxWidths = _cell->boundingBox().widths();
  double scaleFactor = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});

  std::vector<RKInPerInstanceAttributesAtoms> data = std::vector<RKInPerInstanceAttributesAtoms>();

  uint32_t asymmetricBondIndex=0;
  for(int k1=minimumReplicaX;k1<=maximumReplicaX+1;k1++)
  {
    for(int k2=minimumReplicaY;k2<=maximumReplicaY+1;k2++)
    {
      for(int k3=minimumReplicaZ;k3<=maximumReplicaZ+1;k3++)
      {
        double3 position = _cell->unitCell() * double3(k1,k2,k3); // + origin();
        float4 ambient = float4(1.0f,1.0f,1.0f,1.0f);
        float4 diffuse = float4(1.0f,1.0f,1.0f,1.0f);
        float4 specular = float4(1.0f,1.0f,1.0f,1.0f);
        float4 scale = float4(scaleFactor,scaleFactor,scaleFactor,1.0f);
        RKInPerInstanceAttributesAtoms sphere = RKInPerInstanceAttributesAtoms(float4(position,1.0), ambient, diffuse, specular, scale, asymmetricBondIndex);
        data.push_back(sphere);
      }
    }
    asymmetricBondIndex++;
  }

  return data;
}

std::vector<RKInPerInstanceAttributesBonds> GaussianCubeVolume::renderUnitCellCylinders() const
{
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();

  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  double3 boundingBoxWidths = _cell->boundingBox().widths();
  double scaleFactor = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});

  std::vector<RKInPerInstanceAttributesBonds> data = std::vector<RKInPerInstanceAttributesBonds>();

  for(int k1=minimumReplicaX;k1<=maximumReplicaX+1;k1++)
  {
    for(int k2=minimumReplicaY;k2<=maximumReplicaY+1;k2++)
    {
      for(int k3=minimumReplicaZ;k3<=maximumReplicaZ+1;k3++)
      {
        if(k1<=maximumReplicaX)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1+1,k2,k3); // + origin();
          float4 scale = float4(scaleFactor,1.0f,scaleFactor,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }

        if(k2<=maximumReplicaY)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1,k2+1,k3); // + origin();
          float4 scale = float4(scaleFactor,1.0f,scaleFactor,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }

        if(k3<=maximumReplicaZ)
        {
          double3 position1 = _cell->unitCell() * double3(k1,k2,k3); // + origin();
          double3 position2 = _cell->unitCell() * double3(k1,k2,k3+1); // + origin();
          float4 scale = float4(scaleFactor,1.0f,scaleFactor,1.0f);
          RKInPerInstanceAttributesBonds cylinder =
                  RKInPerInstanceAttributesBonds(float4(position1,1.0),
                                                 float4(position2,1.0),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 float4(1.0f,1.0f,1.0f,1.0f),
                                                 scale,0,0);
          data.push_back(cylinder);
        }
      }
    }
  }

  return data;
}

std::vector<float> GaussianCubeVolume::gridData()
{
  int encompassingcubicsize = pow(2, _encompassingPowerOfTwoCubicGridSize);
  auto *p = reinterpret_cast<float*>(_data.data());
  std::vector<float> copiedData(encompassingcubicsize*encompassingcubicsize*encompassingcubicsize);
  for(int x=0;x<_dimensions.x;x++)
  {
    for(int y=0;y<_dimensions.y;y++)
    {
      for(int z=0;z<_dimensions.z;z++)
      {
        int index = int(x+encompassingcubicsize*y+z*encompassingcubicsize*encompassingcubicsize);
        copiedData[index] = p[x + _dimensions.x*y + z*_dimensions.x*_dimensions.y];
      }
    }
  }

  return copiedData;
}
std::vector<float4> GaussianCubeVolume::gridValueAndGradientData()
{
   auto *p = reinterpret_cast<float*>(_data.data());
   std::vector<float> energyData(_dimensions.x*_dimensions.y*_dimensions.z);

   // normalize data
   for(int i=0;i<_dimensions.x*_dimensions.y*_dimensions.z;i++)
   {
     energyData[i] = (p[i] - _range.first) / (_range.second - _range.first);
   };

  int encompassingcubicsize = pow(2,_encompassingPowerOfTwoCubicGridSize);
  std::vector<float4> gradientData(encompassingcubicsize*encompassingcubicsize*encompassingcubicsize);

  for(int x=0;x<_dimensions.x;x++)
  {
    for(int y=0;y<_dimensions.y;y++)
    {
      for(int z=0;z<_dimensions.z;z++)
      {
        float value = energyData[x+_dimensions.x*y+z*_dimensions.x*_dimensions.y];

        // x
        int xi = (int)(x + 0.5f);
        float xf = x + 0.5f - xi;
        float xd0 = energyData[int(((xi-1 + _dimensions.x) % _dimensions.x)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float xd1 = energyData[int((xi)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float xd2 = energyData[int(((xi+1 + _dimensions.x) % _dimensions.x)+y*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float gx = (xd1 - xd0) * (1.0f - xf) + (xd2 - xd1) * xf; // lerp

        // y
        int yi = (int)(y + 0.5f);
        float yf = y + 0.5f - yi;
        float yd0 = energyData[int(x + ((yi-1+_dimensions.y) % _dimensions.y)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float yd1 = energyData[int(x + (yi)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float yd2 = energyData[int(x + ((yi+1+_dimensions.y) % _dimensions.y)*_dimensions.x+z*_dimensions.x*_dimensions.y)];
        float gy = (yd1 - yd0) * (1.0f - yf) + (yd2 - yd1) * yf; // lerp

        // z
        int zi = (int)(z + 0.5f);
        float zf = z + 0.5f - zi;
        float zd0 = energyData[int(x+y*_dimensions.x+((zi-1+_dimensions.z) % _dimensions.z)*_dimensions.x*_dimensions.y)];
        float zd1 = energyData[int(x+y*_dimensions.x+(zi)*_dimensions.x*_dimensions.y)];
        float zd2 = energyData[int(x+y*_dimensions.x+((zi+1+_dimensions.z) % _dimensions.z)*_dimensions.x*_dimensions.y)];
        float gz =  (zd1 - zd0) * (1.0f - zf) + (zd2 - zd1) * zf; // lerp

        int index = int(x+encompassingcubicsize*y+z*encompassingcubicsize*encompassingcubicsize);
        gradientData[index] = float4(value, gx, gy, gz);
      }
    }
  }
  return gradientData;
}

// MARK: bond-computations
// =====================================================================

double GaussianCubeVolume::bondLength(std::shared_ptr<SKBond> bond) const
{
  double3 pos1 = bond->atom1()->position();
  double3 pos2 = bond->atom2()->position();
  double3 ds = pos2 - pos1;
  ds.x -= floor(ds.x + 0.5);
  ds.y -= floor(ds.y + 0.5);
  ds.z -= floor(ds.z + 0.5);
  return (_cell->unitCell() * ds).length();
}

double3 GaussianCubeVolume::bondVector(std::shared_ptr<SKBond> bond) const
{
  double3 pos1 = bond->atom1()->position();
  double3 pos2 = bond->atom2()->position();
  double3 ds = pos2 - pos1;
  ds.x -= floor(ds.x + 0.5);
  ds.y -= floor(ds.y + 0.5);
  ds.z -= floor(ds.z + 0.5);
  return ds;
}

std::pair<double3, double3> GaussianCubeVolume::computeChangedBondLength(std::shared_ptr<SKBond> bond, double bondLength) const
{
  double3 pos1 = bond->atom1()->position();
  std::shared_ptr<SKAsymmetricAtom> asymmetricAtom1 = bond->atom1()->parent();

  double3 pos2 = bond->atom2()->position();
  std::shared_ptr<SKAsymmetricAtom> asymmetricAtom2 = bond->atom2()->parent();

  double oldBondLength = this->bondLength(bond);

  double3 bondVector = this->bondVector(bond).normalise();

  bool isAllFixed1 = asymmetricAtom1->isFixed().x && asymmetricAtom1->isFixed().y && asymmetricAtom1->isFixed().z;
  bool isAllFixed2 = asymmetricAtom2->isFixed().x && asymmetricAtom2->isFixed().y && asymmetricAtom2->isFixed().z;

  if(!isAllFixed1 && !isAllFixed2)
  {
    double3 newPos1 = _cell->convertToFractional(_cell->convertToCartesian(pos1) - 0.5 * (bondLength - oldBondLength) * bondVector);
    double3 newPos2 = _cell->convertToFractional(_cell->convertToCartesian(pos2) + 0.5 * (bondLength - oldBondLength) * bondVector);
    return std::make_pair(newPos1, newPos2);
  }
  else if(isAllFixed1 && !isAllFixed2)
  {
    double3 newPos2 = _cell->convertToFractional(_cell->convertToCartesian(pos1) + bondLength * bondVector);
    return std::make_pair(pos1, newPos2);
  }
  else if(!isAllFixed1 && isAllFixed2)
  {
    double3 newPos1 = _cell->convertToFractional(_cell->convertToCartesian(pos2) - bondLength * bondVector);
    return std::make_pair(newPos1, pos2);
  }

  return std::make_pair(pos1,pos2);
}

/*
std::vector<double2> Crystal::potentialParameters() const
{

    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    std::vector<double2> atomData = std::vector<double2>();

    for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
    {
       if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
       {
         for(std::shared_ptr<SKAtomCopy> copy : atom->copies())
         {
           if(copy->type() == SKAtomCopy::AtomCopyType::copy)
           {
             double2 parameter = atom->potentialParameters();
             atomData.push_back(parameter);
           }
         }
       }
    }

    return atomData;
}

*/

void GaussianCubeVolume::computeBonds()
{
  std::vector<std::shared_ptr<SKAtomCopy>> copies =  _atomsTreeController->atomCopies();

  std::vector<std::shared_ptr<SKBond>> bonds;

  for(size_t i=0;i<copies.size();i++)
  {
    copies[i]->setType(SKAtomCopy::AtomCopyType::copy);
    double3 posA = _cell->unitCell() * copies[i]->position();
    int elementIdentifierA = copies[i]->parent()->elementIdentifier();
    double covalentRadiusA = PredefinedElements::predefinedElements[elementIdentifierA]._covalentRadius;
    for(size_t j=i+1;j<copies.size();j++)
    {
      double3 posB = _cell->unitCell() * copies[j]->position();
      int elementIdentifierB = copies[j]->parent()->elementIdentifier();
      double covalentRadiusB = PredefinedElements::predefinedElements[elementIdentifierB]._covalentRadius;

      double3 separationVector = (posA-posB);
      double3 periodicSeparationVector = _cell->applyUnitCellBoundaryCondition(separationVector);
      double bondCriteria = covalentRadiusA + covalentRadiusB + 0.56;
      double bondLength = periodicSeparationVector.length();
      if(bondLength < bondCriteria)
      {
        if(bondLength<0.1)
        {
          // a duplicate when: (a) both occupancies are 1.0, or (b) when they are the same asymmetric type
          if(!(copies[i]->parent()->occupancy() < 1.0 || copies[j]->parent()->occupancy() < 1.0) || copies[i]->asymmetricIndex() == copies[j]->asymmetricIndex())
          {
            copies[i]->setType(SKAtomCopy::AtomCopyType::duplicate);
          }
        }
        if (separationVector.length() > bondCriteria)
        {
          std::shared_ptr<SKBond> bond = std::make_shared<SKBond>(copies[i],copies[j], SKBond::BoundaryType::external);
          bonds.push_back(bond);
        }
        else
        {
          std::shared_ptr<SKBond> bond = std::make_shared<SKBond>(copies[i],copies[j], SKBond::BoundaryType::internal);
          bonds.push_back(bond);
        }
      }
    }
  }

  std::vector<std::shared_ptr<SKBond>> filtered_bonds;
  std::copy_if (bonds.begin(), bonds.end(), std::back_inserter(filtered_bonds), [](std::shared_ptr<SKBond> i){return i->atom1()->type() == SKAtomCopy::AtomCopyType::copy && i->atom2()->type() == SKAtomCopy::AtomCopyType::copy;} );
  _bondSetController->setBonds(filtered_bonds);
}


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<GaussianCubeVolume> &volume)
{
  stream << volume->_versionNumber;

  stream << volume->_dimensions;
  stream << volume->_spacing;
  stream << volume->_range.first;
  stream << volume->_range.second;
  stream << volume->_data;
  stream << volume->_average;
  stream << volume->_variance;

  stream << qint64(0x6f6b6199);

  // handle super class
  stream << std::static_pointer_cast<Structure>(volume);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<GaussianCubeVolume> &volume)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "GaussianCubeVolume");
  }

  stream >> volume->_dimensions;
  stream >> volume->_spacing;
  stream >> volume->_range.first;
  stream >> volume->_range.second;
  stream >> volume->_data;
  stream >> volume->_average;
  stream >> volume->_variance;

  qint64 magicNumber;
  stream >> magicNumber;
  if(magicNumber != qint64(0x6f6b6199))
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "GaussianCubeVolume invalid magic-number");
  }

  std::shared_ptr<Structure> object = std::static_pointer_cast<Structure>(volume);
  stream >> object;

  return stream;
}
