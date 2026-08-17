#include "vulkanwindow.h"

#include "rkcamera.h"
#include "vulkanrenderer.h"

#include <cmath>
#include <exception>

#include <QEvent>
#include <QExposeEvent>
#include <QMouseEvent>
#include <QRect>
#include <QWheelEvent>

#ifdef Q_OS_MACOS
extern "C" {
void *makeViewMetalCompatible(void *handle);
}
#endif

VulkanWindow::VulkanWindow(QWindow *parent) : QWindow(parent)
{
  setSurfaceType(QSurface::RasterSurface);

  _timer = new QTimer(this);
  _timer->setSingleShot(true);
  QObject::connect(_timer, &QTimer::timeout, this, [this]() {
    _quality = RKRenderQuality::high;
    drawFrame();
  });
}

VulkanWindow::~VulkanWindow()
{
  prepareForDestruction();
}

void VulkanWindow::prepareForDestruction()
{
  _destroyed = true;
  if (_timer)
  {
    _timer->stop();
  }
  if (_renderer)
  {
    _renderer->waitIdle();
  }
  _scene.reset();
  _renderer.reset();
  _initialized = false;
}

void VulkanWindow::initializeRenderer()
{
  if (_destroyed || _initialized)
  {
    return;
  }

  try
  {
#ifdef Q_OS_MACOS
    makeViewMetalCompatible(reinterpret_cast<void *>(winId()));
#endif

    _renderer = std::make_unique<VulkanRenderer>(this);
    _scene = std::make_unique<VulkanScene>(_renderer.get());
    if (_dataSource)
    {
      _scene->setRenderDataSource(_dataSource);
    }
    if (!_renderStructures.empty())
    {
      _scene->setRenderStructures(_renderStructures);
    }
    _scene->initialize();
    _initialized = true;
  }
  catch (const std::exception &e)
  {
    qCritical("Vulkan renderer initialization failed: %s", e.what());
  }
}

void VulkanWindow::exposeEvent(QExposeEvent *event)
{
  QWindow::exposeEvent(event);
  if (_destroyed)
  {
    return;
  }
  if (isExposed())
  {
    initializeRenderer();
    drawFrame();
  }
}

bool VulkanWindow::event(QEvent *event)
{
  if (event->type() == QEvent::Close)
  {
    prepareForDestruction();
  }
  if (_destroyed)
  {
    return QWindow::event(event);
  }
  if (event->type() == QEvent::UpdateRequest)
  {
    drawFrame();
    return true;
  }
  return QWindow::event(event);
}

void VulkanWindow::resizeEvent(QResizeEvent *event)
{
  QWindow::resizeEvent(event);
  if (_destroyed || !_initialized || !_renderer)
  {
    return;
  }
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->updateCameraForWindowResize(event->size().width(), event->size().height());
  }
  _renderer->resize(static_cast<uint32_t>(event->size().width()), static_cast<uint32_t>(event->size().height()));
  drawFrame();
}

void VulkanWindow::drawFrame()
{
  if (_destroyed || !_initialized || !_scene || !isExposed())
  {
    return;
  }
  _scene->draw(_quality);
}

void VulkanWindow::scheduleFrame()
{
  requestUpdate();
}

void VulkanWindow::cycleRibbonAODebugMode()
{
  if (!_scene)
  {
    return;
  }
  _scene->cycleRibbonAODebugMode();
  scheduleFrame();
}

void VulkanWindow::redraw()
{
  scheduleFrame();
}

void VulkanWindow::redrawWithQuality(RKRenderQuality quality)
{
  _quality = quality;
  redraw();
}

void VulkanWindow::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = structures;
  if (_scene)
  {
    _scene->setRenderStructures(_renderStructures);
  }
}

void VulkanWindow::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = source;
  if (source)
  {
    _camera = source->camera();
    if (std::shared_ptr<RKCamera> camera = _camera.lock())
    {
      camera->updateCameraForWindowResize(size().width(), size().height());
      camera->resetForNewBoundingBox(source->renderBoundingBox());
    }
  }

  if (_scene)
  {
    _scene->setRenderDataSource(source);
    scheduleFrame();
  }
}

void VulkanWindow::reloadData()
{
  if (_scene)
  {
    _scene->reloadData();
    scheduleFrame();
  }
}

