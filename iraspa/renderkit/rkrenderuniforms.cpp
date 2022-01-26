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

#include "rkrenderuniforms.h"
#include "rkrenderkitprotocols.h"
#include "mathkit.h"

RKTransformationUniforms::RKTransformationUniforms(double4x4 projectionMatrix, double4x4 modelViewMatrix, double4x4 modelMatrix, double4x4 viewMatrix, double4x4 axesProjectionMatrix, double4x4 axesModelViewMatrix, bool isOrthographic, double bloomLevel, double bloomPulse, int multiSampling)
{
    double4x4 currentMvpMatrix = projectionMatrix * modelViewMatrix;
    this->projectionMatrix = float4x4(projectionMatrix);
    this->modelViewMatrix = float4x4(modelViewMatrix);
    this->mvpMatrix = float4x4(currentMvpMatrix);
    this->shadowMatrix = float4x4(currentMvpMatrix);

    this->projectionMatrixInverse = float4x4(double4x4::inverse(projectionMatrix));
    this->viewMatrixInverse = float4x4(double4x4::inverse(modelViewMatrix));

    this->axesProjectionMatrix = float4x4(axesProjectionMatrix);
    this->axesViewMatrix = float4x4(axesModelViewMatrix);
    this->axesMvpMatrix = float4x4(axesProjectionMatrix * axesModelViewMatrix);

    double4x4 modifiedCameraViewMatrix = viewMatrix;
    if(isOrthographic)
    {
      modifiedCameraViewMatrix.wz = -10000.0;
    }
    double4 camera_position = double4x4::inverse(modifiedCameraViewMatrix * modelMatrix) * double4(0.0, 0.0, 0.0, 1.0);
    this->cameraPosition = float4(camera_position);

    double3x3 currentNormalMatrix = double3x3::transpose(double3x3::inverse(double3x3(modelViewMatrix)));
    this->normalMatrix = float4x4(double4x4(currentNormalMatrix));
    this->bloomLevel = float(bloomLevel);
    this->bloomPulse = float(bloomPulse);
    this->numberOfMultiSamplePoints = float(multiSampling);
};


