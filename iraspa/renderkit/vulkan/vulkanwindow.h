#pragma once

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include <QEvent>
#include <QPoint>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QWindow>

#include "rkrenderkitprotocols.h"
#include "trackball.h"
#include "vulkanscene.h"

enum class VulkanTracking
{
  none = 0,
  panning = 1,
  trucking = 2,
  addToSelection = 3,
  newSelection = 4,
  draggedAddToSelection = 5,
  draggedNewSelection = 6,
  backgroundClick = 7,
  measurement = 8,
  translateSelection = 9,
  other = 10
};

class VulkanWindow : public QWindow, public RKRenderViewController
{
  Q_OBJECT

public:
  explicit VulkanWindow(QWindow *parent = nullptr);
  ~VulkanWindow() override;
  void prepareForDestruction();

  const QStringList &logData() const override final { return _logData; }
  void redraw() override final;
  void redrawWithQuality(RKRenderQuality quality) override final;

  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures) override final;
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source) override final;
  void reloadData() override final;
  void reloadData(RKRenderQuality ambientOcclusionQuality) override final;
  void reloadAmbientOcclusionData() override final;
  void reloadRenderData() override final;
  void reloadSelectionData() override final;
  void reloadRenderMeasurePointsData() override final;
  void reloadBoundingBoxData() override final;
  void reloadGlobalAxesData() override final;
  void reloadBackgroundImage() override final;
  void invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures) override final;
  void invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures) override final;

  void updateTransformUniforms() override final;
  void updateStructureUniforms() override final;
  void updateIsosurfaceUniforms() override final;
  void updateLightUniforms() override final;
  void updateGlobalAxesUniforms() override final;
  void reloadStructureUniforms() override final;
  void updateVertexArrays() override final;

  QImage renderSceneToImage(int width, int height, RKRenderQuality quality) override final;
  std::array<int, 4> pickTexture(int x, int y, int width, int height) override final;
  std::optional<float> pickDepth(int x, int y, int width, int height) override final;
  void cycleRibbonAODebugMode();

signals:
  void rendererInitializationFailed(const QString &reason);

protected:
  bool event(QEvent *event) override;
  void exposeEvent(QExposeEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void initializeRenderer();
  void drawFrame();
  void scheduleFrame();
  void updateSelectionOverlay();

  // Declared before the renderer so that it outlives the VkInstance and the
  // VkSurfaceKHR the renderer borrows from it. <QVulkanInstance> is deliberately
  // not included here: it defines VK_NO_PROTOTYPES before pulling in vulkan.h,
  // which would undeclare the entry points the renderkit calls directly. QWindow
  // already forward declares the class.
#if QT_CONFIG(vulkan)
  std::unique_ptr<QVulkanInstance> _vulkanInstance;
#endif
  std::unique_ptr<VulkanRenderer> _renderer;
  std::unique_ptr<VulkanScene> _scene;

  bool _initialized = false;
  bool _initializationFailed = false;
  bool _destroyed = false;
  RKRenderQuality _quality = RKRenderQuality::high;
  std::shared_ptr<RKRenderDataSource> _dataSource;
  QStringList _logData{};
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures{};
  std::weak_ptr<RKCamera> _camera;

  TrackBall _trackBall{};
  std::optional<QPoint> _startPoint = std::nullopt;
  VulkanTracking _tracking = VulkanTracking::none;
  QPoint _origin;
  QPoint _draggedPos;
  QTimer *_timer = nullptr;
};
