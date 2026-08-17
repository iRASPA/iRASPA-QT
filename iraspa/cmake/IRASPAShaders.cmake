include_guard(GLOBAL)

find_program(IRASPA_GLSLANG_VALIDATOR
  NAMES glslangValidator
  HINTS
    "$ENV{VULKAN_SDK}/bin"
    "$ENV{VULKAN_SDK}/Bin"
    "$ENV{VULKAN_SDK}/macOS/bin"
  DOC "glslangValidator executable, used to compile GLSL down to SPIR-V"
)

# Compiles GLSL shaders to SPIR-V and embeds the result into <target> as a Qt
# resource. The SPIR-V modules end up under ":<PREFIX>/shaders/<name>.spv",
# which is where the renderer looks for them.
#
# NAME     basename of the generated .qrc and of the object library carrying it,
#          has to be unique across the project because rcc derives its
#          initialiser symbol from it.
# PREFIX   resource prefix.
# SHADERS  GLSL sources, relative to the current source directory.
function(iraspa_add_shader_resource target)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "" "NAME;PREFIX" "SHADERS")

  if(NOT IRASPA_GLSLANG_VALIDATOR)
    message(FATAL_ERROR
      "glslangValidator was not found, so the Vulkan shaders cannot be compiled. "
      "Install it (e.g. 'brew install glslang', 'apt install glslang-tools') or "
      "configure with -DIRASPA_USE_VULKAN=OFF.")
  endif()

  set(shader_dir "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

  # glslangValidator resolves #include itself, so every .glsl fragment in the
  # shader directory counts as a dependency of every compiled shader.
  file(GLOB shader_includes CONFIGURE_DEPENDS "${shader_dir}/*.glsl")

  set(spirv_modules "")
  foreach(shader IN LISTS ARG_SHADERS)
    get_filename_component(shader_name "${shader}" NAME)
    set(spirv_module "${CMAKE_CURRENT_BINARY_DIR}/shaders/${shader_name}.spv")

    add_custom_command(
      OUTPUT "${spirv_module}"
      COMMAND "${IRASPA_GLSLANG_VALIDATOR}"
              -V "-I${shader_dir}"
              "${CMAKE_CURRENT_SOURCE_DIR}/${shader}"
              -o "${spirv_module}"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${shader}" ${shader_includes}
      COMMENT "Compiling shader ${shader_name}"
      VERBATIM
    )

    list(APPEND spirv_modules "${spirv_module}")
  endforeach()

  set(entries "")
  foreach(spirv_module IN LISTS spirv_modules)
    file(RELATIVE_PATH entry "${CMAKE_CURRENT_BINARY_DIR}" "${spirv_module}")
    string(APPEND entries "        <file>${entry}</file>\n")
  endforeach()

  # Written through configure_file() so that an unchanged resource list does not
  # invalidate the build.
  set(resource_file "${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME}.qrc")
  file(WRITE "${resource_file}.in"
    "<RCC>\n    <qresource prefix=\"${ARG_PREFIX}\">\n${entries}    </qresource>\n</RCC>\n")
  configure_file("${resource_file}.in" "${resource_file}" COPYONLY)

  # The versionless qt_add_resources() only exists from Qt 5.15 onwards.
  if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_resources(resource_sources "${resource_file}")
  else()
    qt5_add_resources(resource_sources "${resource_file}")
  endif()

  # The compiled resource is wrapped in an object library because the rules that
  # produce it have to live in the same directory as the target that uses them.
  add_library(${ARG_NAME} OBJECT ${resource_sources})
  set_target_properties(${ARG_NAME} PROPERTIES
    AUTOMOC OFF
    AUTOUIC OFF
    AUTORCC OFF
  )
  target_link_libraries(${ARG_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Core)
  target_link_libraries(${target} PRIVATE ${ARG_NAME})
endfunction()
