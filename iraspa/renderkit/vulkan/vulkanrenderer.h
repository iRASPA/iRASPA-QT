#ifndef VKRENDER_H
#define VKRENDER_H

#include <array>
#include <cstdint>
#include <vector>

#include <QByteArray>
#include <QImage>
#include <QRect>
#include <QWindow>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vulkan/vulkan.h>

#include "rkrenderuniforms.h"
#include "ribbonaolayout.h"

struct QueueFamilyIndices
{
  uint32_t graphicsFamily = UINT32_MAX;
  uint32_t presentFamily = UINT32_MAX;

  bool isComplete() const { return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX; }
};

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanBuffer
{
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkDeviceSize size = 0;
};

struct VulkanTexture
{
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t depth = 1;
  uint32_t layers = 1;
};

class VulkanRenderer
{
public:
  explicit VulkanRenderer(QWindow *window);
  explicit VulkanRenderer(uint32_t width, uint32_t height);
  ~VulkanRenderer();

  void resize(uint32_t w, uint32_t h);
  void waitIdle();
  bool isOffscreen() const { return _offscreen; }
  QImage takeReadbackImage() const { return _readbackImage; }

  bool beginFrame();
  VkCommandBuffer currentCommandBuffer() const;
  void snapshotDepthAndResume();
  void endFrame();

  VkDevice device() const { return _device; }
  VkPhysicalDevice physicalDevice() const { return _physicalDevice; }
  VkRenderPass renderPass() const { return _renderPass; }
  VkRenderPass pickRenderPass() const { return _pickRenderPass; }
  VkSampleCountFlagBits msaaSampleCount() const { return _msaaSamples; }
  bool sampleRateShadingEnabled() const { return _sampleRateShading; }
  VkExtent2D swapChainExtent() const { return _swapChainExtent; }
  VkPipelineLayout scenePipelineLayout() const { return _scenePipelineLayout; }
  VkPipelineLayout backgroundPipelineLayout() const { return _backgroundPipelineLayout; }
  VkPipelineLayout volumePipelineLayout() const { return _volumePipelineLayout; }
  VkDescriptorSetLayout sceneDescriptorSetLayout() const { return _sceneDescriptorSetLayout; }
  VkDescriptorSetLayout samplerDescriptorSetLayout() const { return _samplerDescriptorSetLayout; }
  VkSampler linearSampler() const { return _linearSampler; }
  VkSampler repeatSampler() const { return _repeatSampler; }
  VkSampler nearestSampler() const { return _nearestSampler; }
  VkSampler shadowCompareSampler() const { return _shadowCompareSampler; }
  VkFormat depthFormat() const { return _depthFormat; }
  // true when the scene depth buffer has a stencil aspect (used for the bond box-cap passes)
  bool sceneDepthHasStencil();
  VkCommandBuffer beginOneTimeCommands();
  void submitOneTimeCommands(VkCommandBuffer commandBuffer);
  VulkanTexture createAttachmentTexture(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
                                        VkImageAspectFlags aspect, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                             uint32_t layerCount = 1);
  void copyImageToHost(VkImage image, VkImageLayout srcLayout, VkFormat format, VkImageAspectFlags aspect, uint32_t width,
                       uint32_t height, void *dst, VkDeviceSize size);
  VkDescriptorPool descriptorPool() const { return _descriptorPool; }
  const VulkanTexture &whiteTexture() const { return _whiteTexture; }
  VkImageView sampledDepthView() const { return _sampledDepthView; }

  VkDeviceSize structureUniformStride() const { return _structureUniformStride; }
  VkDeviceSize isosurfaceUniformStride() const { return _isosurfaceUniformStride; }
  VkDeviceSize minUniformBufferOffsetAlignment() const { return _minUniformBufferOffsetAlignment; }

