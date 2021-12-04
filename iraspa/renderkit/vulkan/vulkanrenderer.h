#ifndef VKRENDER_H
#define VKRENDER_H

#include <array>
#include <unordered_map>
#include <vector>


#include <QByteArray>

#ifndef NOMINMAX
#define NOMINMAX
#endif  // !NOMINMAX

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rkrenderuniforms.h"
#include "rkcamera.h"

struct QueueFamilyIndices
{
  uint32_t graphicsFamily = -1;
  uint32_t presentFamily = -1;

  bool isComplete() { return graphicsFamily != -1 && presentFamily != -1; }
};

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};


struct InstanceData
{
  glm::mat4 transform;
  glm::vec4 color;
};

struct DrawItem
{
  int meshId;
  uint32_t instanceBase;
  uint32_t instanceCount;
};

struct Vertex
{
  glm::vec3 pos;
};

const int VERTEX_BUFFER_BIND_ID = 0;
const int INSTANCE_BUFFER_BIND_ID = 1;

class VulkanRenderer {
   public:
#ifdef VK_USE_PLATFORM_MACOS_MVK
    VulkanRenderer(const void* viewId);
#elif VK_USE_PLATFORM_WIN32_KHR
    VulkanRenderer(HWND viewId);
#endif
    virtual ~VulkanRenderer();

    void drawFrame(std::shared_ptr<RKCamera> camera);
    void updateCamera(glm::mat4x4& view, glm::mat4x4& proj, glm::vec3& camPos, glm::vec3& worldUp);
    void updateUniformBuffer(std::shared_ptr<RKCamera> camera);

    void recreateVertexBuffer();
    void recreateIndexBuffer();

    void buildCommandBuffers(const std::vector<DrawItem>& drawItems);
    void updateInstanceBuffer(const std::vector<InstanceData>& instData);

    void resize(uint32_t w, uint32_t h);

    void clear();

    bool isCached(int id);
    void addGeometry(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, int id);

   private:
    VkClearColorValue mClearColor = {{0.8f, 0.8f, 0.8f, 1.0f}};
    glm::mat4x4 proj, view;
    glm::vec3 camPos, worldUp;

    uint32_t height = 0;
    uint32_t width = 0;

    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const std::vector<Vertex> vertices = {{{-0.5f, -0.5f, 0.0f}}, {{0.5f, -0.5f, 0.0f}}, {{0.5f, 0.5f, 0.0f}}, {{-0.5f, 0.5f, 0.0f}}};
    const std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = false;
#endif

    void initVulkan();
    void setupDebugCallback();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width,
                     uint32_t height,
                     VkFormat format,
                     VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkImage& image,
                     VkDeviceMemory& imageMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    bool hasStencilComponent(VkFormat format);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void createDepthResources();

    void rebuildCommandBuffer(const size_t index);

    void createCommandBuffers();

    void createSemaphores();
    void createFences();
    void recreateSwapChain();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createVertexBuffer();
    void createIndexBuffer();
    void createInstanceBuffer(const int maxInstanceCount);
    void createUniformBuffer();
    void createDescriptorPool();
    void createDescriptorSet();
    void createDescriptorSetLayout();

    void cleanup();
    void cleanupSwapChain();

    void createInstance();
    void checkExtensions();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    VkShaderModule createShaderModule(QByteArray& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    bool isInstanceDirty = false;
    std::vector<InstanceData> mInstances;
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;
    std::vector<DrawItem> mDrawItems;

    struct Mesh {
        int32_t vertexBase;
        uint32_t indexBase;
        uint32_t indexCount;
    };
    std::vector<Mesh> mMeshes;
    std::unordered_map<int, Mesh> mIdToMesh;

    VkInstance instance;
    VkPhysicalDevice physicalDevice = nullptr;
    VkDevice device;

    VkSurfaceKHR surface;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkFence> waitFences;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceBufferMemory;

    size_t instanceBuffersCount = 0;

    std::vector<VkBuffer> instanceStagingBuffer;
    std::vector<VkDeviceMemory> instanceStagingBufferMemory;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    VkDebugReportCallbackEXT callback;



#ifdef VK_USE_PLATFORM_MACOS_MVK
    const void* viewId;
#elif VK_USE_PLATFORM_WIN32_KHR
    HWND viewId;
#endif
};

#endif  // VKRENDER_H

