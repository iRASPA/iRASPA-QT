target_include_directories(iraspa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(iraspa PRIVATE
  arrowxgeometry.cpp
  arrowygeometry.cpp
  arrowzgeometry.cpp
  axessystemdefaultgeometry.cpp
  backplanegeometry.cpp
  cappedcylinderdoublebondgeometry.cpp
  cappedcylindergeometry.cpp
  cappedcylinderpartialdoublebondgeometry.cpp
  cappedcylindersinglebondgeometry.cpp
  cappedcylindertriplebondgeometry.cpp
  cappednsidedprismgeometry.cpp
  cubegeometry.cpp
  cubeprimitivegeometry.cpp
  cylindergeometry.cpp
  nsidedprismgeometry.cpp
  nspheregeometry.cpp
  octahedrongeometry.cpp
  quadgeometry.cpp
  spheregeometry.cpp
  uncappedcylindergeometry.cpp
  unitcubegeometry.cpp
)
