INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/skvulkan.cpp \
    $$PWD/skcomputeenergygrid.cpp \
    $$PWD/skcomputeisosurface.cpp \
    $$PWD/skcomputevoidfraction.cpp

HEADERS += \
    $$PWD/skvulkan.h \
    $$PWD/skcomputeenergygrid.h \
    $$PWD/skcomputeisosurface.h \
    $$PWD/skcomputevoidfraction.h

SIM_SHADERS = $$PWD/shaders/energy_grid.comp \
              $$PWD/shaders/void_fraction.comp \
              $$PWD/shaders/mc_classify.comp \
              $$PWD/shaders/mc_construct.comp \
              $$PWD/shaders/mc_traverse.comp

macx{
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

simglsl.output = ${QMAKE_FILE_NAME}.spv
simglsl.commands = $$GLSLANG_VALIDATOR -V -I$$PWD/shaders ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
simglsl.CONFIG += target_predeps
simglsl.input = SIM_SHADERS
simglsl.variable_out = HEADERS

QMAKE_EXTRA_COMPILERS += simglsl

RESOURCES += $$PWD/simulationkit_shaders.qrc
