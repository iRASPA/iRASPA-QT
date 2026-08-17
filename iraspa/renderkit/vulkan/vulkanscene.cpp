#include "vulkanscene.h"

#include <optional>

#include "rkcamera.h"
#include "rkglobalaxes.h"
#include "rkimposters.h"
#include "rkrenderuniforms.h"
#include "ribbonaolayout.h"

#include <algorithm>
#include <exception>

#include <QDebug>

VulkanScene::VulkanScene(VulkanRenderer *renderer) : _renderer(renderer)
{
}

void VulkanScene::initialize()
{
  if (_initialized || !_renderer)
  {
    return;
  }

  _backgroundShader = std::make_unique<VulkanBackgroundShader>(_renderer);
  _atomShader = std::make_unique<VulkanAtomSphereShader>(_renderer);
  _bondShader = std::make_unique<VulkanBondShader>(_renderer);
  _objectShader = std::make_unique<VulkanPrimitiveObjectShader>(_renderer);
  _unitCellShader = std::make_unique<VulkanUnitCellShader>(_renderer);
  _localAxesShader = std::make_unique<VulkanLocalAxesShader>(_renderer);
  _ribbonShader = std::make_unique<VulkanRibbonShader>(_renderer, _atomShader.get());
  _boundingBoxShader = std::make_unique<VulkanBoundingBoxShader>(_renderer);
  _isosurfaceShader = std::make_unique<VulkanIsosurfaceShader>(_renderer);
  _volumeShader = std::make_unique<VulkanVolumeShader>(_renderer);
  _atomTextShader = std::make_unique<VulkanAtomTextShader>(_renderer);
  _selectionShader = std::make_unique<VulkanSelectionShader>(_renderer, _ribbonShader.get());
  _measurementShader = std::make_unique<VulkanMeasurementShader>(_renderer);
  _globalAxesShader = std::make_unique<VulkanGlobalAxesShader>(_renderer);
  _pickingShader = std::make_unique<VulkanPickingShader>(_renderer, _atomShader.get(), _bondShader.get(),
                                                        _objectShader.get(), _ribbonShader.get());
  _backgroundShader->initialize();
  _atomShader->initialize();
  _bondShader->initialize();
  _objectShader->initialize();
  _unitCellShader->initialize();
  _localAxesShader->initialize();
  _ribbonShader->initialize();
  _boundingBoxShader->initialize();
  _isosurfaceShader->initialize();
  _volumeShader->initialize();
  _atomTextShader->initialize();
  _selectionShader->initialize();
  _measurementShader->initialize();
  _globalAxesShader->initialize();
  _pickingShader->initialize();
  _initialized = true;

  if (_dataSource)
  {
    _backgroundShader->reload(_dataSource);
    _boundingBoxShader->setRenderDataSource(_dataSource);
    _globalAxesShader->setRenderDataSource(_dataSource);
    _measurementShader->setRenderDataSource(_dataSource);
  }
  if (!_renderStructures.empty())
  {
    setRenderStructures(_renderStructures);
  }
  reloadData();
}

void VulkanScene::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  if (!_initialized)
  {
    return;
  }
  if (_renderer)
  {
    _renderer->waitIdle();
  }
  _atomShader->setRenderStructures(_renderStructures);
  _bondShader->setRenderStructures(_renderStructures);
  _objectShader->setRenderStructures(_renderStructures);
  _unitCellShader->setRenderStructures(_renderStructures);
  _localAxesShader->setRenderStructures(_renderStructures);
  _ribbonShader->setRenderStructures(_renderStructures);
  _isosurfaceShader->setRenderStructures(_renderStructures);
  _volumeShader->setRenderStructures(_renderStructures);
  _atomTextShader->setRenderStructures(_renderStructures);
  _selectionShader->setRenderStructures(_renderStructures);
  _measurementShader->setRenderStructures(_renderStructures);
}

void VulkanScene::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = source;
  if (source)
  {
    _camera = source->camera();
  }

  if (_initialized)
  {
    _backgroundShader->reload(source);
    _boundingBoxShader->setRenderDataSource(source);
    _globalAxesShader->setRenderDataSource(source);
    _measurementShader->setRenderDataSource(source);
    reloadData();
  }
}

