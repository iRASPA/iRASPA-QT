INCLUDEPATH += $$PWD

macx{
  SOURCES +=  $$PWD/makeviewmetal.mm
}

SOURCES += \
    $$PWD/vulkanwindow.cpp \
    $$PWD/vulkanscene.cpp \
    $$PWD/vulkanrenderer.cpp \
    $$PWD/vulkanshader.cpp \
    $$PWD/vulkanbackgroundshader.cpp \
    $$PWD/vulkanatomsphereshader.cpp \
    $$PWD/vulkanatomambientocclusionshader.cpp \
    $$PWD/vulkanbondshader.cpp \
    $$PWD/vulkanprimitiveobjectshader.cpp \
    $$PWD/vulkanunitcellshader.cpp \
    $$PWD/vulkanlocalaxesshader.cpp \
    $$PWD/vulkanboundingboxshader.cpp \
    $$PWD/vulkanisosurfaceshader.cpp \
    $$PWD/vulkanvolumeshader.cpp \
    $$PWD/vulkanglobalaxesshader.cpp \
    $$PWD/vulkanribbonshader.cpp \
    $$PWD/vulkanribbonambientocclusionshader.cpp \
    $$PWD/vulkanatomtextshader.cpp \
    $$PWD/vulkanselectionshader.cpp \
    $$PWD/vulkanmeasurementshader.cpp \
    $$PWD/vulkanpickingshader.cpp \
    $$PWD/rkfontatlas.cpp

HEADERS += \
    $$PWD/vulkanwindow.h \
    $$PWD/vulkanscene.h \
    $$PWD/vulkanrenderer.h \
    $$PWD/vulkanshader.h \
    $$PWD/vulkanbackgroundshader.h \
    $$PWD/vulkanatomsphereshader.h \
    $$PWD/vulkanatomambientocclusionshader.h \
    $$PWD/vulkanbondshader.h \
    $$PWD/vulkanprimitiveobjectshader.h \
    $$PWD/vulkanunitcellshader.h \
    $$PWD/vulkanlocalaxesshader.h \
    $$PWD/vulkanboundingboxshader.h \
    $$PWD/vulkanisosurfaceshader.h \
    $$PWD/vulkanvolumeshader.h \
    $$PWD/vulkanglobalaxesshader.h \
    $$PWD/vulkanribbonshader.h \
    $$PWD/vulkanribbonambientocclusionshader.h \
    $$PWD/vulkanatomtextshader.h \
    $$PWD/vulkanselectionshader.h \
    $$PWD/vulkanmeasurementshader.h \
    $$PWD/vulkanpickingshader.h \
    $$PWD/vulkanuniformstringliterals.h

