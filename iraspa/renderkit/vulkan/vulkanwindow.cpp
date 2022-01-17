#include "vulkanwindow.h"

#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#ifdef VK_USE_PLATFORM_MACOS_MVK
extern "C" {
void makeViewMetalCompatible(void* handle);
}
#endif

VulkanWindow::VulkanWindow(QWindow *parent) : QWindow(parent) {
#ifdef VK_USE_PLATFORM_MACOS_MVK
    makeViewMetalCompatible(reinterpret_cast<void*>(winId()));
    const void* viewId = reinterpret_cast<void*>(winId());
    vulkanRenderer = new VulkanRenderer(viewId);
#elif VK_USE_PLATFORM_WIN32_KHR
    vulkanRenderer = new VulkanRenderer(reinterpret_cast<HWND>(winId()));
#endif

    {
        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
#if defined(Q_OS_WIN)
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "C:/Users/ddubb/Porsche_911_GT2.obj")) {
                   throw std::runtime_error(err);
#else
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "/Users/dubbelda/Research/Codes/QtVulkan/Porsche_911_GT2.obj")) {
                   throw std::runtime_error(err);
#endif
        }
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                //                Vertex vertex = {};
                glm::vec3 pos = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
                                 attrib.vertices[3 * index.vertex_index + 2]};
                vertices.push_back(pos);
                indices.push_back(indices.size());
            }
        }
        vulkanRenderer->addGeometry(vertices, indices, 1);
        vulkanRenderer->recreateVertexBuffer();
        vulkanRenderer->recreateIndexBuffer();

        std::vector<InstanceData> instData;
        InstanceData a;
        a.transform = glm::mat4(1.0f);
        a.color = glm::vec4(1.0f);
        instData.push_back(a);
        vulkanRenderer->updateInstanceBuffer(instData);

        std::vector<DrawItem> drawItems;
        DrawItem b;
        b.meshId = 1;
        b.instanceBase = 0;
        b.instanceCount = 1;
        drawItems.push_back(b);
        vulkanRenderer->buildCommandBuffers(drawItems);
    }

    {
        glm::vec3 camPos(2, 2, 3);
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

        glm::mat4x4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4x4 proj = glm::perspective(glm::radians(45.0f), windowWidth / (float)windowHeigh, 0.1f, 1000.0f);
        proj[1][1] *= -1;

        vulkanRenderer->updateCamera(view, proj, camPos, worldUp);
        vulkanRenderer->updateUniformBuffer(_camera);
    }

    drawFrame();

    //renderTimer.setInterval(16);
    //QObject::connect(&renderTimer, &QTimer::timeout, this, &VkWindow::drawFrame);
    //renderTimer.start();
}

void VulkanWindow::drawFrame() {
    Q_ASSERT(vulkanRenderer != nullptr);
    vulkanRenderer->drawFrame(_camera);
}

VulkanWindow::~VulkanWindow() {
    delete vulkanRenderer;
}

void VulkanWindow::resizeEvent(QResizeEvent* event)
{
  QWindow::resizeEvent(event);
  if(vulkanRenderer)
  {
    if (windowWidth != event->size().width() || windowHeigh != event->size().height()) {
        windowWidth = event->size().width();
        windowHeigh = event->size().height();
        vulkanRenderer->resize(windowWidth, windowHeigh);
        drawFrame();
    }
  }
}

void VulkanWindow::redraw()
{

}

void VulkanWindow::redrawWithQuality(RKRenderQuality quality)
{

}

void VulkanWindow::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = structures;
}
void VulkanWindow::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  qDebug() << "setRenderDataSource";

  _dataSource = source;

  if(std::shared_ptr<RKRenderDataSource> dataSource = source)
  {
    //_camera = dataSource->camera();

     _camera->updateCameraForWindowResize(this->size().width(),this->size().height());
     _camera->resetForNewBoundingBox(dataSource->renderBoundingBox());
  }
  drawFrame();
}
void VulkanWindow::reloadData()
{

}
void VulkanWindow::reloadData(RKRenderQuality ambientOcclusionQuality)
{

}
void VulkanWindow::reloadAmbientOcclusionData()
{

}
void VulkanWindow::reloadRenderData()
{

}
void VulkanWindow::reloadSelectionData()
{

}
void VulkanWindow::reloadRenderMeasurePointsData()
{

}

void VulkanWindow::reloadBoundingBoxData()
{

}

void VulkanWindow::reloadGlobalAxesData()
{

}

void VulkanWindow::reloadBackgroundImage()
{

}

void VulkanWindow::invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}
void VulkanWindow::invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}
void VulkanWindow::computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}
void VulkanWindow::computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}

