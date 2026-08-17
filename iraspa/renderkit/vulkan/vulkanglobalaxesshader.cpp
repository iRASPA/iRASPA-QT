#include "vulkanglobalaxesshader.h"

#include "axessystemdefaultgeometry.h"
#include "backplanegeometry.h"
#include "rkfontatlas.h"
#include "rkglobalaxes.h"
#include "vulkanshader.h"

#include <QImage>
#include <QtGlobal>
#include <algorithm>
#include <cstddef>

#if defined(Q_OS_WIN)
static const QString kAxesFontName = QStringLiteral("Segoe UI");
#else
static const QString kAxesFontName = QStringLiteral("Helvetica");
#endif

namespace
{
VkVertexInputAttributeDescription axesAttribute(uint32_t location, uint32_t binding, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = binding;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

float4 colorToFloat4(const QColor &color)
{
  return float4(static_cast<float>(color.redF()), static_cast<float>(color.greenF()), static_cast<float>(color.blueF()),
                static_cast<float>(color.alphaF()));
}
}  // namespace

VulkanGlobalAxesShader::VulkanGlobalAxesShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanGlobalAxesShader::~VulkanGlobalAxesShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyGpuResources();
  if (_backgroundPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _backgroundPipeline, nullptr);
  }
  if (_systemPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _systemPipeline, nullptr);
  }
  if (_textPipeline)
  {
    vkDestroyPipeline(_renderer->device(), _textPipeline, nullptr);
  }
}

void VulkanGlobalAxesShader::destroyGpuResources()
{
  _renderer->destroyBuffer(_backgroundVertexBuffer);
  _renderer->destroyBuffer(_backgroundIndexBuffer);
  _renderer->destroyBuffer(_systemVertexBuffer);
  _renderer->destroyBuffer(_systemIndexBuffer);
  _renderer->destroyBuffer(_textInstanceBuffer);
  _renderer->destroyTexture(_fontTexture);
  _fontSamplerSet = VK_NULL_HANDLE;
  _backgroundIndexCount = 0;
  _systemIndexCount = 0;
  _textInstanceCount = 0;
}

void VulkanGlobalAxesShader::initialize()
{
  VulkanShader::PipelineConfig backgroundConfig;
  backgroundConfig.vertexShaderResource = QStringLiteral(":/shaders/global_axes_background.vert.spv");
  backgroundConfig.fragmentShaderResource = QStringLiteral(":/shaders/global_axes_background.frag.spv");
  backgroundConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  backgroundConfig.cullMode = VK_CULL_MODE_NONE;
  backgroundConfig.depthTest = true;
  backgroundConfig.depthWrite = true;
  backgroundConfig.depthCompareOp = VK_COMPARE_OP_ALWAYS;
  backgroundConfig.blend = true;
  backgroundConfig.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  backgroundConfig.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  VkVertexInputBindingDescription backgroundBinding{};
  backgroundBinding.binding = 0;
  backgroundBinding.stride = sizeof(RKVertex);
  backgroundBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  backgroundConfig.bindings = {backgroundBinding};
  backgroundConfig.attributes = {axesAttribute(0, 0, offsetof(RKVertex, position))};
  _backgroundPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), backgroundConfig);

  VulkanShader::PipelineConfig systemConfig;
  systemConfig.vertexShaderResource = QStringLiteral(":/shaders/global_axes_system.vert.spv");
  systemConfig.fragmentShaderResource = QStringLiteral(":/shaders/global_axes_system.frag.spv");
  systemConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  systemConfig.cullMode = VK_CULL_MODE_BACK_BIT;
  systemConfig.depthTest = true;
  systemConfig.depthWrite = true;
  systemConfig.depthClamp = true;
  VkVertexInputBindingDescription systemBinding{};
  systemBinding.binding = 0;
  systemBinding.stride = sizeof(RKPrimitiveVertex);
  systemBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  systemConfig.bindings = {systemBinding};
  systemConfig.attributes = {
      axesAttribute(0, 0, offsetof(RKPrimitiveVertex, position)),
      axesAttribute(1, 0, offsetof(RKPrimitiveVertex, normal)),
      axesAttribute(2, 0, offsetof(RKPrimitiveVertex, color)),
  };
  _systemPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), systemConfig);

  VulkanShader::PipelineConfig textConfig;
  textConfig.vertexShaderResource = QStringLiteral(":/shaders/global_axes_text.vert.spv");
  textConfig.fragmentShaderResource = QStringLiteral(":/shaders/global_axes_text.frag.spv");
  textConfig.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  textConfig.cullMode = VK_CULL_MODE_BACK_BIT;
  textConfig.depthTest = true;
  textConfig.depthWrite = false;
  textConfig.depthClamp = true;
  textConfig.blend = true;
  textConfig.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  textConfig.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  VkVertexInputBindingDescription textBinding{};
  textBinding.binding = 0;
  textBinding.stride = sizeof(AxesTextInstance);
  textBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  textConfig.bindings = {textBinding};
  textConfig.attributes = {
      axesAttribute(0, 0, offsetof(AxesTextInstance, position)),
      axesAttribute(1, 0, offsetof(AxesTextInstance, scale)),
      axesAttribute(2, 0, offsetof(AxesTextInstance, vertexCoordinatesData)),
      axesAttribute(3, 0, offsetof(AxesTextInstance, textureCoordinatesData)),
      axesAttribute(4, 0, offsetof(AxesTextInstance, axisId)),
  };
  _textPipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), textConfig);
}

