#include "primitive.h"
#include "crystal.h"
#include "crystalcylinderprimitive.h"
#include "crystalellipsoidprimitive.h"
#include "crystalpolygonalprismprimitive.h"

Primitive::Primitive()
{

}

Primitive::Primitive(const Primitive &primitive): Structure(primitive)
{
}

Primitive::Primitive(std::shared_ptr<Structure> s): Structure(s)
{
  _cell = std::make_shared<SKCell>(*s->cell());

  if(dynamic_cast<Crystal*>(s.get()) ||
     dynamic_cast<CrystalEllipsoidPrimitive*>(s.get()) ||
     dynamic_cast<CrystalCylinderPrimitive*>(s.get()) ||
     dynamic_cast<CrystalPolygonalPrismPrimitive*>(s.get()))
  {
    convertAsymmetricAtomsToFractional();
  }

  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
  computeBonds();
}


double Primitive::primitiveSelectionFrequency() const
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _primitiveSelectionWorleyNoise3DFrequency;
  case RKSelectionStyle::striped:
      return _primitiveSelectionStripesFrequency;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
     return 0.0;
  }
  return 0.0;
}

void Primitive::setPrimitiveSelectionFrequency(double value)
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _primitiveSelectionWorleyNoise3DFrequency = value;
    break;
  case RKSelectionStyle::striped:
    _primitiveSelectionStripesFrequency = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}

double Primitive::primitiveSelectionDensity() const
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _primitiveSelectionWorleyNoise3DJitter;
  case RKSelectionStyle::striped:
    return _primitiveSelectionStripesDensity;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    return 0.0;
  }
  return 0.0;
}
void Primitive::setPrimitiveSelectionDensity(double value)
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _primitiveSelectionWorleyNoise3DJitter = value;
    break;
  case RKSelectionStyle::striped:
    _primitiveSelectionStripesDensity = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Primitive> &primitive)
{
  stream << primitive->_versionNumber;

  stream << primitive->_primitiveTransformationMatrix;
  stream << primitive->_primitiveOrientation;
  stream << primitive->_primitiveRotationDelta;
  stream << primitive->_primitiveOpacity;
  stream << primitive->_primitiveIsCapped;
  stream << primitive->_primitiveIsFractional;
  stream << primitive->_primitiveNumberOfSides;
  stream << primitive->_primitiveThickness;

  stream << primitive->_primitiveHue;
  stream << primitive->_primitiveSaturation;
  stream << primitive->_primitiveValue;

  stream << static_cast<typename std::underlying_type<RKSelectionStyle>::type>(primitive->_primitiveSelectionStyle);
  stream << primitive->_primitiveSelectionStripesDensity;
  stream << primitive->_primitiveSelectionStripesFrequency;
  stream << primitive->_primitiveSelectionWorleyNoise3DFrequency;
  stream << primitive->_primitiveSelectionWorleyNoise3DJitter;
  stream << primitive->_primitiveSelectionScaling;
  stream << primitive->_primitiveSelectionIntensity;

  stream << primitive->_primitiveFrontSideHDR;
  stream << primitive->_primitiveFrontSideHDRExposure;
  stream << primitive->_primitiveFrontSideAmbientColor;
  stream << primitive->_primitiveFrontSideDiffuseColor;
  stream << primitive->_primitiveFrontSideSpecularColor;
  stream << primitive->_primitiveFrontSideDiffuseIntensity;
  stream << primitive->_primitiveFrontSideAmbientIntensity;
  stream << primitive->_primitiveFrontSideSpecularIntensity;
  stream << primitive->_primitiveFrontSideShininess;

  stream << primitive->_primitiveBackSideHDR;
  stream << primitive->_primitiveBackSideHDRExposure;
  stream << primitive->_primitiveBackSideAmbientColor;
  stream << primitive->_primitiveBackSideDiffuseColor;
  stream << primitive->_primitiveBackSideSpecularColor;
  stream << primitive->_primitiveBackSideDiffuseIntensity;
  stream << primitive->_primitiveBackSideAmbientIntensity;
  stream << primitive->_primitiveBackSideSpecularIntensity;
  stream << primitive->_primitiveBackSideShininess;


  // handle super class
  stream << std::static_pointer_cast<Structure>(primitive);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Primitive> &primitive)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > primitive->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "CrystalCylinderPrimitive");
  }

  stream >> primitive->_primitiveTransformationMatrix;
  stream >> primitive->_primitiveOrientation;
  stream >> primitive->_primitiveRotationDelta;

  stream >> primitive->_primitiveOpacity;
  stream >> primitive->_primitiveIsCapped;
  stream >> primitive->_primitiveIsFractional;
  stream >> primitive->_primitiveNumberOfSides;
  stream >> primitive->_primitiveThickness;

  stream >> primitive->_primitiveHue;
  stream >> primitive->_primitiveSaturation;
  stream >> primitive->_primitiveValue;

  qint64 primitiveSelectionStyle;
  stream >> primitiveSelectionStyle;
  primitive->_primitiveSelectionStyle = RKSelectionStyle(primitiveSelectionStyle);
  stream >> primitive->_primitiveSelectionStripesDensity;
  stream >> primitive->_primitiveSelectionStripesFrequency;
  stream >> primitive->_primitiveSelectionWorleyNoise3DFrequency;
  stream >> primitive->_primitiveSelectionWorleyNoise3DJitter;
  stream >> primitive->_primitiveSelectionScaling;
  stream >> primitive->_primitiveSelectionIntensity;

  stream >> primitive->_primitiveFrontSideHDR;
  stream >> primitive->_primitiveFrontSideHDRExposure;
  stream >> primitive->_primitiveFrontSideAmbientColor;
  stream >> primitive->_primitiveFrontSideDiffuseColor;
  stream >> primitive->_primitiveFrontSideSpecularColor;
  stream >> primitive->_primitiveFrontSideDiffuseIntensity;
  stream >> primitive->_primitiveFrontSideAmbientIntensity;
  stream >> primitive->_primitiveFrontSideSpecularIntensity;
  stream >> primitive->_primitiveFrontSideShininess;

  stream >> primitive->_primitiveBackSideHDR;
  stream >> primitive->_primitiveBackSideHDRExposure;
  stream >> primitive->_primitiveBackSideAmbientColor;
  stream >> primitive->_primitiveBackSideDiffuseColor;
  stream >> primitive->_primitiveBackSideSpecularColor;
  stream >> primitive->_primitiveBackSideDiffuseIntensity;
  stream >> primitive->_primitiveBackSideAmbientIntensity;
  stream >> primitive->_primitiveBackSideSpecularIntensity;
  stream >> primitive->_primitiveBackSideShininess;

  std::shared_ptr<Structure> structure = std::static_pointer_cast<Structure>(primitive);
  stream >> structure;

  return stream;
}