RKStructureUniforms::RKStructureUniforms(size_t sceneIdentifier, size_t movieIdentifier, std::shared_ptr<RKRenderObject> structure)
{
  this->sceneIdentifier = int32_t(sceneIdentifier);
  this->MovieIdentifier = int32_t(movieIdentifier);

  if (RKRenderObject* renderStructure = dynamic_cast<RKRenderObject*>(structure.get()))
  {
    SKBoundingBox boundingbox = renderStructure->cell()->boundingBox();
    double3 centerOfRotation = boundingbox.center();

    double4x4 currentModelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(double4x4(renderStructure->orientation()),centerOfRotation, renderStructure->origin());

    this->modelMatrix = float4x4(currentModelMatrix);
    this->inverseModelMatrix = float4x4(double4x4::inverse(currentModelMatrix));

    double3x3 box = renderStructure->cell()->box();

    double3 numberOfReplicas = renderStructure->cell()->numberOfReplicas();
    this->numberOfReplicas = float4(numberOfReplicas.x,numberOfReplicas.y,numberOfReplicas.z,1.0);

    this->boxMatrix = float4x4(box);
    this->inverseBoxMatrix = float4x4(double4x4::inverse(box));
    double3 shift = renderStructure->cell()->unitCell() * double3(renderStructure->cell()->minimumReplicas());
    this->boxMatrix[3][0] = float(shift.x);
    this->boxMatrix[3][1] = float(shift.y);
    this->boxMatrix[3][2] = float(shift.z);

    if (RKRenderLocalAxesSource* renderStructure = dynamic_cast<RKRenderLocalAxesSource*>(structure.get()))
    {
      double3 offset = renderStructure->renderLocalAxes().offset();

      switch(renderStructure->renderLocalAxes().position())
      {
      case RKLocalAxes::Position::none:
        this->localAxisPosition = float4(0.0,0.0,0.0,1.0) + float4(offset.x,offset.y,offset.z,0.0);
        break;
      case RKLocalAxes::Position::origin:
        this->localAxisPosition = float4(0.0,0.0,0.0,1.0) + float4(offset.x,offset.y,offset.z,0.0);
        break;
      case RKLocalAxes::Position::center:
        this->localAxisPosition =  box * float4(0.5,0.5,0.5,1.0) + float4(offset.x,offset.y,offset.z,0.0);
        break;
      case RKLocalAxes::Position::originBoundingBox:
        this->localAxisPosition = float4(boundingbox.minimum().x,boundingbox.minimum().y,boundingbox.minimum().z,1.0) + float4(offset.x,offset.y,offset.z,0.0);
        break;
      case RKLocalAxes::Position::centerBoundingBox:
        this->localAxisPosition = float4(centerOfRotation.x, centerOfRotation.y, centerOfRotation.z, 1.0) + float4(offset.x,offset.y,offset.z,0.0);
        break;
      default:
        break;
      }
    }

    if (RKRenderUnitCellSource* renderStructure = dynamic_cast<RKRenderUnitCellSource*>(structure.get()))
    {
        this->unitCellScaling =  float(renderStructure->unitCellScaleFactor());
        this->unitCellDiffuseColor = float(renderStructure->unitCellDiffuseIntensity()) * float4(renderStructure->unitCellDiffuseColor().redF(),renderStructure->unitCellDiffuseColor().greenF(),
                                                                                                 renderStructure->unitCellDiffuseColor().blueF(),renderStructure->unitCellDiffuseColor().alphaF());
    }

    if (RKRenderAtomSource* source = dynamic_cast<RKRenderAtomSource*>(structure.get()))
    {
      this->colorAtomsWithBondColor = source->colorAtomsWithBondColor();
      this->atomScaleFactor = float(source->atomScaleFactor());

      this->atomHDR = source->atomHDR();
      this->atomHDRExposure = float(source->atomHDRExposure());
      this->atomSelectionIntensity = float(source->atomSelectionIntensity());
      this->clipAtomsAtUnitCell = source->clipAtomsAtUnitCell();

      this->atomHue = float(source->atomHue());
      this->atomSaturation = float(source->atomSaturation());
      this->atomValue = float(source->atomValue());

      this->ambientOcclusion = source->atomAmbientOcclusion();
      this->ambientOcclusionPatchNumber = int32_t(source->atomAmbientOcclusionPatchNumber());
      this->ambientOcclusionPatchSize = float(source->atomAmbientOcclusionPatchSize());
      this->ambientOcclusionInverseTextureSize = float(1.0/double(source->atomAmbientOcclusionTextureSize()));


      this->atomAmbient = float(source->atomAmbientIntensity()) * float4(source->atomAmbientColor().redF(),source->atomAmbientColor().greenF(),
                                                                            source->atomAmbientColor().blueF(),source->atomAmbientColor().alphaF());
      this->atomDiffuse = float(source->atomDiffuseIntensity()) * float4(source->atomDiffuseColor().redF(),source->atomDiffuseColor().greenF(),
                                                                            source->atomDiffuseColor().blueF(),source->atomDiffuseColor().alphaF());
      this->atomSpecular = float(source->atomSpecularIntensity()) * float4(source->atomSpecularColor().redF(),source->atomSpecularColor().greenF(),
                                                                              source->atomSpecularColor().blueF(),source->atomSpecularColor().alphaF());
      this->atomShininess = float(source->atomShininess());

      this->atomSelectionScaling = float(std::max(1.001,source->atomSelectionScaling())); // avoid artifacts
      this->atomSelectionStripesDensity = float(source->atomSelectionStripesDensity());
      this->atomSelectionStripesFrequency = float(source->atomSelectionStripesFrequency());
      this->atomSelectionWorleyNoise3DFrequency = float(source->atomSelectionWorleyNoise3DFrequency());
      this->atomSelectionWorleyNoise3DJitter = float(source->atomSelectionWorleyNoise3DJitter());

      this->atomAnnotationTextColor = float4(source->renderTextColor().redF(),source->renderTextColor().greenF(),
                                             source->renderTextColor().blueF(),source->renderTextColor().alphaF());
      this->atomAnnotationTextScaling = float(source->renderTextScaling());
      this->atomAnnotationTextDisplacement = float4(float(source->renderTextOffset().x),
                                                   float(source->renderTextOffset().y),
                                                   float(source->renderTextOffset().z),
                                                   0.0);
    }

    if (RKRenderBondSource* source = dynamic_cast<RKRenderBondSource*>(structure.get()))
    {
      this->bondScaling = float(source->bondScaleFactor());
      this->bondColorMode = int32_t(source->bondColorMode());

      this->bondHDR = source->bondHDR();
      this->bondHDRExposure = float(source->bondHDRExposure());
      this->bondSelectionIntensity = float(source->bondSelectionIntensity());
      this->clipBondsAtUnitCell = source->clipBondsAtUnitCell();

      this->bondHue = float(source->bondHue());
      this->bondSaturation = float(source->bondSaturation());
      this->bondValue = float(source->bondValue());

      this->bondAmbientColor = float(source->bondAmbientIntensity()) * float4(source->bondAmbientColor().redF(),source->bondAmbientColor().greenF(),
                                                                              source->bondAmbientColor().blueF(),source->bondAmbientColor().alphaF());
      this->bondDiffuseColor = float(source->bondDiffuseIntensity()) * float4(source->bondDiffuseColor().redF(),source->bondDiffuseColor().greenF(),
                                                                              source->bondDiffuseColor().blueF(),source->bondDiffuseColor().alphaF());
      this->bondSpecularColor = float(source->bondSpecularIntensity()) * float4(source->bondSpecularColor().redF(),source->bondSpecularColor().greenF(),
                                                                                source->bondSpecularColor().blueF(),source->bondSpecularColor().alphaF());
      this->bondShininess = float(source->bondShininess());

      this->bondSelectionStripesDensity = float(source->bondSelectionStripesDensity());
      this->bondSelectionStripesFrequency = float(source->bondSelectionStripesFrequency());
      this->bondSelectionWorleyNoise3DFrequency = float(source->bondSelectionWorleyNoise3DFrequency());
      this->bondSelectionWorleyNoise3DJitter = float(source->bondSelectionWorleyNoise3DJitter());
      this->bondSelectionIntensity = float(source->bondSelectionIntensity());
      this->bondSelectionScaling = float(std::max(1.001,source->bondSelectionScaling())); // avoid artifacts

      double3x3 unitCell = renderStructure->cell()->unitCell();
      double3 corner = unitCell * double3(double(renderStructure->cell()->minimumReplicas().x), double(renderStructure->cell()->minimumReplicas().y), double(renderStructure->cell()->minimumReplicas().z));
      double3 corner2 = unitCell * double3(double(renderStructure->cell()->maximumReplicas().x)+1.0, double(renderStructure->cell()->maximumReplicas().y)+1.0, double(renderStructure->cell()->maximumReplicas().z)+1.0);

      //this->boxMatrix = float4x4(box);
      //double3 shift = renderStructure->cell()->unitCell() * double3(renderStructure->cell()->minimumReplicas());
      //this->boxMatrix[3][0] = float(shift.x);
      //this->boxMatrix[3][1] = float(shift.y);
      //this->boxMatrix[3][2] = float(shift.z);



      // clipping planes are in object space
      double3 u_plane0 = double3::normalize(double3::cross(box[0],box[1]));
      double3 u_plane1 = double3::normalize(double3::cross(box[2],box[0]));
      double3 u_plane2 = double3::normalize(double3::cross(box[1],box[2]));
      this->clipPlaneBack = float4(u_plane0.x, u_plane0.y, u_plane0.z, -double3::dot(u_plane0,corner));
      this->clipPlaneBottom = float4(u_plane1.x, u_plane1.y, u_plane1.z, -double3::dot(u_plane1,corner));
      this->clipPlaneLeft = float4(u_plane2.x, u_plane2.y, u_plane2.z, -double3::dot(u_plane2,corner));
      this->clipPlaneFront = float4(-u_plane0.x, -u_plane0.y, -u_plane0.z, double3::dot(u_plane0,corner2));
      this->clipPlaneTop = float4(-u_plane1.x, -u_plane1.y, -u_plane1.z, double3::dot(u_plane1,corner2));
      this->clipPlaneRight = float4(-u_plane2.x, -u_plane2.y, -u_plane2.z, double3::dot(u_plane2,corner2));
    }

    if(RKRenderPrimitiveObjectsSource *source = dynamic_cast<RKRenderPrimitiveObjectsSource*>(structure.get()))
    {
      float4x4 primitiveModelMatrix = float4x4(double4x4(source->primitiveOrientation()));
      float4x4 primitiveNormalMatrix = float4x4(double3x3(source->primitiveOrientation()).inverse().transpose());  // tranpose

      this->primitiveTransformationMatrix = primitiveModelMatrix * float4x4(source->primitiveTransformationMatrix());
      this->primitiveTransformationNormalMatrix = primitiveNormalMatrix * float4x4(source->primitiveTransformationMatrix().inverse().transpose());

      this->primitiveOpacity = source->primitiveOpacity();

      this->primitiveSelectionStripesDensity = float(source->primitiveSelectionStripesDensity());
      this->primitiveSelectionStripesFrequency = float(source->primitiveSelectionStripesFrequency());
      this->primitiveSelectionWorleyNoise3DFrequency = float(source->primitiveSelectionWorleyNoise3DFrequency());
      this->primitiveSelectionWorleyNoise3DJitter = float(source->primitiveSelectionWorleyNoise3DJitter());
      this->primitiveSelectionIntensity = float(source->primitiveSelectionIntensity());
      this->primitiveSelectionScaling = float(std::max(1.001,source->primitiveSelectionScaling())); // avoid artifacts

      this->primitiveHue = float(source->primitiveHue());
      this->primitiveSaturation = float(source->primitiveSaturation());
      this->primitiveValue = float(source->primitiveValue());

      this->primitiveFrontSideHDR = source->primitiveFrontSideHDR();
      this->primitiveFrontSideHDRExposure = float(source->primitiveFrontSideHDRExposure());
      this->primitiveAmbientFrontSide = float(source->primitiveFrontSideAmbientIntensity()) * float4(source->primitiveFrontSideAmbientColor(), source->primitiveOpacity());
      this->primitiveDiffuseFrontSide = float(source->primitiveFrontSideDiffuseIntensity()) * float4(source->primitiveFrontSideDiffuseColor(), source->primitiveOpacity());
      this->primitiveSpecularFrontSide = float(source->primitiveFrontSideSpecularIntensity()) * float4(source->primitiveFrontSideSpecularColor(), source->primitiveOpacity());
      this->primitiveShininessFrontSide = float(source->primitiveFrontSideShininess());

      this->primitiveBackSideHDR = source->primitiveBackSideHDR();
      this->primitiveBackSideHDRExposure = float(source->primitiveBackSideHDRExposure());
      this->primitiveAmbientBackSide = float(source->primitiveBackSideAmbientIntensity()) * float4(source->primitiveBackSideAmbientColor(), source->primitiveOpacity());
      this->primitiveDiffuseBackSide = float(source->primitiveBackSideDiffuseIntensity()) * float4(source->primitiveBackSideDiffuseColor(), source->primitiveOpacity());
      this->primitiveSpecularBackSide = float(source->primitiveBackSideSpecularIntensity()) * float4(source->primitiveBackSideSpecularColor(), source->primitiveOpacity());
      this->primitiveShininessBackSide = float(source->primitiveBackSideShininess());
    }
  }
}