void VulkanWindow::updateTransformUniforms()
{
  vulkanRenderer->updateUniformBuffer(_camera);
}
void VulkanWindow::updateStructureUniforms()
{

}
void VulkanWindow::updateIsosurfaceUniforms()
{

}
void VulkanWindow::updateLightUniforms()
{

}

void VulkanWindow::updateGlobalAxesUniforms()
{

}

void VulkanWindow::updateVertexArrays()
{

}

QImage VulkanWindow::renderSceneToImage(int width, int height, RKRenderQuality quality)
{
    return QImage();
}

std::array<int,4> VulkanWindow::pickTexture(int x, int y, int width, int height)
{
    std::array<int,4> pixel;
    return pixel;
}

void VulkanWindow::mousePressEvent(QMouseEvent *event)
{
  //_timer->stop();

  _tracking = Tracking::none;

  _startPoint = event->pos();
  _origin = event->pos();
  _draggedPos = event->pos();

  //_quality = RKRenderQuality::medium;

   _camera->setTrackBallRotation(simd_quatd(1.0, double3(0.0, 0.0, 0.0)));


  if(event->modifiers() & Qt::ShiftModifier)   // contains shift
  {
    // Using the shift key means a new selection is chosen. If later a drag occurs it is modified to 'draggedNewSelection'
    _tracking = Tracking::newSelection;
  }
  else if(event->modifiers()  == Qt::ControlModifier)  // command, not option
  {
    // Using the command key means the selection is extended. If later a drag occurs it is modified to 'draggedAddToSelection'
    _tracking = Tracking::addToSelection;

  }
  else if(event->modifiers()  & Qt::AltModifier & Qt::ControlModifier)  // option and command
  {
    _tracking = Tracking::translateSelection;
  }
  else if(event->modifiers()  == Qt::AltModifier) // option, not command
  {
    _tracking = Tracking::measurement;
    #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
      _trackBall.start(event->pos().x(),event->pos().y(), 0, 0, this->width(), this->height());
    #else
      _trackBall.start(event->position().x(),event->position().y(), 0, 0, this->width(), this->height());
    #endif
  }
  else
  {
      qDebug() << "background click";
    _tracking = Tracking::backgroundClick;
    #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
      _trackBall.start(event->pos().x(),event->pos().y(), 0, 0, this->width(), this->height());
    #else
      _trackBall.start(event->position().x(),event->position().y(), 0, 0, this->width(), this->height());
    #endif
  }

  drawFrame();
}


void VulkanWindow::mouseMoveEvent(QMouseEvent *event)
{
  //qDebug() << "mouse move";
  switch(_tracking)
  {
    case Tracking::none:
      break;
    case Tracking::newSelection:
      // convert to dragged version
      _tracking = Tracking::draggedNewSelection;
      //_draggedPos = event->pos();
      break;
    case Tracking::addToSelection:
       // convert to dragged version
       _tracking = Tracking::draggedAddToSelection;
       //_draggedPos = event->pos();
       break;
    case Tracking::draggedNewSelection:
      //_draggedPos = event->pos();
       break;
    case Tracking::draggedAddToSelection:
      _draggedPos = event->pos();
      break;
    case Tracking::translateSelection:
       break;
    case Tracking::measurement:
       break;
    default:
       _tracking =Tracking::other;
       if(_startPoint)
       {
         #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
           simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->pos().x(), event->pos().y());
         #else
           simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->position().x(), event->position().y());
         #endif

         _camera->setTrackBallRotation(trackBallRotation);
         drawFrame();
       }
       break;
  }


}

void VulkanWindow::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "Mouse release event";
  switch(_tracking)
  {
    case Tracking::none:
      break;
    case Tracking::newSelection:
      break;
    case Tracking::addToSelection:
      break;
    case Tracking::draggedNewSelection:
      break;
    case Tracking::draggedAddToSelection:
      break;
    case Tracking::translateSelection:
      break;
    case Tracking::measurement:
      break;
    case Tracking::backgroundClick:
      break;
    default:
      #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->pos().x(), event->pos().y() );
      #else
        simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->position().x(), event->position().y() );
      #endif

        qDebug() << "set orientation";
      simd_quatd worldRotation = _camera->worldRotation();
      _camera->setWorldRotation(trackBallRotation * worldRotation);
      _camera->setTrackBallRotation(simd_quatd(1.0, double3(0.0, 0.0, 0.0)));
      break;
  }

  //_quality = RKRenderQuality::high;
  _tracking = Tracking::none;

  drawFrame();
}

void VulkanWindow::wheelEvent(QWheelEvent *event)
{
  //_quality = RKRenderQuality::medium;
  //_timer->start(500);
  //makeCurrent();

   _camera->increaseDistance(event->angleDelta().y()/40.0);

  drawFrame();
}
