# TinySTL

基于 `C++14 ` 的 TinySTL。  
编译通过 `clang++6 and above, VS2017, VS2019`  

删除了之前的库，进行了一下整合。

实现的部分：

- 工具库：

    - `pair`
    - `tuple`
    - `type_traits` （部分）
    - `allocator`
    - `unique_ptr`
    - `shared_ptr, weak_ptr`
    - `functional`

- 容器：

    - `array`
    - `vector`， 特化 `vector<bool>` 没有实现
    - `deque`
    - `forward_list`
    - `list`
    - `map, multimap`
    - `set, multiset`
    - `unordered_set, unordered_multiset`
    - `unordered_map, unordered_multimap`

- string：

    - `basic_string`
    - `basic_string_cow`
    - `basic_string_view`

- adapter：

    - `stack`
    - `queue`
    - `priority_queue`

- 算法库：

    - `all_of, any_of, none_of`
    - `for_each`
    - `count_if, count`
    - `mismatch`
    - `find, find_if, find_if_not`
    - `copy, copy_if, copy_n, copy_backward`
    - `move, move_backward`
    - `fill, fill_n`
    - `transform`
    - `generate, generate_n`
    - `swap, swap_ranges, iter_swap`
    - `reverse`
    - `rotate`
    - `is_sorted, is_sorted_until`
    - `sort`
    - `lower_bound, upper_bound, equal_range`
    - `binary_search`
    - `is_heap, is_heap_until`
    - `make_heap`
    - `push_heap, pop_heap`
    - `sort_heap`
    - `max, max_element`
    - `min, min_element`
    - `minmax, minmax_element`
    - `equal`
    - `lexicographical_compare`

    