RKStructureUniforms::RKStructureUniforms(size_t sceneIdentifier, size_t movieIdentifier, std::shared_ptr<RKRenderObject> structure, double4x4 inverseModelMatrix)
{
  if (RKRenderObject* renderStructure = dynamic_cast<RKRenderObject*>(structure.get()))
  {
    if (RKRenderUnitCellSource* source = dynamic_cast<RKRenderUnitCellSource*>(structure.get()))
    {
      this->unitCellScaling =  float(source->unitCellScaleFactor());
      this->unitCellDiffuseColor = float(source->unitCellDiffuseIntensity()) * float4(source->unitCellDiffuseColor().redF(),source->unitCellDiffuseColor().greenF(),
                                                                                      source->unitCellDiffuseColor().blueF(),source->unitCellDiffuseColor().alphaF());
    }

    if (RKRenderAtomSource* source = dynamic_cast<RKRenderAtomSource*>(structure.get()))
    {
      SKBoundingBox boundingbox = renderStructure->cell()->boundingBox();
      double3 centerOfRotation = boundingbox.center();

      // difference
      double4x4 modelMatrix2 = inverseModelMatrix * double4x4::AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(double4x4(renderStructure->orientation()),centerOfRotation, renderStructure->origin());
      this->modelMatrix = float4x4(modelMatrix2);

      this->sceneIdentifier = int32_t(sceneIdentifier);
      this->MovieIdentifier = int32_t(movieIdentifier);

      this->colorAtomsWithBondColor = source->colorAtomsWithBondColor();
      this->atomScaleFactor = float(source->atomScaleFactor());
      this->modelMatrix = float4x4(modelMatrix);
      this->atomHue = source->atomHue();
      this->atomSaturation = source->atomSaturation();
      this->atomValue = source->atomValue();

      this->ambientOcclusion = source->atomAmbientOcclusion();
      this->ambientOcclusionPatchNumber = int32_t(source->atomAmbientOcclusionPatchNumber());
      this->ambientOcclusionPatchSize = float(source->atomAmbientOcclusionPatchSize());
      this->ambientOcclusionInverseTextureSize = float(1.0/double(source->atomAmbientOcclusionTextureSize()));

      this->atomAmbient = float(source->atomAmbientIntensity()) * float4(source->atomAmbientColor().redF(),source->atomAmbientColor().greenF(),
                                                                         source->atomAmbientColor().blueF(),source->atomAmbientColor().alphaF());
      this->atomDiffuse = float(source->atomDiffuseIntensity()) * float4(source->atomDiffuseColor().redF(),source->atomDiffuseColor().greenF(),
                                                                         source->atomDiffuseColor().blueF(),source->atomDiffuseColor().alphaF());
      this->atomSpecular = float(source->atomSpecularIntensity()) * float4(source->atomSpecularColor().redF(),source->atomSpecularColor().greenF(),
                                                                           source->atomSpecularColor().blueF(),source->atomSpecularColor().alphaF());
      this->atomShininess = float(source->atomShininess());


      this->atomHDR = source->atomHDR();
      this->atomHDRExposure = float(source->atomHDRExposure());
      this->atomSelectionIntensity = float(source->atomSelectionIntensity());
      this->clipAtomsAtUnitCell = source->clipAtomsAtUnitCell();



      double3x3 unitCell = renderStructure->cell()->unitCell();
      double3x3 box = renderStructure->cell()->box();
      double3 corner = unitCell * double3(double(renderStructure->cell()->minimumReplicas().x), double(renderStructure->cell()->minimumReplicas().y), double(renderStructure->cell()->minimumReplicas().z));
      double3 corner2 = unitCell * double3(double(renderStructure->cell()->maximumReplicas().x)+1.0, double(renderStructure->cell()->maximumReplicas().y)+1.0, double(renderStructure->cell()->maximumReplicas().z)+1.0);

      this->boxMatrix = float4x4(box);
      double3 shift = renderStructure->cell()->unitCell() * double3(renderStructure->cell()->minimumReplicas());
      this->boxMatrix[3][0] = float(shift.x);
      this->boxMatrix[3][1] = float(shift.y);
      this->boxMatrix[3][2] = float(shift.z);


      this->atomSelectionStripesDensity = float(source->atomSelectionStripesDensity());
      this->atomSelectionStripesFrequency = float(source->atomSelectionStripesFrequency());
      this->atomSelectionWorleyNoise3DFrequency = float(source->atomSelectionWorleyNoise3DFrequency());
      this->atomSelectionWorleyNoise3DJitter = float(source->atomSelectionWorleyNoise3DJitter());
      this->atomSelectionScaling = float(std::max(1.001,source->atomSelectionScaling())); // avoid artifacts
      this->atomSelectionIntensity = float(source->atomSelectionIntensity());

      this->atomAnnotationTextColor = float4(source->renderTextColor().redF(),source->renderTextColor().greenF(),
                                             source->renderTextColor().blueF(),source->renderTextColor().alphaF());
      this->atomAnnotationTextScaling = float(source->renderTextScaling());
      this->atomAnnotationTextDisplacement = float4(float(source->renderTextOffset().x),
                                                   float(source->renderTextOffset().y),
                                                   float(source->renderTextOffset().z),
                                                   0.0);



      // clipping planes are in object space
      double3 u_plane0 = double3::normalize(double3::cross(box[0],box[1]));
      double3 u_plane1 = double3::normalize(double3::cross(box[2],box[0]));
      double3 u_plane2 = double3::normalize(double3::cross(box[1],box[2]));
      this->clipPlaneBack = float4(u_plane0.x, u_plane0.y, u_plane0.z, -double3::dot(u_plane0,corner));
      this->clipPlaneBottom = float4(u_plane1.x, u_plane1.y, u_plane1.z, -double3::dot(u_plane1,corner));
      this->clipPlaneLeft = float4(u_plane2.x, u_plane2.y, u_plane2.z, -double3::dot(u_plane2,corner));
      this->clipPlaneFront = float4(-u_plane0.x, -u_plane0.y, -u_plane0.z, double3::dot(u_plane0,corner2));
      this->clipPlaneTop = float4(-u_plane1.x, -u_plane1.y, -u_plane1.z, double3::dot(u_plane1,corner2));
      this->clipPlaneRight = float4(-u_plane2.x, -u_plane2.y, -u_plane2.z, double3::dot(u_plane2,corner2));
    }

    if (RKRenderBondSource* source = dynamic_cast<RKRenderBondSource*>(structure.get()))
    {
      this->bondScaling = float(source->bondScaleFactor());
      this->bondColorMode = int32_t(source->bondColorMode());

      this->bondHDR = source->bondHDR();
      this->bondHDRExposure = float(source->bondHDRExposure());
      this->clipBondsAtUnitCell = source->clipBondsAtUnitCell();

      this->bondHue = float(source->bondHue());
      this->bondSaturation = float(source->bondSaturation());
      this->bondValue = float(source->bondValue());

      this->bondAmbientColor = float(source->bondAmbientIntensity()) * float4(source->bondAmbientColor().redF(),source->bondAmbientColor().greenF(),
                                                                              source->bondAmbientColor().blueF(),source->bondAmbientColor().alphaF());
      this->bondDiffuseColor = float(source->bondDiffuseIntensity()) * float4(source->bondDiffuseColor().redF(),source->bondDiffuseColor().greenF(),
                                                                              source->bondDiffuseColor().blueF(),source->bondDiffuseColor().alphaF());
      this->bondSpecularColor = float(source->bondSpecularIntensity()) * float4(source->bondSpecularColor().redF(),source->bondSpecularColor().greenF(),
                                                                                source->bondSpecularColor().blueF(),source->bondSpecularColor().alphaF());
      this->bondShininess = float(source->bondShininess());

      this->bondSelectionStripesDensity = float(source->bondSelectionStripesDensity());
      this->bondSelectionStripesFrequency = float(source->bondSelectionStripesFrequency());
      this->bondSelectionWorleyNoise3DFrequency = float(source->bondSelectionWorleyNoise3DFrequency());
      this->bondSelectionWorleyNoise3DJitter = float(source->bondSelectionWorleyNoise3DJitter());
      this->bondSelectionScaling = float(std::max(1.001,source->bondSelectionScaling())); // avoid artifacts
      this->bondSelectionIntensity = float(source->bondSelectionIntensity());
    }
  }
}


