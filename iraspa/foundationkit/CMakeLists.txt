add_library(foundationkit STATIC
  binpacker.cpp
  characterset.cpp
  indexpath.cpp
  logreporting.cpp
  scanner.cpp
  zip.cpp
)
add_library(iRASPA::foundationkit ALIAS foundationkit)

target_include_directories(foundationkit PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(foundationkit
  PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    iRASPA::mathkit
  PRIVATE
    iRASPA::compile_options
    iRASPA::lzma
    ZLIB::ZLIB
)
