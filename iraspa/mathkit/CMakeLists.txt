add_library(mathkit STATIC
  bool3.cpp
  double2.cpp
  double3.cpp
  double3x3.cpp
  double4.cpp
  double4x3.cpp
  double4x4.cpp
  float2.cpp
  float3.cpp
  float3x3.cpp
  float3x4.cpp
  float4.cpp
  float4x4.cpp
  int3.cpp
  int3x3.cpp
  matrix.cpp
  nsrect.cpp
  nssize.cpp
  ring.cpp
  ringmatrix.cpp
  simd_quatd.cpp
)
add_library(iRASPA::mathkit ALIAS mathkit)

target_include_directories(mathkit PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(mathkit
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
  PRIVATE
    iRASPA::compile_options
)

if(APPLE)
  # double3x3.cpp uses the LAPACK routines from Accelerate.
  target_link_libraries(mathkit PRIVATE "-framework Accelerate")
endif()
