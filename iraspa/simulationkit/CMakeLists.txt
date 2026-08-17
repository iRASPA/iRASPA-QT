target_include_directories(iraspa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(iraspa PRIVATE
  constants.cpp
  forcefieldset.cpp
  forcefieldsets.cpp
  forcefieldtype.cpp
  marchingcubes.cpp
)

# The compute backends provide the same classes, so exactly one of them is
# compiled. Vulkan takes precedence when both backends are enabled.
if(IRASPA_USE_VULKAN)
  add_subdirectory(vulkan)
else()
  add_subdirectory(opencl)
endif()
