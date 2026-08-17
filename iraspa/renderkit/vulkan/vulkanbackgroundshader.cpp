#include "vulkanbackgroundshader.h"

#include "backplanegeometry.h"
#include "vulkanshader.h"

#include <cstddef>
#include <QColor>
#include <QImage>

VulkanBackgroundShader::VulkanBackgroundShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanBackgroundShader::~VulkanBackgroundShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  if (_pipeline)
  {
    vkDestroyPipeline(_renderer->device(), _pipeline, nullptr);
  }
  _renderer->destroyBuffer(_vertexBuffer);
  _renderer->destroyBuffer(_indexBuffer);
  _renderer->destroyTexture(_texture);
}

void VulkanBackgroundShader::initialize()
{
  BackPlaneGeometry quad;
  const auto vertices = quad.vertices();
  const auto indices = quad.indices();
  _indexCount = static_cast<uint32_t>(indices.size());
  _renderer->uploadBuffer(_vertexBuffer, vertices.data(), vertices.size() * sizeof(RKVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  _renderer->uploadBuffer(_indexBuffer, indices.data(), indices.size() * sizeof(short), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = QStringLiteral(":/shaders/background.vert.spv");
  config.fragmentShaderResource = QStringLiteral(":/shaders/background.frag.spv");
  config.cullMode = VK_CULL_MODE_NONE;
  config.depthTest = false;
  config.depthWrite = false;
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  VkVertexInputBindingDescription binding{};
  binding.binding = 0;
  binding.stride = sizeof(RKVertex);
  binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  config.bindings = {binding};

  VkVertexInputAttributeDescription attribute{};
  attribute.binding = 0;
  attribute.location = 0;
  attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  attribute.offset = offsetof(RKVertex, position);
  config.attributes = {attribute};

  _pipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->backgroundPipelineLayout(), config);

  QImage image(64, 64, QImage::Format_RGBA8888);
  image.fill(QColor(255, 255, 255, 255));
  _texture = _renderer->createTextureRGBA8(image);
  _samplerSet = _renderer->allocateSamplerDescriptorSet(_texture);
}

void VulkanBackgroundShader::reload(std::shared_ptr<RKRenderDataSource> source)
{
  QImage image;
  if (source)
  {
    image = source->renderBackgroundCachedImage().convertToFormat(QImage::Format_RGBA8888);
  }
  else
  {
    image = QImage(64, 64, QImage::Format_RGBA8888);
    image.fill(QColor(255, 255, 255, 255));
  }

  _renderer->waitIdle();
  _renderer->destroyTexture(_texture);
  _texture = _renderer->createTextureRGBA8(image);
  _samplerSet = _renderer->allocateSamplerDescriptorSet(_texture);
}

void VulkanBackgroundShader::paint(VkCommandBuffer commandBuffer)
{
  if (!_pipeline || _indexCount == 0)
  {
    return;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _renderer->backgroundPipelineLayout(), 0, 1, &_samplerSet, 0, nullptr);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_vertexBuffer.buffer, &offset);
  vkCmdBindIndexBuffer(commandBuffer, _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(commandBuffer, _indexCount, 1, 0, 0, 0);
}