void VulkanWindow::reloadData(RKRenderQuality quality)
{
  if (_scene)
  {
    _scene->reloadData(quality);
    scheduleFrame();
  }
}

void VulkanWindow::reloadAmbientOcclusionData()
{
  if (_scene)
  {
    _scene->reloadAmbientOcclusionData();
    scheduleFrame();
  }
}

void VulkanWindow::reloadRenderData()
{
  reloadData();
}

void VulkanWindow::reloadSelectionData()
{
  if (_scene)
  {
    _scene->reloadSelectionData();
    scheduleFrame();
  }
}

void VulkanWindow::reloadRenderMeasurePointsData()
{
  if (_scene)
  {
    _scene->reloadRenderMeasurePointsData();
    scheduleFrame();
  }
}

void VulkanWindow::reloadBoundingBoxData()
{
  if (_scene)
  {
    _scene->reloadBoundingBoxData();
    scheduleFrame();
  }
}

void VulkanWindow::reloadGlobalAxesData()
{
  if (_scene)
  {
    _scene->reloadGlobalAxesData();
    scheduleFrame();
  }
}

void VulkanWindow::reloadBackgroundImage()
{
  if (_scene)
  {
    _scene->reloadBackgroundImage();
    scheduleFrame();
  }
}

void VulkanWindow::invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  if (_scene)
  {
    _scene->invalidateCachedAmbientOcclusionTextures(structures);
  }
}

void VulkanWindow::invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  if (_scene)
  {
    _scene->invalidateCachedIsosurfaces(structures);
  }
}

void VulkanWindow::updateTransformUniforms()
{
  if (_scene)
  {
    _scene->updateTransformUniforms();
  }
}

void VulkanWindow::updateStructureUniforms()
{
  if (_scene)
  {
    _scene->updateStructureUniforms();
  }
}

void VulkanWindow::updateIsosurfaceUniforms()
{
  if (_scene)
  {
    _scene->updateIsosurfaceUniforms();
  }
}

void VulkanWindow::updateLightUniforms()
{
  if (_scene)
  {
    _scene->updateLightUniforms();
  }
}

void VulkanWindow::updateGlobalAxesUniforms()
{
  if (_scene)
  {
    _scene->updateGlobalAxesUniforms();
  }
}

void VulkanWindow::reloadStructureUniforms()
{
  updateStructureUniforms();
}

void VulkanWindow::updateVertexArrays()
{
  reloadData();
}

QImage VulkanWindow::renderSceneToImage(int width, int height, RKRenderQuality quality)
{
  if (!_scene)
  {
    return QImage();
  }
  return _scene->renderSceneToImage(width, height, quality);
}

std::array<int, 4> VulkanWindow::pickTexture(int x, int y, int, int)
{
  if (!_scene)
  {
    return {0, 0, 0, 0};
  }
  const qreal dpr = devicePixelRatio();
  const int px = static_cast<int>(std::lround(static_cast<qreal>(x) * dpr));
  const int py = static_cast<int>(std::lround(static_cast<qreal>(y) * dpr));
  return _scene->pickTexture(px, py, _quality);
}

std::optional<float> VulkanWindow::pickDepth(int x, int y, int, int)
{
  if (!_scene)
  {
    return std::nullopt;
  }
  const qreal dpr = devicePixelRatio();
  const int px = static_cast<int>(std::lround(static_cast<qreal>(x) * dpr));
  const int py = static_cast<int>(std::lround(static_cast<qreal>(y) * dpr));
  return _scene->pickDepth(px, py, _quality);
}

