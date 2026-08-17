target_include_directories(iraspa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(iraspa PRIVATE
  ribbonaolayout.cpp
  ribbonaotexturepostprocess.cpp
  rkcamera.cpp
  rkglobalaxes.cpp
  rkimposters.cpp
  rklight.cpp
  rklocalaxes.cpp
  rkrendererbackend.cpp
  rkrenderkitprotocols.cpp
  rkrenderuniforms.cpp
  rkribbonmesh.cpp
  trackball.cpp
  volumetransferfunctions.cpp
)

add_subdirectory(geometry)

if(IRASPA_USE_OPENGL)
  add_subdirectory(opengl)
endif()

if(IRASPA_USE_VULKAN)
  add_subdirectory(vulkan)
endif()

# RKFontAtlas is implemented once per backend; Vulkan wins when both are built.
if(IRASPA_USE_VULKAN)
  target_sources(iraspa PRIVATE vulkan/rkfontatlas.cpp)
else()
  target_sources(iraspa PRIVATE opengl/rkfontatlas.cpp)
endif()