RKShadowUniforms::RKShadowUniforms(double4x4 projectionMatrix, double4x4 viewMatrix, double4x4 modelMatrix)
{
  double4x4 ViewToOpenGLDepthTextureMatrix = double4x4(double4(0.5, 0.0, 0.0, 0.0), double4(0.0, 0.5, 0.0, 0.0), double4(0.0, 0.0, 0.5, 0.0), double4(0.5, 0.5, 0.5, 1.0));

  double4x4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
  this->projectionMatrix = float4x4(projectionMatrix);
  this->viewMatrix = float4x4(viewMatrix * modelMatrix);
  this->shadowMatrix = float4x4(ViewToOpenGLDepthTextureMatrix * mvpMatrix);

  this->normalMatrix = float4x4(double4x4(double3x3::transpose(double3x3::inverse(double3x3(viewMatrix * modelMatrix)))));
}

RKIsosurfaceUniforms::RKIsosurfaceUniforms()
{

}

RKIsosurfaceUniforms::RKIsosurfaceUniforms(std::shared_ptr<RKRenderObject> structure)
{
  if (RKRenderObject* renderStructure = dynamic_cast<RKRenderObject*>(structure.get()))
  {
    if (RKRenderVolumetricDataSource* source = dynamic_cast<RKRenderVolumetricDataSource*>(structure.get()))
    {
      double3x3 currentUnitCellMatrix = renderStructure->cell()->unitCell();
      double3x3 currentBoxMatrix = renderStructure->cell()->box();
      this->unitCellMatrix = float4x4(currentUnitCellMatrix);
      this->inverseUnitCellMatrix = float4x4(currentUnitCellMatrix.inverse());
      this->unitCellNormalMatrix = float4x4(double3x3::transpose(double3x3::inverse(currentUnitCellMatrix)));

      this->stepLength = source->adsorptionVolumeStepLength();
      this->transparencyThreshold = source->adsorptionTransparencyThreshold();
      this->transferFunctionIndex = int32_t(source->adsorptionVolumeTransferFunction());
      int3 dimensions = source->dimensions();

      int largestSize = std::max({dimensions.x,dimensions.y,dimensions.z});
      int k = 1;
      while(largestSize > pow(2,k))
      {
        k += 1;
      }
      float encompassingCubeSize = float(pow(2,k));
      this->scaleToEncompassing = float4(dimensions.x/encompassingCubeSize,dimensions.y/encompassingCubeSize,
                                         dimensions.z/encompassingCubeSize,1.0);

      this->boxMatrix = float4x4(currentBoxMatrix);
      this->inverseBoxMatrix = float4x4(currentBoxMatrix.inverse());

      this->hue = float(source->adsorptionSurfaceHue());
      this->saturation = float(source->adsorptionSurfaceSaturation());
      this->value = float(source->adsorptionSurfaceValue());

      this->frontHDR = source->adsorptionSurfaceFrontSideHDR() ? 1 : 0;
      this->frontHDRExposure = float(source->adsorptionSurfaceFrontSideHDRExposure());
      this->ambientBackSide = float(source->adsorptionSurfaceBackSideAmbientIntensity()) * float4(source->adsorptionSurfaceBackSideAmbientColor().redF(),
                                                                                                  source->adsorptionSurfaceBackSideAmbientColor().greenF(),
                                                                                                  source->adsorptionSurfaceBackSideAmbientColor().blueF(),
                                                                                                  source->adsorptionSurfaceOpacity());
      this->diffuseBackSide = float(source->adsorptionSurfaceBackSideDiffuseIntensity()) * float4(source->adsorptionSurfaceBackSideDiffuseColor().redF(),
                                                                                                  source->adsorptionSurfaceBackSideDiffuseColor().greenF(),
                                                                                                  source->adsorptionSurfaceBackSideDiffuseColor().blueF(),
                                                                                                  source->adsorptionSurfaceOpacity());
      this->specularBackSide = float(source->adsorptionSurfaceBackSideSpecularIntensity()) * float4(source->adsorptionSurfaceBackSideSpecularColor().redF(),
                                                                                                    source->adsorptionSurfaceBackSideSpecularColor().greenF(),
                                                                                                    source->adsorptionSurfaceBackSideSpecularColor().blueF(),
                                                                                                    source->adsorptionSurfaceOpacity());
      this->shininessBackSide = float(source->adsorptionSurfaceBackSideShininess());

      this->backHDR = source->adsorptionSurfaceBackSideHDR() ? 1 : 0;
      this->backHDRExposure = float(source->adsorptionSurfaceBackSideHDRExposure());
      this->ambientFrontSide = float(source->adsorptionSurfaceFrontSideAmbientIntensity()) * float4(source->adsorptionSurfaceFrontSideAmbientColor().redF(),
                                                                                                    source->adsorptionSurfaceFrontSideAmbientColor().greenF(),
                                                                                                    source->adsorptionSurfaceFrontSideAmbientColor().blueF(),
                                                                                                    source->adsorptionSurfaceOpacity());
      this->diffuseFrontSide = float(source->adsorptionSurfaceFrontSideDiffuseIntensity()) * float4(source->adsorptionSurfaceFrontSideDiffuseColor().redF(),
                                                                                                    source->adsorptionSurfaceFrontSideDiffuseColor().greenF(),
                                                                                                    source->adsorptionSurfaceFrontSideDiffuseColor().blueF(),
                                                                                                    source->adsorptionSurfaceOpacity());
      this->specularFrontSide = float(source->adsorptionSurfaceFrontSideSpecularIntensity()) * float4(source->adsorptionSurfaceFrontSideSpecularColor().redF(),
                                                                                                      source->adsorptionSurfaceFrontSideSpecularColor().greenF(),
                                                                                                      source->adsorptionSurfaceFrontSideSpecularColor().blueF(),
                                                                                                      source->adsorptionSurfaceOpacity());
      this->shininessFrontSide = float(source->adsorptionSurfaceFrontSideShininess());
    }
  }
};


