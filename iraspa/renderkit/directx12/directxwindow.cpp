#include "directxwindow.h"


DirectXWindow::DirectXWindow()
    : f(Q_NULLPTR),
      green(1.0)
{
}

DirectXWindow::~DirectXWindow()
{
    delete f;
}

void DirectXWindow::initializeD3D()
{
    f = createFence();
    ID3D12Device *dev = device();
    if (FAILED(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator(), Q_NULLPTR, IID_PPV_ARGS(&commandList)))) {
        qWarning("Failed to create command list");
        return;
    }
    commandList->Close();
}

void DirectXWindow::resizeD3D(const QSize &size)
{
    qDebug("resize %d %d", size.width(), size.height());
}

void DirectXWindow::paintD3D()
{
    commandAllocator()->Reset();
    commandList->Reset(commandAllocator(), Q_NULLPTR);

    transitionResource(backBufferRenderTarget(), commandList.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    //green += 0.01f;
    //if (green > 1.0f)
    //    green = 0.0f;
    const float clearColor[] = { 0.0f, green, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(backBufferRenderTargetCPUHandle(), clearColor, 0, Q_NULLPTR);

    transitionResource(backBufferRenderTarget(), commandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->Close();

    ID3D12CommandList *commandLists[] = { commandList.Get() };
    commandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);



   // update(); // schedule the next frame by posting an UpdateRequest event
}




void DirectXWindow::afterPresent()
{
    waitForGPU(f);
}

void DirectXWindow::redraw()
{

}

void DirectXWindow::redrawWithQuality(RKRenderQuality quality)
{

}

void DirectXWindow::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = structures;
}
void DirectXWindow::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  qDebug() << "setRenderDataSource";

  _dataSource = source;

  if(std::shared_ptr<RKRenderDataSource> dataSource = source)
  {
    _camera = dataSource->camera();
    if (std::shared_ptr<RKCamera> camera = _camera.lock())
    {
      camera->updateCameraForWindowResize(this->size().width(),this->size().height());
      camera->resetForNewBoundingBox(dataSource->renderBoundingBox());
    }
  }
}
void DirectXWindow::reloadData()
{

}
void DirectXWindow::reloadData(RKRenderQuality ambientOcclusionQuality)
{

}
void DirectXWindow::reloadAmbientOcclusionData()
{

}
void DirectXWindow::reloadRenderData()
{

}
void DirectXWindow::reloadSelectionData()
{

}
void DirectXWindow::reloadRenderMeasurePointsData()
{

}

void DirectXWindow::reloadBoundingBoxData()
{

}

void DirectXWindow::reloadGlobalAxesData()
{

}

void DirectXWindow::reloadBackgroundImage()
{

}

void DirectXWindow::invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}
void DirectXWindow::invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}
void DirectXWindow::computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}
void DirectXWindow::computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderObject>> structures)
{

}

void DirectXWindow::updateTransformUniforms()
{

}
void DirectXWindow::updateStructureUniforms()
{

}
void DirectXWindow::updateIsosurfaceUniforms()
{

}
void DirectXWindow::updateLightUniforms()
{

}

void DirectXWindow::updateGlobalAxesUniforms()
{

}

void DirectXWindow::updateVertexArrays()
{

}

QImage DirectXWindow::renderSceneToImage(int width, int height, RKRenderQuality quality)
{
    return QImage();
}

std::array<int,4> DirectXWindow::pickTexture(int x, int y, int width, int height)
{
    std::array<int,4> pixel;
    return pixel;
}
