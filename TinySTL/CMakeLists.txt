add_executable(main
    test.cpp
)

target_include_directories(main
    PRIVATE
    ${PROJECT_SOURCE_DIR}/TinySTL
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(main_exe ${PROJECT_BINARY_DIR}/TinySTL/${CMAKE_BUILD_TYPE}/main)
else()
  set(main_exe ${PROJECT_BINARY_DIR}/TinySTL/main)
endif()

add_custom_target(utest
    COMMAND ${main_exe}
    DEPENDS main
    COMMENT "TinySTL unit testing..."
)