void VulkanWindow::mousePressEvent(QMouseEvent *event)
{
  _timer->stop();
  _tracking = VulkanTracking::none;
  _startPoint = event->pos();
  _origin = event->pos();
  _draggedPos = event->pos();
  _quality = RKRenderQuality::medium;

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->setTrackBallRotation(simd_quatd(1.0, double3(0.0, 0.0, 0.0)));
  }

  if (event->modifiers() & Qt::ShiftModifier)
  {
    _tracking = VulkanTracking::newSelection;
  }
  else if (event->modifiers() == Qt::ControlModifier)
  {
    _tracking = VulkanTracking::addToSelection;
  }
  else if ((event->modifiers() & Qt::AltModifier) && (event->modifiers() & Qt::ControlModifier))
  {
    _tracking = VulkanTracking::translateSelection;
  }
  else if (event->modifiers() == Qt::AltModifier)
  {
    _tracking = VulkanTracking::measurement;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    _trackBall.start(event->pos().x(), event->pos().y(), 0, 0, width(), height());
#else
    _trackBall.start(event->position().x(), event->position().y(), 0, 0, width(), height());
#endif
  }
  else
  {
    _tracking = VulkanTracking::backgroundClick;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    _trackBall.start(event->pos().x(), event->pos().y(), 0, 0, width(), height());
#else
    _trackBall.start(event->position().x(), event->position().y(), 0, 0, width(), height());
#endif
  }
  drawFrame();
}

void VulkanWindow::updateSelectionOverlay()
{
  if (!_renderer)
  {
    return;
  }
  if (_tracking != VulkanTracking::draggedNewSelection && _tracking != VulkanTracking::draggedAddToSelection)
  {
    _renderer->clearSelectionOverlay();
    return;
  }
  const qreal dpr = devicePixelRatio();
  const QRect logical = QRect(_origin, _draggedPos).normalized();
  const QRect framebuffer(static_cast<int>(std::lround(static_cast<qreal>(logical.x()) * dpr)),
                          static_cast<int>(std::lround(static_cast<qreal>(logical.y()) * dpr)),
                          static_cast<int>(std::lround(static_cast<qreal>(logical.width()) * dpr)),
                          static_cast<int>(std::lround(static_cast<qreal>(logical.height()) * dpr)));
  _renderer->setSelectionOverlay(framebuffer, _tracking == VulkanTracking::draggedAddToSelection, static_cast<float>(dpr));
}

void VulkanWindow::mouseMoveEvent(QMouseEvent *event)
{
  switch (_tracking)
  {
  case VulkanTracking::none:
    break;
  case VulkanTracking::newSelection:
    _tracking = VulkanTracking::draggedNewSelection;
    _draggedPos = event->pos();
    updateSelectionOverlay();
    drawFrame();
    break;
  case VulkanTracking::addToSelection:
    _tracking = VulkanTracking::draggedAddToSelection;
    _draggedPos = event->pos();
    updateSelectionOverlay();
    drawFrame();
    break;
  case VulkanTracking::draggedNewSelection:
  case VulkanTracking::draggedAddToSelection:
    _draggedPos = event->pos();
    updateSelectionOverlay();
    drawFrame();
    break;
  case VulkanTracking::translateSelection:
  case VulkanTracking::measurement:
    break;
  default:
    _tracking = VulkanTracking::other;
    if (_startPoint)
    {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
      simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->pos().x(), event->pos().y());
#else
      simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->position().x(), event->position().y());
#endif
      if (std::shared_ptr<RKCamera> camera = _camera.lock())
      {
        camera->setTrackBallRotation(trackBallRotation);
      }
      drawFrame();
    }
    break;
  }
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent *event)
{
  switch (_tracking)
  {
  case VulkanTracking::none:
  case VulkanTracking::newSelection:
  case VulkanTracking::addToSelection:
  case VulkanTracking::draggedNewSelection:
  case VulkanTracking::draggedAddToSelection:
  case VulkanTracking::translateSelection:
  case VulkanTracking::measurement:
  case VulkanTracking::backgroundClick:
    break;
  default:
  {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->pos().x(), event->pos().y());
#else
    simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->position().x(), event->position().y());
#endif
    if (std::shared_ptr<RKCamera> camera = _camera.lock())
    {
      camera->setWorldRotation(trackBallRotation * camera->worldRotation());
      camera->setTrackBallRotation(simd_quatd(1.0, double3(0.0, 0.0, 0.0)));
    }
    break;
  }
  }

  _quality = RKRenderQuality::high;
  _tracking = VulkanTracking::none;
  updateSelectionOverlay();
  drawFrame();
}

void VulkanWindow::wheelEvent(QWheelEvent *event)
{
  _quality = RKRenderQuality::medium;
  _timer->start(500);
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->increaseDistance(event->angleDelta().y() / 40.0);
  }
  drawFrame();
}