void VulkanScene::reloadData()
{
  reloadData(RKRenderQuality::low);
}

void VulkanScene::reloadData(RKRenderQuality quality)
{
  if (!_initialized)
  {
    return;
  }
  if (_renderer)
  {
    _renderer->waitIdle();
  }
  _atomShader->reloadData();
  _bondShader->reloadData();
  _objectShader->reloadData();
  _unitCellShader->reloadData();
  _localAxesShader->reloadData();
  _ribbonShader->reloadData();
  _boundingBoxShader->reloadData();
  _isosurfaceShader->reloadData();
  _volumeShader->reloadData();
  _atomShader->reloadAmbientOcclusionData(_dataSource, quality);
  _ribbonShader->reloadAmbientOcclusionData(_dataSource, quality);
  _atomTextShader->reloadData();
  _selectionShader->reloadData();
  _measurementShader->reloadData();
  _globalAxesShader->reloadData();
  updateStructureUniforms();
  updateIsosurfaceUniforms();
}

void VulkanScene::reloadAmbientOcclusionData()
{
  if (!_initialized)
  {
    return;
  }
  if (_renderer)
  {
    _renderer->waitIdle();
  }
  _atomShader->reloadAmbientOcclusionData(_dataSource, RKRenderQuality::low);
  _ribbonShader->reloadAmbientOcclusionData(_dataSource, RKRenderQuality::low);
  updateStructureUniforms();
}

void VulkanScene::reloadSelectionData()
{
  if (_initialized && _selectionShader)
  {
    _selectionShader->reloadData();
  }
}

void VulkanScene::reloadBoundingBoxData()
{
  if (_initialized && _boundingBoxShader)
  {
    _boundingBoxShader->reloadData();
  }
}

void VulkanScene::reloadGlobalAxesData()
{
  if (_initialized && _globalAxesShader)
  {
    updateGlobalAxesUniforms();
    _globalAxesShader->reloadData();
  }
}

void VulkanScene::reloadRenderMeasurePointsData()
{
  if (_initialized && _measurementShader)
  {
    _measurementShader->reloadData();
  }
}

void VulkanScene::reloadBackgroundImage()
{
  if (_initialized && _backgroundShader)
  {
    _backgroundShader->reload(_dataSource);
  }
}

void VulkanScene::invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  if (_initialized && _atomShader)
  {
    _atomShader->invalidateCachedAmbientOcclusionTexture(structures);
  }
  if (_initialized && _ribbonShader)
  {
    _ribbonShader->invalidateCachedAmbientOcclusionTexture(structures);
  }
}

void VulkanScene::invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  if (_initialized && _isosurfaceShader)
  {
    _isosurfaceShader->invalidateIsosurface(structures);
  }
  if (_initialized && _volumeShader)
  {
    _volumeShader->invalidateIsosurface(structures);
  }
}