void VulkanGlobalAxesShader::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  _dataSource = std::move(source);
  reloadData();
}

void VulkanGlobalAxesShader::reloadData()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyGpuResources();
  if (!_dataSource)
  {
    return;
  }

  BackPlaneGeometry background;
  const auto &backgroundVertices = background.vertices();
  const auto &backgroundIndices = background.indices();
  _backgroundIndexCount = static_cast<uint32_t>(backgroundIndices.size());
  _renderer->uploadBuffer(_backgroundVertexBuffer, backgroundVertices.data(), backgroundVertices.size() * sizeof(RKVertex),
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_backgroundIndexBuffer, backgroundIndices.data(), backgroundIndices.size() * sizeof(short),
                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  std::shared_ptr<RKGlobalAxes> axes = _dataSource->axes();
  AxesSystemDefaultGeometry geometry(
      axes->centerType(), axes->centerScale(), colorToFloat4(axes->centerDiffuseColor()), axes->shaftLength(), axes->shaftWidth(),
      colorToFloat4(axes->axisXDiffuseColor()), colorToFloat4(axes->axisYDiffuseColor()), colorToFloat4(axes->axisZDiffuseColor()),
      axes->tipLength(), axes->tipWidth(), colorToFloat4(axes->axisXDiffuseColor()), colorToFloat4(axes->axisYDiffuseColor()),
      colorToFloat4(axes->axisZDiffuseColor()), axes->tipVisibility(), axes->aspectRatio(), axes->NumberOfSectors());
  const auto &systemVertices = geometry.vertices();
  const auto &systemIndices = geometry.indices();
  _systemIndexCount = static_cast<uint32_t>(systemIndices.size());
  if (!systemVertices.empty())
  {
    _renderer->uploadBuffer(_systemVertexBuffer, systemVertices.data(), systemVertices.size() * sizeof(RKPrimitiveVertex),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
  if (!systemIndices.empty())
  {
    _renderer->uploadBuffer(_systemIndexBuffer, systemIndices.data(), systemIndices.size() * sizeof(short),
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  }

  RKFontAtlas atlas(kAxesFontName, 256);
  QImage fontImage(static_cast<int>(atlas.width), static_cast<int>(atlas.height), QImage::Format_RGBA8888);
  for (int y = 0; y < fontImage.height(); ++y)
  {
    uchar *dst = fontImage.scanLine(y);
    const unsigned char *src = atlas.textureData.data() + static_cast<size_t>(y) * static_cast<size_t>(fontImage.width());
    for (int x = 0; x < fontImage.width(); ++x)
    {
      dst[x * 4 + 0] = src[x];
      dst[x * 4 + 1] = src[x];
      dst[x * 4 + 2] = src[x];
      dst[x * 4 + 3] = 255;
    }
  }
  _fontTexture = _renderer->createTextureRGBA8(fontImage);
  _fontSamplerSet = _renderer->allocateSamplerDescriptorSet(_fontTexture);

  auto appendAxis = [&](std::vector<AxesTextInstance> &out, const QString &label, int axisId) {
    const auto glyphs = atlas.buildMeshWithString(float4(axisId == 0 ? 1.0f : 0.0f, axisId == 1 ? 1.0f : 0.0f, axisId == 2 ? 1.0f : 0.0f, 1.0f),
                                                  float4(3.0f, 3.0f, 3.0f, 3.0f), label, RKTextAlignment::center);
    for (const auto &glyph : glyphs)
    {
      AxesTextInstance instance{};
      instance.position = glyph.position;
      instance.scale = glyph.scale;
      instance.vertexCoordinatesData = glyph.vertexCoordinatesData;
      instance.textureCoordinatesData = glyph.textureCoordinatesData;
      instance.axisId = float4(static_cast<float>(axisId), 0.0f, 0.0f, 0.0f);
      out.push_back(instance);
    }
  };

  std::vector<AxesTextInstance> textInstances;
  appendAxis(textInstances, QStringLiteral("X"), 0);
  appendAxis(textInstances, QStringLiteral("Y"), 1);
  appendAxis(textInstances, QStringLiteral("Z"), 2);
  _textInstanceCount = static_cast<uint32_t>(textInstances.size());
  if (!textInstances.empty())
  {
    _renderer->uploadBuffer(_textInstanceBuffer, textInstances.data(), textInstances.size() * sizeof(AxesTextInstance),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  }
}

void VulkanGlobalAxesShader::setAxesViewport(VkCommandBuffer commandBuffer, float x, float yOpenGL, float width, float height) const
{
  const float framebufferHeight = static_cast<float>(_renderer->swapChainExtent().height);
  VkViewport viewport{};
  viewport.x = x;
  viewport.y = framebufferHeight - yOpenGL - height;
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {static_cast<int32_t>(std::max(0.0f, viewport.x)), static_cast<int32_t>(std::max(0.0f, viewport.y))};
  scissor.extent = {static_cast<uint32_t>(std::max(0.0f, viewport.width)), static_cast<uint32_t>(std::max(0.0f, viewport.height))};
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanGlobalAxesShader::restoreViewport(VkCommandBuffer commandBuffer) const
{
  const VkExtent2D extent = _renderer->swapChainExtent();
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(extent.width);
  viewport.height = static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VulkanGlobalAxesShader::paint(VkCommandBuffer commandBuffer)
{
  if (!_dataSource || !_backgroundPipeline || !_systemPipeline || !_textPipeline)
  {
    return;
  }
  std::shared_ptr<RKGlobalAxes> axes = _dataSource->axes();
  if (!axes || axes->position() == RKGlobalAxes::Position::none)
  {
    return;
  }

  const float width = static_cast<float>(_renderer->swapChainExtent().width);
  const float height = static_cast<float>(_renderer->swapChainExtent().height);
  const float minSize = std::min(width, height);
  const float border = minSize * static_cast<float>(axes->borderOffsetScreenFraction());
  const float size = minSize * static_cast<float>(axes->sizeScreenFraction());
  float x = border;
  float y = border;
  switch (axes->position())
  {
  case RKGlobalAxes::Position::bottomLeft:
    x = border;
    y = border;
    break;
  case RKGlobalAxes::Position::midLeft:
    x = border;
    y = 0.5f * minSize - 0.5f * size;
    break;
  case RKGlobalAxes::Position::topLeft:
    x = border;
    y = height - (border + size);
    break;
  case RKGlobalAxes::Position::midTop:
    x = 0.5f * width - 0.5f * size;
    y = height - (border + size);
    break;
  case RKGlobalAxes::Position::topRight:
    x = width - (border + size);
    y = height - (border + size);
    break;
  case RKGlobalAxes::Position::midRight:
    x = width - (border + size);
    y = 0.5f * minSize - 0.5f * size;
    break;
  case RKGlobalAxes::Position::bottomRight:
    x = width - (border + size);
    y = border;
    break;
  case RKGlobalAxes::Position::midBottom:
    x = 0.5f * width - 0.5f * size;
    y = border;
    break;
  case RKGlobalAxes::Position::center:
    x = 0.5f * width - 0.5f * size;
    y = 0.5f * minSize - 0.5f * size;
    break;
  default:
    restoreViewport(commandBuffer);
    return;
  }

  setAxesViewport(commandBuffer, x, y, size, size);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _backgroundPipeline);
  _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), 0);
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_backgroundVertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(commandBuffer, _backgroundIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(commandBuffer, _backgroundIndexCount, 1, 0, 0, 0);

  if (_systemIndexCount > 0 && _systemVertexBuffer.buffer)
  {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _systemPipeline);
    _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), 0);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_systemVertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, _systemIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, _systemIndexCount, 1, 0, 0, 0);
  }

  if (_textInstanceCount > 0 && _textInstanceBuffer.buffer && _fontSamplerSet)
  {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _textPipeline);
    _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), 0);
    _renderer->bindSamplerDescriptorSet(commandBuffer, _renderer->scenePipelineLayout(), _fontSamplerSet);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_textInstanceBuffer.buffer, &offset);
    vkCmdDraw(commandBuffer, 4, _textInstanceCount, 0, 0);
  }

  restoreViewport(commandBuffer);
}