  void bindSceneDescriptors(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t structureIndex) const;
  void bindSamplerDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet samplerSet) const;

  void updateTransformUniforms(const RKTransformationUniforms &uniforms);
  void updateStructureUniforms(const std::vector<RKStructureUniforms> &uniforms);
  void updateIsosurfaceUniforms(const std::vector<RKIsosurfaceUniforms> &uniforms);
  void updateLightUniforms(const RKLightsUniforms &uniforms);
  void updateGlobalAxesUniforms(const RKGlobalAxesUniforms &uniforms);
  void updateRibbonAODebugUniforms(const RibbonAODebugUniforms &uniforms);
  void setSelectionOverlay(const QRect &rect, bool dashed, float pixelScale = 1.0f);
  void clearSelectionOverlay();

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBuffer &out);
  void destroyBuffer(VulkanBuffer &buffer);
  void uploadBuffer(VulkanBuffer &buffer, const void *data, VkDeviceSize size, VkBufferUsageFlags usage);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  VulkanTexture createTextureRGBA8(const QImage &image);
  VulkanTexture createTextureR16F(uint32_t width, uint32_t height, const void *data);
  VulkanTexture createTexture3D_RGBA32F(uint32_t width, uint32_t height, uint32_t depth, const void *data);
  VulkanTexture createTexture2DArray_RGBA32F(uint32_t width, uint32_t layers, const void *data);
  void destroyTexture(VulkanTexture &texture);
  VkDescriptorSet allocateSamplerDescriptorSet(const VulkanTexture &texture);
  VkDescriptorSet allocateVolumeDescriptorSet(const VulkanTexture &volume, VkImageView depthView, const VulkanTexture &transferFunction);
  void updateVolumeDescriptorSet(VkDescriptorSet descriptorSet, const VulkanTexture &volume, VkImageView depthView,
                                 const VulkanTexture &transferFunction);

  VkShaderModule createShaderModule(const QByteArray &code);
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  VkCommandBuffer beginPickPass();
  std::array<int, 4> endPickPassAndReadPixel(int x, int y, float *outDepth = nullptr);

  static VkDeviceSize alignedUniformSize(VkDeviceSize size, VkDeviceSize alignment);

private:
  // Qt hands out the VkInstance and the VkSurfaceKHR when the window was set up
  // for it, which is what lets the same code serve an X11 and a Wayland session.
  // All four are inert when Qt does not manage the window, either because the
  // platform has its own surface code below or because this Qt was built
  // without Vulkan support.
  bool usesQtManagedSurface() const;
  VkInstance qtVkInstance() const;
  VkSurfaceKHR qtVkSurface() const;
  void qtPresentNotify(bool aboutToBeQueued) const;

  void initVulkan();
  void setupDebugCallback();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createSurface();
  void createSwapChain();
  void createOffscreenTarget();
  void recreateOffscreenTargets();
  void createImageViews();
  void createRenderPass();
  void createContinueRenderPass();
  void createPickRenderPass();
  void createDepthResolveRenderPass();
  void createBlurRenderPass();
  void createCompositeRenderPass();
  void chooseMsaaSampleCount();
  void setDefaultViewport(VkCommandBuffer commandBuffer);
  void createPickFramebuffer();
  void destroyPickFramebuffer();
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void createDescriptorSetLayouts();
  void createPipelineLayouts();
  void createSceneFramebuffer();
  void createCompositeFramebuffers();
  void createCommandPool();
  void createDepthResources();
  void createSceneColorResources();
  void destroySceneColorResources();
  void createBlurResources();
  void destroyBlurResources();
  void createFullscreenQuad();
  void createPostProcessPipelines();
  void destroyPostProcessPipelines();
  void createPostProcessDescriptorSets();
  void updatePostProcessDescriptors();
  void recordDepthResolve(VkCommandBuffer commandBuffer);
  void recordBlurAndComposite(VkCommandBuffer commandBuffer);
  void recordSelectionOverlay(VkCommandBuffer commandBuffer);
  void cmdImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout oldLayout,
                       VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage,
                       VkPipelineStageFlags dstStage);
  void beginSimpleColorPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer);
  void drawFullscreenQuad(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout layout, VkDescriptorSet set);
  void createCommandBuffers();
  void createSyncObjects();
  void createUniformBuffers();
  void createDescriptorPool();
  void createSceneDescriptorSet();
  void writeSceneDescriptorSet(uint32_t slot);
  void prepareUniformWrite();
  void destroyUniformBuffers();
  void destroySyncObjects();
  void createSampler();
  void createWhiteTexture();
  void recreateSwapChain();
  void cleanup();
  void cleanupSwapChain(bool destroyRenderPass);
  void createInstance();
  void checkExtensions();
  bool checkValidationLayerSupport();
  std::vector<const char *> getRequiredExtensions();
  bool isDeviceSuitable(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  VkFormat findDepthFormat();
  VkFormat findPickColorFormat();
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                              VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1);
  void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory,
                   VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
  void createImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t arrayLayers, VkImageType imageType,
                   VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                   VkImage &image, VkDeviceMemory &imageMemory, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
  bool hasStencilComponent(VkFormat format);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth = 1,
                         uint32_t layerCount = 1);
  void destroySampledDepth();
  void writeHostVisible(VkDeviceMemory memory, const void *data, VkDeviceSize size);
  uint32_t uniformSlot() const { return _currentFrame; }

  static constexpr uint32_t kMaxFramesInFlight = 3;

  QWindow *_window = nullptr;
  const void *_metalLayer = nullptr;
  bool _offscreen = false;
  VkExtent2D _offscreenExtent{};
  VulkanTexture _offscreenColor;
  QImage _readbackImage;

  std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = false;
