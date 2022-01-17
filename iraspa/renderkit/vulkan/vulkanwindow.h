#ifndef VKWINDOW_H
#define VKWINDOW_H
#include <qtimer.h>
#include <vulkanrenderer.h>
#include <QResizeEvent>
#include <QWindow>
#include "rkrenderkitprotocols.h"
#include "trackball.h"

class VulkanWindow : public QWindow, public RKRenderViewController
{
    enum class Tracking
    {
      none = 0,
      panning = 1,                   // alt + right-mouse drag
      trucking = 2,                  // ctrl + right-mouse drag
      addToSelection = 3,            // command-key
      newSelection = 4,              // shift-key
      draggedAddToSelection = 5,     // drag + command
      draggedNewSelection = 6,       // drag + shift
      backgroundClick = 7,
      measurement = 8,               // alt-key
      translateSelection = 9,        // alt and command-key
      other = 10
    };

   public:
    VulkanWindow(QWindow *parent);
    virtual ~VulkanWindow();

    void resize(int w, int h);
    void drawFrame();

    void mousePressEvent(QMouseEvent *event) override final;
    void mouseMoveEvent(QMouseEvent *event) override final;
    void mouseReleaseEvent(QMouseEvent *event) override final;
    void wheelEvent(QWheelEvent *event) override final;

    virtual void redraw() override final;
    virtual void redrawWithQuality(RKRenderQuality quality)  override final;

    virtual void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)  override final;
    virtual void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)  override final;
    virtual void reloadData()  override final;
    virtual void reloadData(RKRenderQuality ambientOcclusionQuality)  override final;
    virtual void reloadAmbientOcclusionData()  override final;
    virtual void reloadRenderData()  override final;
    virtual void reloadSelectionData()  override final;
    virtual void reloadRenderMeasurePointsData() override final;
    virtual void reloadBoundingBoxData() override final;
    virtual void reloadGlobalAxesData() override final;

    virtual void reloadBackgroundImage()  override final;

    virtual void invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures) override final;
    virtual void invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures) override final;
    virtual void computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderObject>> structures)  override final;
    virtual void computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderObject>> structures)  override final;

    virtual void updateTransformUniforms()  override final;
    virtual void updateStructureUniforms()  override final;
    virtual void updateIsosurfaceUniforms()  override final;
    virtual void updateLightUniforms()  override final;
    virtual void updateGlobalAxesUniforms() override final;

    virtual void updateVertexArrays() override final;

    virtual QImage renderSceneToImage(int width, int height, RKRenderQuality quality)  override final;

    virtual std::array<int,4> pickTexture(int x, int y, int width, int height)  override final;
   protected:
    void resizeEvent(QResizeEvent* event);

    VulkanRenderer* vulkanRenderer = nullptr;
    QTimer renderTimer;

    uint32_t windowWidth = 800;
    uint32_t windowHeigh = 600;

    std::shared_ptr<RKRenderDataSource> _dataSource;
    LogReporting* _logReporter = nullptr;
    QStringList _logData{};

    std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures = std::vector<std::vector<std::shared_ptr<RKRenderObject>>>{};

    std::shared_ptr<RKCamera> _camera = std::make_shared<RKCamera>();
    TrackBall _trackBall = TrackBall();

    std::optional<QPoint> _startPoint = std::nullopt;
    std::optional<QPoint> _panStartPoint = std::nullopt;

    Tracking _tracking = Tracking::none;
    QPoint _origin;
    QPoint _draggedPos;

};

#endif  // VKWINDOW_H

