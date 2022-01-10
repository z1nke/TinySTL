add_executable(main
    algorithm.hpp
    allocators.hpp
    array.hpp
    cow_string.hpp
    deque.hpp
    forward_list.hpp
    functional.hpp
    hashtable.hpp
    iterator.hpp
    list.hpp
    map.hpp
    memory.hpp
    queue.hpp
    rbtree.hpp
    set.hpp
    stack.hpp
    string.hpp
    string_view.hpp
    tuple.hpp
    type_traits.hpp
    unordered_map.hpp
    unordered_set.hpp
    utility.hpp
    vector.hpp

    test.cpp
)

target_include_directories(main
    PRIVATE
    ${PROJECT_SOURCE_DIR}/TinySTL
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(main_exe ${PROJECT_BINARY_DIR}/TinySTL/${CMAKE_CFG_INTDIR}/main.exe)
else()
  set(main_exe ${PROJECT_BINARY_DIR}/TinySTL/main)
endif()

add_custom_target(utest
    COMMAND ${main_exe}
    DEPENDS main
    COMMENT "TinySTL(${main_exe}) unit testing..."
)