#include "vulkanatomtextshader.h"

#include "rkfontatlas.h"
#include "vulkanshader.h"

#include <QImage>
#include <QString>
#include <cstddef>

namespace
{
VkVertexInputAttributeDescription textAttribute(uint32_t location, uint32_t offset)
{
  VkVertexInputAttributeDescription description{};
  description.location = location;
  description.binding = 0;
  description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  description.offset = offset;
  return description;
}

QImage fontAtlasImage(const RKFontAtlas &atlas)
{
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
  return fontImage;
}
}  // namespace

VulkanAtomTextShader::VulkanAtomTextShader(VulkanRenderer *renderer) : _renderer(renderer)
{
}

VulkanAtomTextShader::~VulkanAtomTextShader()
{
  if (!_renderer)
  {
    return;
  }
  _renderer->waitIdle();
  destroyStructureBuffers();
  destroyFonts();
  if (_pipeline)
  {
    vkDestroyPipeline(_renderer->device(), _pipeline, nullptr);
  }
}

void VulkanAtomTextShader::destroyStructureBuffers()
{
  for (auto &scene : _structureBuffers)
  {
    for (auto &buffers : scene)
    {
      _renderer->destroyBuffer(buffers.instanceBuffer);
    }
  }
  _structureBuffers.clear();
}

void VulkanAtomTextShader::destroyFonts()
{
  for (auto &entry : _fonts)
  {
    if (entry.second.samplerSet)
    {
      vkFreeDescriptorSets(_renderer->device(), _renderer->descriptorPool(), 1, &entry.second.samplerSet);
    }
    _renderer->destroyTexture(entry.second.texture);
  }
  _fonts.clear();
}

VulkanAtomTextShader::FontGpu &VulkanAtomTextShader::fontForName(const QString &fontName)
{
  auto found = _fonts.find(fontName);
  if (found != _fonts.end())
  {
    return found->second;
  }

  FontGpu gpu;
  gpu.atlas = std::make_unique<RKFontAtlas>(fontName, 256);
  gpu.texture = _renderer->createTextureRGBA8(fontAtlasImage(*gpu.atlas));
  gpu.samplerSet = _renderer->allocateSamplerDescriptorSet(gpu.texture);
  auto inserted = _fonts.emplace(fontName, std::move(gpu));
  return inserted.first->second;
}

void VulkanAtomTextShader::initialize()
{
  VulkanShader::PipelineConfig config;
  config.vertexShaderResource = QStringLiteral(":/shaders/atom_text.vert.spv");
  config.fragmentShaderResource = QStringLiteral(":/shaders/atom_text.frag.spv");
  config.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  config.cullMode = VK_CULL_MODE_BACK_BIT;
  config.depthTest = true;
  config.depthWrite = false;
  config.blend = true;
  config.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  config.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

  VkVertexInputBindingDescription instanceBinding{};
  instanceBinding.binding = 0;
  instanceBinding.stride = sizeof(RKInPerInstanceAttributesText);
  instanceBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
  config.bindings = {instanceBinding};
  config.attributes = {
      textAttribute(0, offsetof(RKInPerInstanceAttributesText, position)),
      textAttribute(1, offsetof(RKInPerInstanceAttributesText, scale)),
      textAttribute(2, offsetof(RKInPerInstanceAttributesText, vertexCoordinatesData)),
      textAttribute(3, offsetof(RKInPerInstanceAttributesText, textureCoordinatesData)),
  };
  _pipeline = VulkanShader::createGraphicsPipeline(_renderer, _renderer->scenePipelineLayout(), config);
}

void VulkanAtomTextShader::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  reloadData();
}

void VulkanAtomTextShader::reloadData()
{
  destroyStructureBuffers();
  _structureBuffers.resize(_renderStructures.size());

  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _structureBuffers[i].resize(_renderStructures[i].size());
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      auto *source = dynamic_cast<RKRenderAtomSource *>(_renderStructures[i][j].get());
      if (!source || source->renderTextType() == RKTextType::none)
      {
        continue;
      }

      const QString fontName = source->renderTextFont();
      _structureBuffers[i][j].fontName = fontName;
      FontGpu &font = fontForName(fontName);
      const std::vector<RKInPerInstanceAttributesText> atomTextData = source->atomTextData(font.atlas.get());
      _structureBuffers[i][j].instanceCount = static_cast<uint32_t>(atomTextData.size());
      if (!atomTextData.empty())
      {
        _renderer->uploadBuffer(_structureBuffers[i][j].instanceBuffer, atomTextData.data(),
                                atomTextData.size() * sizeof(RKInPerInstanceAttributesText), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
      }
    }
  }
}

void VulkanAtomTextShader::paint(VkCommandBuffer commandBuffer)
{
  if (!_pipeline)
  {
    return;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

  uint32_t flatIndex = 0;
  for (size_t i = 0; i < _structureBuffers.size(); ++i)
  {
    for (size_t j = 0; j < _structureBuffers[i].size(); ++j)
    {
      auto *object = (i < _renderStructures.size() && j < _renderStructures[i].size()) ? _renderStructures[i][j].get() : nullptr;
      auto *source = dynamic_cast<RKRenderAtomSource *>(object);
      const StructureBuffers &buffers = _structureBuffers[i][j];
      if (object && object->isVisible() && source && source->drawAtoms() && source->renderTextType() != RKTextType::none &&
          buffers.instanceCount > 0 && buffers.instanceBuffer.buffer)
      {
        auto fontIt = _fonts.find(buffers.fontName);
        if (fontIt != _fonts.end() && fontIt->second.samplerSet)
        {
          _renderer->bindSceneDescriptors(commandBuffer, _renderer->scenePipelineLayout(), flatIndex);
          _renderer->bindSamplerDescriptorSet(commandBuffer, _renderer->scenePipelineLayout(), fontIt->second.samplerSet);
          VkDeviceSize offset = 0;
          vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffers.instanceBuffer.buffer, &offset);
          vkCmdDraw(commandBuffer, 4, buffers.instanceCount, 0, 0);
        }
      }
      ++flatIndex;
    }
  }
}
