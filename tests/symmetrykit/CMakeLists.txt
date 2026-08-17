add_executable(tst_symmetrykit
  main.cpp
  qfuzzycompare.cpp
  tst_cifparser.cpp
  tst_symmetrykitfindconventionalcell.cpp
  tst_symmetrykitfindpointgroup.cpp
  tst_symmetrykitfindpointgroupnopartialoccupancies.cpp
  tst_symmetrykitfindspacegroup.cpp
  tst_symmetrykitfindspacegroupnopartialoccupancies.cpp
  tst_symmetrykitsmallestprimitivecell.cpp
  tst_symmetrykitsmallestprimitivecellnopartialoccupancies.cpp
  tst_symmetrykittransformationmatrix.cpp
)

target_link_libraries(tst_symmetrykit PRIVATE
  iRASPA::compile_options
  iRASPA::symmetrykit
  Qt${QT_VERSION_MAJOR}::Test
  Qt${QT_VERSION_MAJOR}::Widgets
)

# The test cases open the reference structures through paths that are relative
# to the working directory.
add_test(NAME symmetrykit
  COMMAND tst_symmetrykit
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
