add_executable(tst_mathkit
  main.cpp
  tst_hnf.cpp
  tst_smf.cpp
)

target_link_libraries(tst_mathkit PRIVATE
  iRASPA::compile_options
  iRASPA::mathkit
  Qt${QT_VERSION_MAJOR}::Test
  Qt${QT_VERSION_MAJOR}::Widgets
)

add_test(NAME mathkit COMMAND tst_mathkit)
