add_executable(main
    test.cpp
)

target_include_directories(main
    PRIVATE
    ${PROJECT_SOURCE_DIR}/TinySTL
)

add_custom_target(utest
    COMMAND ${PROJECT_BINARY_DIR}/TinySTL/main
    DEPENDS main
    COMMENT "TinySTL unit testing..."
)