RKLightUniform::RKLightUniform(std::shared_ptr<RKRenderDataSource> project, int lightIndex)
{
  std::vector<std::shared_ptr<RKLight>> lights = project->renderLights();
  std::shared_ptr<RKLight> light = lights[lightIndex];
  this->ambient = float(light->ambientIntensity()) * float4(light->ambientColor().redF(),light->ambientColor().greenF(),light->ambientColor().blueF(),light->ambientColor().alphaF());
  this->diffuse = float(light->diffuseIntensity()) * float4(light->diffuseColor().redF(),light->diffuseColor().greenF(),light->diffuseColor().blueF(),light->diffuseColor().alphaF());
  this->specular = float(light->specularIntensity()) * float4(light->specularColor().redF(),light->specularColor().greenF(),light->specularColor().blueF(),light->specularColor().alphaF());
  this->shininess = float(light->shininess());
};

RKLightUniform::RKLightUniform()
{

};

// Note: must be aligned at vector-length (16-bytes boundaries, 4 Floats of 4 bytes)
// current number of bytes: 512 bytes
RKLightsUniforms::RKLightsUniforms(std::shared_ptr<RKRenderDataSource> project)
{
  if(project)
  {
    std::vector<std::shared_ptr<RKLight>> currentLights = project->renderLights();
    std::shared_ptr<RKLight> light = currentLights[0];
    this->lights[0].ambient = float(light->ambientIntensity()) * float4(light->ambientColor().redF(),light->ambientColor().greenF(),light->ambientColor().blueF(),light->ambientColor().alphaF());
    this->lights[0].diffuse = float(light->diffuseIntensity()) * float4(light->diffuseColor().redF(),light->diffuseColor().greenF(),light->diffuseColor().blueF(),light->diffuseColor().alphaF());
    this->lights[0].specular = float(light->specularIntensity()) * float4(light->specularColor().redF(),light->specularColor().greenF(),light->specularColor().blueF(),light->specularColor().alphaF());
    this->lights[0].shininess = float(light->shininess());
  }
};