SHADERS = $$PWD/shaders/background.vert \
          $$PWD/shaders/background.frag \
          $$PWD/shaders/atom_ortho_imposter.vert \
          $$PWD/shaders/atom_ortho_imposter.frag \
          $$PWD/shaders/atom_persp_imposter.vert \
          $$PWD/shaders/atom_persp_imposter.frag \
          $$PWD/shaders/ao_shadow.vert \
          $$PWD/shaders/ao_shadow.frag \
          $$PWD/shaders/ao_accumulate.vert \
          $$PWD/shaders/ao_accumulate.frag \
          $$PWD/shaders/bond_imposter.vert \
          $$PWD/shaders/bond_imposter.frag \
          $$PWD/shaders/bond_imposter_external.frag \
          $$PWD/shaders/unit_cell_sphere.vert \
          $$PWD/shaders/unit_cell_sphere.frag \
          $$PWD/shaders/unit_cell_cylinder.vert \
          $$PWD/shaders/unit_cell_cylinder.frag \
          $$PWD/shaders/bounding_box_sphere.vert \
          $$PWD/shaders/bounding_box_cylinder.vert \
          $$PWD/shaders/bounding_box.frag \
          $$PWD/shaders/local_axes.vert \
          $$PWD/shaders/local_axes.frag \
          $$PWD/shaders/isosurface.vert \
          $$PWD/shaders/isosurface.frag \
          $$PWD/shaders/volume.vert \
          $$PWD/shaders/volume.frag \
          $$PWD/shaders/primitive_object.vert \
          $$PWD/shaders/primitive_object.frag \
          $$PWD/shaders/global_axes_background.vert \
          $$PWD/shaders/global_axes_background.frag \
          $$PWD/shaders/global_axes_system.vert \
          $$PWD/shaders/global_axes_system.frag \
          $$PWD/shaders/global_axes_text.vert \
          $$PWD/shaders/global_axes_text.frag \
          $$PWD/shaders/ribbon.vert \
          $$PWD/shaders/ribbon.frag \
          $$PWD/shaders/ribbon_ao_shadow.vert \
          $$PWD/shaders/ribbon_ao_shadow.frag \
          $$PWD/shaders/ribbon_ao_accumulate.vert \
          $$PWD/shaders/ribbon_ao_accumulate.frag \
          $$PWD/shaders/atom_text.vert \
          $$PWD/shaders/atom_text.frag \
          $$PWD/shaders/atom_selection_ortho_imposter.vert \
          $$PWD/shaders/atom_selection_persp_imposter.vert \
          $$PWD/shaders/atom_selection_glow_imposter.frag \
          $$PWD/shaders/atom_selection_stripes_imposter.frag \
          $$PWD/shaders/atom_selection_worley_imposter.frag \
          $$PWD/shaders/atom_measurement_ortho_imposter.vert \
          $$PWD/shaders/atom_measurement_persp_imposter.vert \
          $$PWD/shaders/atom_measurement_imposter.frag \
          $$PWD/shaders/bond_selection_imposter.vert \
          $$PWD/shaders/bond_selection_glow_imposter.frag \
          $$PWD/shaders/bond_selection_stripes_imposter.frag \
          $$PWD/shaders/bond_selection_worley_imposter.frag \
          $$PWD/shaders/primitive_selection.vert \
          $$PWD/shaders/primitive_selection_glow.frag \
          $$PWD/shaders/primitive_selection_stripes.frag \
          $$PWD/shaders/primitive_selection_worley.frag \
          $$PWD/shaders/ribbon_selection_glow.vert \
          $$PWD/shaders/ribbon_selection_glow.frag \
          $$PWD/shaders/ribbon_selection_stripes.vert \
          $$PWD/shaders/ribbon_selection_stripes.frag \
          $$PWD/shaders/ribbon_selection_worley.vert \
          $$PWD/shaders/ribbon_selection_worley.frag \
          $$PWD/shaders/pick_atom_ortho_imposter.vert \
          $$PWD/shaders/pick_atom_persp_imposter.vert \
          $$PWD/shaders/pick_atom_imposter.frag \
          $$PWD/shaders/pick_bond_imposter.vert \
          $$PWD/shaders/pick_bond_imposter.frag \
          $$PWD/shaders/pick_bond_imposter_external.frag \
          $$PWD/shaders/pick_primitive.vert \
          $$PWD/shaders/pick_primitive.frag \
          $$PWD/shaders/pick_ribbon.vert \
          $$PWD/shaders/pick_ribbon.frag \
          $$PWD/shaders/fullscreen.vert \
          $$PWD/shaders/blur_downsample.frag \
          $$PWD/shaders/blur_horizontal.vert \
          $$PWD/shaders/blur_vertical.vert \
          $$PWD/shaders/blur.frag \
          $$PWD/shaders/composite.frag \
          $$PWD/shaders/selection_overlay.frag \
          $$PWD/shaders/depth_resolve.frag

macx {
  exists(/opt/homebrew/bin/glslangValidator) {
    GLSLANG_VALIDATOR = /opt/homebrew/bin/glslangValidator
  } else:exists(/usr/local/bin/glslangValidator) {
    GLSLANG_VALIDATOR = /usr/local/bin/glslangValidator
  } else {
    isEmpty(VULKAN_SDK_PATH) {
      VULKAN_SDK_PATH = $$(VULKAN_SDK)
    }
    isEmpty(VULKAN_SDK_PATH) {
      VULKAN_SDK_PATH = /usr/local/VulkanSDK/1.2.182.0
    }
    GLSLANG_VALIDATOR = $${VULKAN_SDK_PATH}/macOS/bin/glslangValidator
    !exists($$GLSLANG_VALIDATOR) {
      GLSLANG_VALIDATOR = glslangValidator
    }
  }
}
win32 {
  isEmpty(VULKAN_SDK_PATH) {
    VULKAN_SDK_PATH = $$(VULKAN_SDK)
  }
  isEmpty(VULKAN_SDK_PATH) {
    VULKAN_SDK_PATH = "C:/VulkanSDK/1.2.141.2"
  }
  GLSLANG_VALIDATOR = $${VULKAN_SDK_PATH}/Bin/glslangValidator.exe
}
unix:!macx {
  GLSLANG_VALIDATOR = glslangValidator
}

glsl.output = ${QMAKE_FILE_NAME}.spv
glsl.commands = $$GLSLANG_VALIDATOR -V -I$$PWD/shaders ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
glsl.CONFIG += target_predeps
glsl.input = SHADERS
glsl.variable_out = HEADERS

QMAKE_EXTRA_COMPILERS += glsl

RESOURCES += $$PWD/shaders.qrc
