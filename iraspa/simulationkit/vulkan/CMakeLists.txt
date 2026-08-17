target_include_directories(iraspa PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(iraspa PRIVATE
  skcomputeenergygrid.cpp
  skcomputeisosurface.cpp
  skcomputevoidfraction.cpp
  skvulkan.cpp
)

iraspa_add_shader_resource(iraspa
  NAME simulationkitshaders
  PREFIX "/simulationkit"
  SHADERS
    shaders/energy_grid.comp
    shaders/mc_classify.comp
    shaders/mc_construct.comp
    shaders/mc_traverse.comp
    shaders/void_fraction.comp
)