RKGlobalAxesUniforms::RKGlobalAxesUniforms(std::shared_ptr<RKRenderDataSource> project)
{
  if(project)
  {
    axesBackgroundColor = float4(project->axes()->axesBackgroundColor().redF(),
                                 project->axes()->axesBackgroundColor().greenF(),
                                 project->axes()->axesBackgroundColor().blueF(),
                                 project->axes()->axesBackgroundColor().alphaF());
    axesBackGroundStyle = static_cast<typename std::underlying_type<RKGlobalAxes::BackgroundStyle>::type>(project->axes()->axesBackgroundStyle());

    axesScale = float(project->axes()->axisScale());
    centerScale = float(project->axes()->centerScale());
    textOffset = float(project->axes()->textOffset());

    textColor[0] = float4(project->axes()->textColorX().redF(),
                          project->axes()->textColorX().greenF(),
                          project->axes()->textColorX().blueF(),
                          project->axes()->textColorX().alphaF());
    textColor[1] = float4(project->axes()->textColorY().redF(),
                          project->axes()->textColorY().greenF(),
                          project->axes()->textColorY().blueF(),
                          project->axes()->textColorY().alphaF());
    textColor[2] = float4(project->axes()->textColorZ().redF(),
                          project->axes()->textColorZ().greenF(),
                          project->axes()->textColorZ().blueF(),
                          project->axes()->textColorZ().alphaF());

    textDisplacement[0] = float4(project->axes()->textDisplacementX().x,
                                 project->axes()->textDisplacementX().y,
                                 project->axes()->textDisplacementX().z,
                                 1.0);
    textDisplacement[1] = float4(project->axes()->textDisplacementY().x,
                                 project->axes()->textDisplacementY().y,
                                 project->axes()->textDisplacementY().z,
                                 1.0);
    textDisplacement[2] = float4(project->axes()->textDisplacementZ().x,
                                 project->axes()->textDisplacementZ().y,
                                 project->axes()->textDisplacementZ().z,
                                 1.0);

    textScale = float4(project->axes()->textScale().x,
                       project->axes()->textScale().y,
                       project->axes()->textScale().z,
                       1.0);
  }
}
