INCLUDEPATH += $$PWD

macx{
  SOURCES +=  $$PWD/makeviewmetal.mm
}

SOURCES += \
    $$PWD/vulkanwindow.cpp \
    $$PWD/vulkanrenderer.cpp \
    $$PWD/rkfontatlas.cpp

HEADERS += \
    $$PWD/tiny_obj_loader.h \
    $$PWD/vulkanwindow.h \
    $$PWD/vulkanrenderer.h \
    $$PWD/vulkanrenderer.h

# compile shaders and include them into resources
SHADERS = $$PWD/shaders/shader.vert \
          $$PWD/shaders/shader.frag

VULKAN_SDK_PATH = /usr/local/VulkanSDK/1.2.182.0
glsl.output = ${QMAKE_FILE_NAME}.spv
glsl.commands = $${VULKAN_SDK_PATH}/macOS/bin/glslangValidator -V  ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
glsl.CONFIG += target_predeps
glsl.input = SHADERS
glsl.variable_out = HEADERS

QMAKE_EXTRA_COMPILERS += glsl

RESOURCES   += $$PWD/shaders.qrc

