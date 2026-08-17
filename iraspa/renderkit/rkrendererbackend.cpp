/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#include "rkrendererbackend.h"

#include <cstring>
#include <vector>

#include <QDebug>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#if defined(USE_VULKAN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <vulkan/vulkan.h>
#ifndef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#endif
#ifndef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#define VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR 0x00000001
#endif
#endif

namespace
{
bool gProbed = false;
bool gOpenGLAvailable = false;
bool gVulkanAvailable = false;

#if defined(USE_OPENGL)
bool probeOpenGL()
{
  QSurfaceFormat format;
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);

  QOffscreenSurface surface;
  surface.setFormat(format);
  surface.create();
  if (!surface.isValid())
  {
    qDebug() << "OpenGL probe: offscreen surface is not valid";
    return false;
  }

  QOpenGLContext context;
  context.setFormat(format);
  if (!context.create())
  {
    qDebug() << "OpenGL probe: failed to create a 3.3 core context";
    return false;
  }
  if (!context.makeCurrent(&surface))
  {
    qDebug() << "OpenGL probe: failed to make the context current";
    return false;
  }

  const QSurfaceFormat actual = context.format();
  context.doneCurrent();
  const bool ok = actual.majorVersion() > 3 || (actual.majorVersion() == 3 && actual.minorVersion() >= 3);
  if (!ok)
  {
    qDebug() << "OpenGL probe: context is" << actual.majorVersion() << "." << actual.minorVersion();
  }
  return ok;
}
#endif

#if defined(USE_VULKAN)
bool hasInstanceExtension(const std::vector<VkExtensionProperties> &available, const char *name)
{
  for (const VkExtensionProperties &extension : available)
  {
    if (strcmp(extension.extensionName, name) == 0)
    {
      return true;
    }
  }
  return false;
}

bool probeVulkan()
{
  uint32_t extensionCount = 0;
  if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS)
  {
    qDebug() << "Vulkan probe: failed to enumerate instance extensions";
    return false;
  }
  std::vector<VkExtensionProperties> available(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, available.data());

  std::vector<const char *> instanceExtensions;
  VkInstanceCreateFlags flags = 0;
#ifdef Q_OS_MACOS
  if (hasInstanceExtension(available, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
  {
    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }
  if (hasInstanceExtension(available, "VK_KHR_get_physical_device_properties2"))
  {
    instanceExtensions.push_back("VK_KHR_get_physical_device_properties2");
  }
#endif

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "iRASPA";
  appInfo.applicationVersion = VK_MAKE_VERSION(2, 1, 0);
  appInfo.pEngineName = "iRASPA";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.flags = flags;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
  createInfo.ppEnabledExtensionNames = instanceExtensions.data();

  VkInstance instance = VK_NULL_HANDLE;
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS || instance == VK_NULL_HANDLE)
  {
    qDebug() << "Vulkan probe: failed to create a Vulkan instance";
    return false;
  }

  uint32_t deviceCount = 0;
  const VkResult deviceResult = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  vkDestroyInstance(instance, nullptr);
  if (deviceResult != VK_SUCCESS || deviceCount == 0)
  {
    qDebug() << "Vulkan probe: no physical devices";
    return false;
  }
  return true;
}
#endif
} // namespace

void RKRendererAvailability::probe()
{
  if (gProbed)
  {
    return;
  }
  gProbed = true;
#if defined(USE_OPENGL)
  gOpenGLAvailable = probeOpenGL();
#else
  gOpenGLAvailable = false;
#endif
#if defined(USE_VULKAN)
  gVulkanAvailable = probeVulkan();
#else
  gVulkanAvailable = false;
#endif
  qDebug() << "Renderer availability: OpenGL =" << gOpenGLAvailable << "Vulkan =" << gVulkanAvailable;
}

bool RKRendererAvailability::isOpenGLAvailable()
{
  probe();
  return gOpenGLAvailable;
}

bool RKRendererAvailability::isVulkanAvailable()
{
  probe();
  return gVulkanAvailable;
}

RKRendererBackend RKRendererAvailability::preferredBackend()
{
  probe();
  if (gVulkanAvailable)
  {
    return RKRendererBackend::Vulkan;
  }
  if (gOpenGLAvailable)
  {
    return RKRendererBackend::OpenGL;
  }
#if defined(USE_VULKAN)
  return RKRendererBackend::Vulkan;
#else
  return RKRendererBackend::OpenGL;
#endif
}
