#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

static BOOL isMoltenVKMetalLayer(CALayer *layer)
{
  if (layer == nil)
  {
    return NO;
  }
  // QContainerLayer subclasses CALayer and crashes MoltenVK.
  // QMetalLayer subclasses CAMetalLayer and is the layer Qt already created.
  NSString *name = NSStringFromClass([layer class]);
  return [name isEqualToString:@"QMetalLayer"] || [layer isMemberOfClass:[CAMetalLayer class]];
}

static void configureMoltenVKMetalLayer(CAMetalLayer *metalLayer)
{
  metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  metalLayer.framebufferOnly = NO;
  metalLayer.opaque = YES;
}

static CALayer *findMoltenVKMetalLayer(CALayer *root)
{
  if (root == nil)
  {
    return nil;
  }
  NSMutableArray<CALayer *> *stack = [NSMutableArray arrayWithObject:root];
  while (stack.count > 0)
  {
    CALayer *layer = stack.firstObject;
    [stack removeObjectAtIndex:0];
    if (isMoltenVKMetalLayer(layer))
    {
      return layer;
    }
    if (layer.sublayers.count > 0)
    {
      [stack addObjectsFromArray:layer.sublayers];
    }
  }
  return nil;
}

extern "C" void *makeViewMetalCompatible(void *handle)
{
  NSView *view = (__bridge NSView *)handle;
  if (![view isKindOfClass:[NSView class]])
  {
    return NULL;
  }

  view.wantsLayer = YES;

  CALayer *existing = findMoltenVKMetalLayer(view.layer);
  if (existing)
  {
    configureMoltenVKMetalLayer((CAMetalLayer *)existing);
    return (__bridge void *)existing;
  }

  // Do not replace Qt's QContainerLayer; MoltenVK needs a real CAMetalLayer.
  CAMetalLayer *metalLayer = [CAMetalLayer layer];
  const CGFloat scale = (view.window && view.window.backingScaleFactor > 0.0)
                            ? view.window.backingScaleFactor
                            : NSScreen.mainScreen.backingScaleFactor;
  metalLayer.contentsScale = scale;
  configureMoltenVKMetalLayer(metalLayer);
  metalLayer.frame = view.layer ? view.layer.bounds : NSRectToCGRect(view.bounds);
  metalLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
  CGSize backing = [view convertSizeToBacking:view.bounds.size];
  if (backing.width < 1.0)
  {
    backing.width = 1.0;
  }
  if (backing.height < 1.0)
  {
    backing.height = 1.0;
  }
  metalLayer.drawableSize = backing;

  if (view.layer)
  {
    [view.layer addSublayer:metalLayer];
  }
  else
  {
    view.layer = metalLayer;
  }
  return (__bridge void *)metalLayer;
}