void VulkanScene::updateTransformUniforms()
{
  if (!_initialized || !_renderer)
  {
    return;
  }

  double4x4 projectionMatrix{};
  double4x4 modelViewMatrix{};
  double4x4 modelMatrix{};
  double4x4 viewMatrix{};
  double4x4 axesProjectionMatrix{};
  double4x4 axesModelViewMatrix{};
  double bloomLevel = 1.0;
  double bloomPulse = 1.0;
  bool isOrthographic = true;

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    projectionMatrix = camera->projectionMatrix();
    modelViewMatrix = camera->modelViewMatrix();
    modelMatrix = camera->modelMatrix();
    viewMatrix = camera->viewMatrix();
    isOrthographic = camera->isOrthographic();
    bloomLevel = camera->bloomLevel();
    bloomPulse = camera->bloomPulse();
    if (_dataSource)
    {
      axesProjectionMatrix = camera->axesProjectionMatrix(_dataSource->axes()->totalAxesSize());
      axesModelViewMatrix = camera->axesModelViewMatrix();
    }
  }

  RKTransformationUniforms uniforms(projectionMatrix, modelViewMatrix, modelMatrix, viewMatrix, axesProjectionMatrix, axesModelViewMatrix,
                                    isOrthographic, bloomLevel, bloomPulse, static_cast<int>(_renderer->msaaSampleCount()));
  _isOrthographic = isOrthographic;

  // OpenGL/Direct3D/Metal clip space is Y-up; Vulkan NDC is Y-down. Scale Y by -1 so
  // the same OpenGL projection puts the top of the scene at the top of the framebuffer.
  // That Y flip also reverses triangle winding, so pipelines use counter-clockwise
  // (OpenGL's native front face). Z[-w,w] → Z[0,w] is the same convert Metal uses.
  const float4x4 glToVulkanClip(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);
  uniforms.projectionMatrix = glToVulkanClip * uniforms.projectionMatrix;
  uniforms.mvpMatrix = glToVulkanClip * uniforms.mvpMatrix;
  uniforms.shadowMatrix = glToVulkanClip * uniforms.shadowMatrix;
  uniforms.axesProjectionMatrix = glToVulkanClip * uniforms.axesProjectionMatrix;
  uniforms.axesMvpMatrix = glToVulkanClip * uniforms.axesMvpMatrix;

  _renderer->updateTransformUniforms(uniforms);
}

void VulkanScene::updateStructureUniforms()
{
  if (!_initialized || !_renderer)
  {
    return;
  }

  std::vector<RKStructureUniforms> structureUniforms;
  size_t flatIndex = 0;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      RKStructureUniforms structureUniform(i, j, _renderStructures[i][j]);
      structureUniform.structureIdentifier = static_cast<int32_t>(flatIndex);
      structureUniforms.push_back(structureUniform);
      ++flatIndex;
    }
  }
  if (structureUniforms.empty())
  {
    structureUniforms.push_back(RKStructureUniforms());
  }
  _renderer->updateStructureUniforms(structureUniforms);
}

void VulkanScene::updateIsosurfaceUniforms()
{
  if (!_initialized || !_renderer)
  {
    return;
  }

  std::vector<RKIsosurfaceUniforms> isosurfaceUniforms;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      isosurfaceUniforms.push_back(RKIsosurfaceUniforms(_renderStructures[i][j]));
    }
  }
  if (isosurfaceUniforms.empty())
  {
    isosurfaceUniforms.push_back(RKIsosurfaceUniforms());
  }
  _renderer->updateIsosurfaceUniforms(isosurfaceUniforms);
}

void VulkanScene::updateLightUniforms()
{
  if (!_initialized || !_renderer)
  {
    return;
  }
  _renderer->updateLightUniforms(RKLightsUniforms(_dataSource));
}

void VulkanScene::updateGlobalAxesUniforms()
{
  if (!_initialized || !_renderer || !_dataSource)
  {
    return;
  }
  _renderer->updateGlobalAxesUniforms(RKGlobalAxesUniforms(_dataSource));
}

void VulkanScene::recordScene(VkCommandBuffer commandBuffer, RKRenderQuality quality)
{
  _backgroundShader->paint(commandBuffer);
  _atomShader->paint(commandBuffer, quality, _isOrthographic);
  _bondShader->paint(commandBuffer, quality);
  _objectShader->paintOpaque(commandBuffer);
  _unitCellShader->paint(commandBuffer);
  _localAxesShader->paint(commandBuffer);
  _ribbonShader->paint(commandBuffer);
  _boundingBoxShader->paint(commandBuffer);
  _isosurfaceShader->paintOpaque(commandBuffer);

  const std::vector<RKBackToFrontItem> order = backToFrontRenderOrder(_renderStructures, _camera.lock().get());

  if (_volumeShader->hasVisibleVolumes())
  {
    _renderer->snapshotDepthAndResume();
    for (const RKBackToFrontItem &item : order)
    {
      _volumeShader->paintOpaque(commandBuffer, item.sceneIndex, item.movieIndex);
    }
    if (_volumeShader->hasTransparentVolumes())
    {
      _renderer->snapshotDepthAndResume();
    }
  }

  // Draw all transparent objects back-to-front per structure, interleaving the
  // shader types so overlapping transparent objects from different movies blend correctly.
  for (const RKBackToFrontItem &item : order)
  {
    _volumeShader->paintTransparent(commandBuffer, item.sceneIndex, item.movieIndex);
    _isosurfaceShader->paintTransparent(commandBuffer, item.sceneIndex, item.movieIndex);
    _objectShader->paintTransparent(commandBuffer, item.sceneIndex, item.movieIndex);
  }

  _selectionShader->paint(commandBuffer, quality, _isOrthographic);
  _measurementShader->paint(commandBuffer, _isOrthographic);
  _atomTextShader->paint(commandBuffer);
  _globalAxesShader->paint(commandBuffer);
}

