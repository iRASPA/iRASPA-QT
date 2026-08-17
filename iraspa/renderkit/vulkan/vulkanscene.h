#pragma once

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include <QImage>

#include "rkrenderkitprotocols.h"
#include "vulkanatomsphereshader.h"
#include "vulkanatomtextshader.h"
#include "vulkanbackgroundshader.h"
#include "vulkanbondshader.h"
#include "vulkanboundingboxshader.h"
#include "vulkanglobalaxesshader.h"
#include "vulkanisosurfaceshader.h"
#include "vulkanlocalaxesshader.h"
#include "vulkanmeasurementshader.h"
#include "vulkanpickingshader.h"
#include "vulkanprimitiveobjectshader.h"
#include "vulkanrenderer.h"
#include "vulkanribbonshader.h"
#include "vulkanselectionshader.h"
#include "vulkanunitcellshader.h"
#include "vulkanvolumeshader.h"

class VulkanScene
{
public:
  explicit VulkanScene(VulkanRenderer *renderer);

  void initialize();
  bool isInitialized() const { return _initialized; }

  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source);
  void reloadData();
  void reloadData(RKRenderQuality ambientOcclusionQuality);
  void reloadAmbientOcclusionData();
  void reloadSelectionData();
  void reloadBoundingBoxData();
  void reloadGlobalAxesData();
  void reloadBackgroundImage();
  void reloadRenderMeasurePointsData();
  void invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures);
  void invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures);

  void updateTransformUniforms();
  void updateStructureUniforms();
  void updateIsosurfaceUniforms();
  void updateLightUniforms();
  void updateGlobalAxesUniforms();

  void draw(RKRenderQuality quality = RKRenderQuality::high);
  void cycleRibbonAODebugMode();
  RibbonAODebugMode ribbonAODebugMode() const;
  QImage renderSceneToImage(int width, int height, RKRenderQuality quality);
  std::array<int, 4> pickTexture(int x, int y, RKRenderQuality quality = RKRenderQuality::high);
  std::optional<float> pickDepth(int x, int y, RKRenderQuality quality = RKRenderQuality::high);

  std::weak_ptr<RKCamera> camera() const { return _camera; }

private:
  void recordScene(VkCommandBuffer commandBuffer, RKRenderQuality quality);

  VulkanRenderer *_renderer = nullptr;
  std::unique_ptr<VulkanBackgroundShader> _backgroundShader;
  std::unique_ptr<VulkanAtomSphereShader> _atomShader;
  std::unique_ptr<VulkanBondShader> _bondShader;
  std::unique_ptr<VulkanPrimitiveObjectShader> _objectShader;
  std::unique_ptr<VulkanUnitCellShader> _unitCellShader;
  std::unique_ptr<VulkanLocalAxesShader> _localAxesShader;
  std::unique_ptr<VulkanRibbonShader> _ribbonShader;
  std::unique_ptr<VulkanBoundingBoxShader> _boundingBoxShader;
  std::unique_ptr<VulkanIsosurfaceShader> _isosurfaceShader;
  std::unique_ptr<VulkanVolumeShader> _volumeShader;
  std::unique_ptr<VulkanAtomTextShader> _atomTextShader;
  std::unique_ptr<VulkanSelectionShader> _selectionShader;
  std::unique_ptr<VulkanMeasurementShader> _measurementShader;
  std::unique_ptr<VulkanPickingShader> _pickingShader;
  std::unique_ptr<VulkanGlobalAxesShader> _globalAxesShader;

  bool _initialized = false;
  std::shared_ptr<RKRenderDataSource> _dataSource;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures{};
  std::weak_ptr<RKCamera> _camera;
  bool _isOrthographic = true;
};
