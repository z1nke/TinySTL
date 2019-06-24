#pragma once

#include <cassert>  // for assert
#include <cstddef>  // for ptrdiff_t, size_t
#include <cstdlib>  // for exit
#include <new>      // for placement new 

#include "utility.h"

namespace tiny_stl 
{

template <typename T>
inline T* _allocate(ptrdiff_t size, T* /* p */) 
{
    std::set_new_handler(nullptr);
    T* tmp = reinterpret_cast<T*>(::operator new(static_cast<size_t>(size * sizeof(T))));
    if (tmp == nullptr) 
    {
        printf("out of memery");
        exit(EXIT_FAILURE);
    }
    return tmp;
}

template <typename T>
inline void _deallocate(T* buffer) noexcept 
{
    ::operator delete(buffer);
}

template <typename T, typename... Args>
inline void __construct(T* p, Args&&... args) 
{
    new (const_cast<void*>(static_cast<const volatile void*>(p)))
        T(tiny_stl::forward<Args>(args)...);            // placement new, ctor T
}

template <typename T>
inline void destroy_at(T* ptr) 
{
    ptr->~T();
}

template <typename T>
class allocator 
{
public:
    using value_type        = T;
    using pointer           = T*;
    using const_pointer     = const T*;
    using reference         = T&;
    using const_reference   = const T&;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;

    using propagate_on_container_move_assignment = tiny_stl::true_type; // c++14
    using is_always_equal   = tiny_stl::true_type;                      // c++17

    using _Not_user_specialized = void;

public:
    allocator() noexcept { }    // do nothing
    allocator(const allocator<T>&) noexcept { } 

    template <typename Other>
    allocator(const allocator<Other>&) noexcept { }

    template <typename Other>
    allocator<T>& operator=(const allocator<Other>&) noexcept 
    {   
        // do nothing
        return *this;
    }

    template <typename U>
    struct rebind 
    {
        using other = allocator<U>;
    };

    pointer allocate(size_type n) 
    {
        return _allocate(static_cast<difference_type>(n), 
            reinterpret_cast<pointer>(0));
    }

    void deallocate(pointer p, size_t n) noexcept 
    {
        _deallocate(p);
    }

    pointer address(reference x) const noexcept 
    {
        return tiny_stl::addressof(x);
    }

    const_pointer address(const_reference x) const noexcept 
    {
        return reinterpret_cast<const_pointer>(&x);
    }

    template <typename T, typename... Args>
    void construct(T* p, Args&&... args) 
    {
        __construct(p, tiny_stl::forward<Args>(args)...);
    }

    template <typename T>
    void destroy(T* ptr) 
    {
        destroy_at(ptr);
    }

    size_type max_size() const noexcept 
    {
        return (UINT_MAX / sizeof(T));
    }
};  // class allocator<T>

template <>
class allocator<void> 
{
public:
    using value_type    = void;
    using pointer       = void*;
    using const_pointer = const void*;

    template <class U>
    struct rebind 
    {
        using other = allocator<U>;
    };

public:
    allocator() noexcept { }
    allocator(const allocator<void>&) noexcept { }

    template <typename Other>
    allocator(const allocator<Other>&) noexcept { }

    template <typename Other>
    allocator<void>& operator=(const allocator<Other>&) noexcept 
    {
        return *this;
    }
};  // class allocator<void>


template <typename T>
inline bool operator==(const allocator<T>& lhs, 
                       const allocator<T>& rhs) noexcept 
{
    return true;
}

template <typename T>
inline bool operator!=(const allocator<T>& lhs,
                       const allocator<T>& rhs) noexcept 
{
    return (!(lhs == rhs));
}

template <typename Alloc>
inline void __Swap_alloc(Alloc& lhs, Alloc& rhs, true_type) 
{
    _Swap_ADL(lhs, rhs);
}

template <typename Alloc>
inline void __Swap_alloc(Alloc& lhs, Alloc& rhs, false_type) 
{
    assert(lhs == rhs);
}

template <typename Alloc>
inline void _Swap_alloc(Alloc& lhs, Alloc& rhs) noexcept 
{
    typename allocator_traits<Alloc>::propagate_on_container_swap tag;
    __Swap_alloc(lhs, rhs, tag);
}

}   // namespace tiny_stl