void VulkanScene::draw(RKRenderQuality quality)
{
  if (!_initialized || !_renderer)
  {
    return;
  }

  // Shared UBOs are overwritten in place. Finish the previous submit before
  // replacing camera/structure uniforms, or one in-flight frame can present
  // the new camera with the previous project's geometry (or vice versa).
  _renderer->waitIdle();
  updateTransformUniforms();
  updateStructureUniforms();
  updateIsosurfaceUniforms();
  updateLightUniforms();
  updateGlobalAxesUniforms();
  const VkExtent2D extent = _renderer->swapChainExtent();
  if (_ribbonShader)
  {
    _renderer->updateRibbonAODebugUniforms(_ribbonShader->debugUniforms(static_cast<int>(extent.width), static_cast<int>(extent.height)));
  }

  if (!_renderer->beginFrame())
  {
    return;
  }
  recordScene(_renderer->currentCommandBuffer(), quality);
  _renderer->endFrame();
}

void VulkanScene::cycleRibbonAODebugMode()
{
  if (!_ribbonShader)
  {
    return;
  }
  RibbonAODebugMode mode = _ribbonShader->aoDebugMode();
  ::cycleRibbonAODebugMode(mode);
  _ribbonShader->setAoDebugMode(mode);
  qDebug().noquote() << QStringLiteral("Ribbon AO debug:") << ribbonAODebugModeLabel(mode);
  if (mode == RibbonAODebugMode::uniformColors)
  {
    qDebug().noquote() << ribbonColorUniformDebugOverlayText(_renderStructures);
  }
}

RibbonAODebugMode VulkanScene::ribbonAODebugMode() const
{
  return _ribbonShader ? _ribbonShader->aoDebugMode() : RibbonAODebugMode::off;
}

QImage VulkanScene::renderSceneToImage(int width, int height, RKRenderQuality quality)
{
  if (!_initialized || !_renderer)
  {
    return QImage();
  }

  const int w = std::max(1, width);
  const int h = std::max(1, height);
  _renderer->waitIdle();
  _renderer->resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->updateCameraForWindowResize(w, h);
  }
  draw(quality);
  return _renderer->takeReadbackImage();
}

std::array<int, 4> VulkanScene::pickTexture(int x, int y, RKRenderQuality quality)
{
  if (!_initialized || !_renderer || !_pickingShader)
  {
    return {0, 0, 0, 0};
  }

  updateTransformUniforms();
  updateStructureUniforms();

  VkCommandBuffer commandBuffer = _renderer->beginPickPass();
  if (!commandBuffer)
  {
    return {0, 0, 0, 0};
  }

  _pickingShader->paint(commandBuffer, quality, _isOrthographic);
  return _renderer->endPickPassAndReadPixel(x, y);
}

std::optional<float> VulkanScene::pickDepth(int x, int y, RKRenderQuality quality)
{
  if (!_initialized || !_renderer || !_pickingShader)
  {
    return std::nullopt;
  }

  updateTransformUniforms();
  updateStructureUniforms();

  VkCommandBuffer commandBuffer = _renderer->beginPickPass();
  if (!commandBuffer)
  {
    return std::nullopt;
  }

  _pickingShader->paint(commandBuffer, quality, _isOrthographic);
  float depth = 1.0f;
  const std::array<int, 4> pixel = _renderer->endPickPassAndReadPixel(x, y, &depth);
  if (pixel[0] < 1)
  {
    return std::nullopt;
  }
  return depth;
}
