#include "iraspaexport.h"

#include "rkcamera.h"
#include "rkrendererbackend.h"
#include "scenelist.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <memory>

#include <QDebug>
#include <QImage>

#if defined(USE_VULKAN)
#include "vulkanrenderer.h"
#include "vulkanscene.h"
#endif
#if defined(USE_OPENGL)
#include "opengloffscreenrenderer.h"
#endif

namespace
{

constexpr int kCameraMovieFrames = 120;
constexpr double kRotationYStep = -3.0 * M_PI / 180.0;
constexpr double kLemniscateYaw = M_PI;
constexpr double kLemniscatePitch = M_PI / 2.0;

int nearestEvenInt(int value)
{
  return (value % 2 == 0) ? value : (value + 1);
}

std::shared_ptr<ProjectStructure> projectFromNode(const std::shared_ptr<ProjectTreeNode> &node)
{
  if (!node || !node->representedObject())
  {
    return nullptr;
  }
  return std::dynamic_pointer_cast<ProjectStructure>(node->representedObject()->project());
}

template <typename Renderer>
void bindCopiedProject(Renderer &renderer, std::shared_ptr<ProjectStructure> project, int width, int height)
{
  project->setInitialSelectionIfNeeded();
  std::shared_ptr<SceneList> sceneList = project->sceneList();
  if (!sceneList)
  {
    throw std::runtime_error("the project has no scene list");
  }

  std::shared_ptr<RKCamera> camera = project->camera();
  if (camera)
  {
    camera->updateViewMatrix();
  }
  const RKCamera savedCamera = camera ? *camera : RKCamera();

  renderer.setRenderStructures(sceneList->selectediRASPARenderStructures());
  renderer.setRenderDataSource(project);

  if (camera)
  {
    *camera = savedCamera;
    camera->updateViewMatrix();
    camera->updateCameraForWindowResize(width, height);
  }
  renderer.reloadData(RKRenderQuality::picture);
}

template <typename Renderer>
QImage renderCopiedPicture(Renderer &renderer, std::shared_ptr<ProjectStructure> project, int width, int height)
{
  bindCopiedProject(renderer, project, width, height);
  return renderer.renderSceneToImage(width, height, RKRenderQuality::picture);
}

template <typename Renderer>
void renderCopiedMovie(Renderer &renderer, std::shared_ptr<ProjectStructure> project, QUrl fileURL, int width, int height,
                       MovieWriter::Format format, ProjectStructure::MovieType movieType)
{
  const int evenWidth = nearestEvenInt(width);
  const int evenHeight = nearestEvenInt(height);
  bindCopiedProject(renderer, project, evenWidth, evenHeight);

  MovieWriter movie(evenWidth, evenHeight, project->movieFramesPerSecond(), nullptr, format);
  if (movie.initialize(fileURL.toLocalFile().toStdString()) < 0)
  {
    throw std::runtime_error("failed to create movie");
  }

  std::shared_ptr<RKCamera> camera = project->camera();
  const simd_quatd startRotation = camera ? camera->worldRotation() : simd_quatd(1.0, double3(0.0, 0.0, 0.0));
  const int frameCount = (movieType == ProjectStructure::MovieType::frames)
                             ? std::max(1, static_cast<int>(project->maxNumberOfMoviesFrames()))
                             : kCameraMovieFrames;

  for (int frame = 0; frame < frameCount; ++frame)
  {
    const double t = 2.0 * M_PI * static_cast<double>(frame) / static_cast<double>(std::max(1, frameCount));
    switch (movieType)
    {
    case ProjectStructure::MovieType::frames:
      if (std::shared_ptr<SceneList> sceneList = project->sceneList())
      {
        sceneList->setSelectedFrameIndex(static_cast<size_t>(frame));
        renderer.setRenderStructures(sceneList->selectediRASPARenderStructures());
        renderer.reloadData(RKRenderQuality::picture);
      }
      break;
    case ProjectStructure::MovieType::rotationY:
      if (camera)
      {
        camera->setWorldRotation(startRotation);
        camera->rotateCameraAroundAxisY(kRotationYStep * frame);
      }
      break;
    case ProjectStructure::MovieType::rotationXYlemniscate:
      if (camera)
      {
        camera->setWorldRotation(startRotation);
        camera->rotateCameraAroundAxisY(kLemniscateYaw * std::sin(t));
        camera->rotateCameraAroundAxisX(kLemniscatePitch * std::sin(t) * std::cos(t));
      }
      break;
    }

    const QImage image = renderer.renderSceneToImage(evenWidth, evenHeight, RKRenderQuality::picture);
    if (image.isNull())
    {
      throw std::runtime_error("the renderer produced no image");
    }
    movie.addFrame(image.bits(), static_cast<size_t>(frame));
  }
  movie.finalize();
}

} // namespace

void runOffscreenPictureExport(std::shared_ptr<ProjectTreeNode> nodeCopy, QUrl fileURL, int width, int height,
                               RKRendererBackend backend)
{
  try
  {
    std::shared_ptr<ProjectStructure> project = projectFromNode(nodeCopy);
    if (!project)
    {
      throw std::runtime_error("the copied project is not a structure project");
    }
    QImage image;
    if (backend == RKRendererBackend::OpenGL)
    {
#if defined(USE_OPENGL)
      OpenGLOffscreenRenderer renderer(width, height);
      image = renderCopiedPicture(renderer, project, width, height);
#else
      throw std::runtime_error("OpenGL is not available for export");
#endif
    }
    else
    {
#if defined(USE_VULKAN)
      VulkanRenderer renderer(static_cast<uint32_t>(std::max(1, width)), static_cast<uint32_t>(std::max(1, height)));
      VulkanScene scene(&renderer);
      scene.initialize();
      image = renderCopiedPicture(scene, project, width, height);
#else
      throw std::runtime_error("Vulkan is not available for export");
#endif
    }
    if (image.isNull() || !image.save(fileURL.toLocalFile()))
    {
      throw std::runtime_error("failed to write the picture file");
    }
  }
  catch (const std::exception &e)
  {
    qWarning("Offscreen picture export failed: %s", e.what());
  }
}

void runOffscreenMovieExport(std::shared_ptr<ProjectTreeNode> nodeCopy, QUrl fileURL, int width, int height,
                             MovieWriter::Format format, ProjectStructure::MovieType movieType,
                             RKRendererBackend backend)
{
  try
  {
    std::shared_ptr<ProjectStructure> project = projectFromNode(nodeCopy);
    if (!project)
    {
      throw std::runtime_error("the copied project is not a structure project");
    }
    if (backend == RKRendererBackend::OpenGL)
    {
#if defined(USE_OPENGL)
      OpenGLOffscreenRenderer renderer(nearestEvenInt(width), nearestEvenInt(height));
      renderCopiedMovie(renderer, project, fileURL, width, height, format, movieType);
#else
      throw std::runtime_error("OpenGL is not available for export");
#endif
    }
    else
    {
#if defined(USE_VULKAN)
      VulkanRenderer renderer(static_cast<uint32_t>(std::max(1, nearestEvenInt(width))),
                              static_cast<uint32_t>(std::max(1, nearestEvenInt(height))));
      VulkanScene scene(&renderer);
      scene.initialize();
      renderCopiedMovie(scene, project, fileURL, width, height, format, movieType);
#else
      throw std::runtime_error("Vulkan is not available for export");
#endif
    }
  }
  catch (const std::exception &e)
  {
    qWarning("Offscreen movie export failed: %s", e.what());
  }
}
