/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "rkimposters.h"

#include <algorithm>

#include <symmetrykit.h>

std::vector<RKBackToFrontItem> backToFrontRenderOrder(
    const std::vector<std::vector<std::shared_ptr<RKRenderObject>>> &structures, RKCamera *camera)
{
  struct Item
  {
    RKBackToFrontItem order;
    double depth = 0.0;
  };

  std::vector<Item> items;
  uint32_t index = 0;
  for (size_t i = 0; i < structures.size(); ++i)
  {
    for (size_t j = 0; j < structures[i].size(); ++j)
    {
      double depth = 0.0;
      if (camera)
      {
        if (RKRenderObject *structure = structures[i][j].get())
        {
          SKBoundingBox box = structure->cell() ? structure->cell()->boundingBox() : SKBoundingBox();
          const double3 center = box.center();
          const double4x4 modelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPointWithTranslation(
              double4x4(structure->orientation()), center, structure->origin());
          const double4 worldCenter = modelMatrix * double4(center.x, center.y, center.z, 1.0);
          const double4 viewCenter = camera->modelViewMatrix() * worldCenter;
          depth = viewCenter.z;
        }
      }
      items.push_back(Item{RKBackToFrontItem{i, j, index}, depth});
      ++index;
    }
  }

  // the camera looks along the negative z-axis in view space, so the most
  // negative view-space z is farthest away and must be drawn first
  std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) { return a.depth < b.depth; });

  std::vector<RKBackToFrontItem> order;
  order.reserve(items.size());
  for (const Item &item : items)
  {
    order.push_back(item.order);
  }
  return order;
}
