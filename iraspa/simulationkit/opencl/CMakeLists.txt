target_include_directories(iraspa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(iraspa PRIVATE
  skcomputeenergygrid.cpp
  skcomputeisosurface.cpp
  skcomputevoidfraction.cpp
  skopencl.cpp
)