#endif

  VkInstance _instance = VK_NULL_HANDLE;
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
  VkDevice _device = VK_NULL_HANDLE;
  VkSurfaceKHR _surface = VK_NULL_HANDLE;
  bool _ownsInstance = true;
  bool _ownsSurface = true;
  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkQueue _presentQueue = VK_NULL_HANDLE;

  VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
  std::vector<VkImage> _swapChainImages;
  VkFormat _swapChainImageFormat = VK_FORMAT_UNDEFINED;
  VkExtent2D _swapChainExtent{};
  std::vector<VkImageView> _swapChainImageViews;
  std::vector<VkFramebuffer> _compositeFramebuffers;

  VkSampleCountFlagBits _msaaSamples = VK_SAMPLE_COUNT_1_BIT;
  bool _sampleRateShading = false;
  VkFormat _sceneColorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
  VulkanTexture _sceneColor;
  VulkanTexture _glowColor;
  VkFramebuffer _sceneFramebuffer = VK_NULL_HANDLE;

  VkImage _depthImage = VK_NULL_HANDLE;
  VkDeviceMemory _depthImageMemory = VK_NULL_HANDLE;
  VkImageView _depthImageView = VK_NULL_HANDLE;        // attachment view (depth + stencil aspects)
  VkImageView _depthSampleView = VK_NULL_HANDLE;       // sampled view (depth aspect only)
  VkImage _sampledDepthImage = VK_NULL_HANDLE;
  VkDeviceMemory _sampledDepthMemory = VK_NULL_HANDLE;
  VkImageView _sampledDepthView = VK_NULL_HANDLE;            // sampled view (depth aspect only)
  VkImageView _sampledDepthAttachmentView = VK_NULL_HANDLE;  // attachment view (depth + stencil aspects)
  VkFormat _depthFormat = VK_FORMAT_UNDEFINED;
  bool _sampledDepthInitialized = false;

  VkRenderPass _renderPass = VK_NULL_HANDLE;
  VkRenderPass _continueRenderPass = VK_NULL_HANDLE;
  VkRenderPass _pickRenderPass = VK_NULL_HANDLE;
  VkRenderPass _depthResolveRenderPass = VK_NULL_HANDLE;
  VkRenderPass _blurRenderPass = VK_NULL_HANDLE;
  VkRenderPass _compositeRenderPass = VK_NULL_HANDLE;
  VkFramebuffer _sampledDepthFramebuffer = VK_NULL_HANDLE;
  VkFramebuffer _pickFramebuffer = VK_NULL_HANDLE;
  VkImage _pickColorImage = VK_NULL_HANDLE;
  VkDeviceMemory _pickColorMemory = VK_NULL_HANDLE;
  VkImageView _pickColorView = VK_NULL_HANDLE;
  VkImage _pickDepthImage = VK_NULL_HANDLE;
  VkDeviceMemory _pickDepthMemory = VK_NULL_HANDLE;
  VkImageView _pickDepthView = VK_NULL_HANDLE;
  VulkanBuffer _pickReadbackBuffer;
  VulkanBuffer _pickDepthReadbackBuffer;
  VkCommandBuffer _pickCommandBuffer = VK_NULL_HANDLE;
  VkFormat _pickColorFormat = VK_FORMAT_R32G32B32A32_UINT;
  VkDescriptorSetLayout _sceneDescriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _samplerDescriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _volumeDescriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _msaaSamplerDescriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _compositeDescriptorSetLayout = VK_NULL_HANDLE;
  VkPipelineLayout _scenePipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _backgroundPipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _volumePipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _msaaSamplerPipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _samplerPipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _compositePipelineLayout = VK_NULL_HANDLE;
  VkPipelineLayout _overlayPipelineLayout = VK_NULL_HANDLE;
  VkPipeline _depthResolvePipeline = VK_NULL_HANDLE;
  VkPipeline _blurDownsamplePipeline = VK_NULL_HANDLE;
  VkPipeline _blurHorizontalPipeline = VK_NULL_HANDLE;
  VkPipeline _blurVerticalPipeline = VK_NULL_HANDLE;
  VkPipeline _compositePipeline = VK_NULL_HANDLE;
  VkPipeline _overlayPipeline = VK_NULL_HANDLE;
  bool _selectionOverlayEnabled = false;
  bool _selectionOverlayDashed = false;
  float _selectionOverlayPixelScale = 1.0f;
  QRect _selectionOverlayRect;
  VulkanTexture _blurDownsample;
  VulkanTexture _blurHorizontal;
  VulkanTexture _blurVertical;
  VkFramebuffer _blurDownsampleFramebuffer = VK_NULL_HANDLE;
  VkFramebuffer _blurHorizontalFramebuffer = VK_NULL_HANDLE;
  VkFramebuffer _blurVerticalFramebuffer = VK_NULL_HANDLE;
  VkDescriptorSet _depthResolveDescriptorSet = VK_NULL_HANDLE;
  VkDescriptorSet _blurDownsampleDescriptorSet = VK_NULL_HANDLE;
  VkDescriptorSet _blurHorizontalDescriptorSet = VK_NULL_HANDLE;
  VkDescriptorSet _blurVerticalDescriptorSet = VK_NULL_HANDLE;
  VkDescriptorSet _compositeDescriptorSet = VK_NULL_HANDLE;
  VulkanBuffer _quadVertexBuffer;
  VulkanBuffer _quadIndexBuffer;
  uint32_t _quadIndexCount = 0;

  std::array<VkSemaphore, kMaxFramesInFlight> _imageAvailableSemaphores{};
  std::array<VkSemaphore, kMaxFramesInFlight> _renderFinishedSemaphores{};
  std::array<VkFence, kMaxFramesInFlight> _inFlightFences{};
  std::vector<VkFence> _imagesInFlight;
  VkCommandPool _commandPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> _commandBuffers;

  std::array<VulkanBuffer, kMaxFramesInFlight> _frameUniformBuffers{};
  std::array<VulkanBuffer, kMaxFramesInFlight> _structureUniformBuffers{};
  std::array<VulkanBuffer, kMaxFramesInFlight> _isosurfaceUniformBuffers{};
  std::array<VulkanBuffer, kMaxFramesInFlight> _lightsUniformBuffers{};
  std::array<VulkanBuffer, kMaxFramesInFlight> _axesUniformBuffers{};
  std::array<VulkanBuffer, kMaxFramesInFlight> _ribbonAODebugUniformBuffers{};

  VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
  std::array<VkDescriptorSet, kMaxFramesInFlight> _sceneDescriptorSets{};
  VkSampler _linearSampler = VK_NULL_HANDLE;
  VkSampler _repeatSampler = VK_NULL_HANDLE;
  VkSampler _nearestSampler = VK_NULL_HANDLE;
  VkSampler _shadowCompareSampler = VK_NULL_HANDLE;
  VulkanTexture _whiteTexture;

  VkDebugReportCallbackEXT _callback = VK_NULL_HANDLE;

  VkDeviceSize _minUniformBufferOffsetAlignment = 256;
  VkDeviceSize _structureUniformStride = 256;
  VkDeviceSize _isosurfaceUniformStride = 256;

  uint32_t _currentImageIndex = 0;
  uint32_t _currentFrame = 0;
  bool _frameStarted = false;
  bool _continuedPass = false;
  bool _usePortabilityEnumeration = false;
};

#endif
