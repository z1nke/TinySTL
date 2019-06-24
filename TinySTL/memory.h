#pragma once

#include <intrin.h> // for _Interlockedxxx
#include <cstring>
#include <new>
#include <ostream>

#include "allocators.h"
#include "algorithm.h"
#include "iterator.h"

namespace tiny_stl 
{

namespace 
{

template <typename T, typename... Args>
inline void __Construct_in_place(T& dest, Args&&... args) 
    noexcept((is_nothrow_constructible<T, Args...>::value)) 
{
    ::new (static_cast<void*>(&dest))
        T(tiny_stl::forward<Args>(args)...);
}

template <typename T>
inline void __Destroy_in_place(T& obj) noexcept 
{
    obj.~T();
}


// use plain function to copy/fill

template <typename InIt, typename FwdIt>
inline FwdIt _Uninitialized_copy(InIt first, InIt last,
                                 FwdIt dest, false_type /*no special optimization*/) 
{
    for (; first != last; ++dest, ++first) 
        __Construct_in_place(*dest, *first);
    
    return dest;
}

template <typename InIt, typename FwdIt>
inline FwdIt _Uninitialized_copy(InIt first, InIt last,
                                 FwdIt dest, true_type /*is pod -- assign*/) 
{
    for (; first != last; ++first, ++dest) 
        *dest = *first;

    return dest;
}

template <typename InIt, typename Size, typename FwdIt>
inline FwdIt _Uninitialized_copy_n(InIt first, Size n,
                                   FwdIt dest, 
                                   false_type /*no special optimization*/) 
{
    for (; n > 0; ++dest, ++first, --n) 
        __Construct_in_place(*dest, *first);

    return dest;
}

template <typename InIt, typename Size, typename FwdIt>
inline FwdIt _Uninitialized_copy_n(InIt first, Size n, 
                                   FwdIt dest, 
                                   true_type /*is pod -- assign*/) 
{
    for (; n > 0; ++first, ++dest, --n) 
        *dest = *first;
    
    return dest;
}

template <typename FwdIt, typename T>
inline void _Uninitialized_fill(FwdIt first, FwdIt last, const T& x, 
                                false_type /*is not pod type*/) 
{

    for (; first != last; ++first) 
        __Construct_in_place(*first, x);
}

template <typename FwdIt, typename T>
inline void _Uninitialized_fill(FwdIt first, FwdIt last, const T& x,
                                true_type /*is pod -- assign*/ )
{
    for (; first != last; ++first) 
        *first = x;
}

template <typename FwdIt, typename Size, typename T>
inline void _Uninitialized_fill_n(FwdIt first, Size n, const T& x, 
                                false_type/*is not pod type*/) 
{

    for (; n--; ++first) 
        __Construct_in_place(*first, x);
}

template <typename FwdIt, typename Size, typename T> 
inline void _Uninitialized_fill_n(FwdIt first, Size n, const T& x,
                                true_type /*is pod -- copy*/) 
{
    for (; n--; ++first) 
        *first = x;
}

}  // unnamed namespace

template <typename InIter, typename FwdIter>
inline FwdIter uninitialized_copy(InIter first, 
                                  InIter last,
                                  FwdIter dest) 
{
    using T = typename iterator_traits<InIter>::value_type;
    return _Uninitialized_copy(first, last, dest, 
        bool_constant<is_pod<T>::value>{});
}

// const char* and const wchar_t* version
inline char* uninitialized_copy(const char* first, const char* last, 
                                char* dest) 
{
    memmove(dest, first, sizeof(char) * (last - first));
    return dest + (last - first);
}

inline wchar_t* uninitialized_copy(const wchar_t* first, 
                                   const wchar_t* last,
                                   wchar_t* dest) 
{
    memmove(dest, first, sizeof(wchar_t) * (last - first));
    return dest + (last - first);
}

template <typename InIter, typename Size, 
    typename FwdIter>
inline FwdIter uninitialized_copy_n(InIter first, Size n,
                                    FwdIter dest) 
{
    using T = typename iterator_traits<InIter>::value_type;
    return _Uninitialized_copy_n(first, n, dest, 
        integral_constant<bool, is_pod_v<T>>{});
}

inline char* uninitialized_copy_n(const char* first, 
                                size_t n, char* dest) 
{
    memmove(dest, first, n);
    return dest + n;
}

inline wchar_t* uninitialized_copy_n(const wchar_t* first, 
                                size_t n, wchar_t* dest) 
{
    memmove(dest, first, n * sizeof(wchar_t));
    return dest + n;
}

// use x to construct [first, last)
template <typename FwdIter, typename T>
inline void uninitialized_fill(FwdIter first, 
                    FwdIter last, const T& x) 
{
    _Uninitialized_fill(first, last, x, 
        integral_constant<bool, is_pod_v<T>>{});
}

// use x to construct [first, first + n)
template <typename FwdIter, typename Size, typename T>
inline void uninitialized_fill_n(FwdIter first, 
                                 Size n, const T& x) 
{
    _Uninitialized_fill_n(first, n, x,
        integral_constant<bool, is_pod_v<T>>{});
}


// use allocator to copy/fill

template <typename Alloc, typename = void>
struct _Is_default_allocator : false_type { };

template <typename T>
struct _Is_default_allocator<allocator<T>,
    typename allocator<T>::_Not_user_specialized> : true_type { };

template <typename Void, typename... Ts>
struct _Has_no_alloc_construct : true_type { };

template <typename Alloc, typename Ptr, typename... Args>
struct _Has_no_alloc_construct<void_t<
    decltype(tiny_stl::declval<Alloc&>().construct(tiny_stl::declval<Ptr>(), 
        tiny_stl::declval<Args>()...))>, Alloc, Ptr, Args...> : false_type { };


template <typename Alloc, typename Ptr, typename... Args>
using _Use_default_construct = disjunction<
    _Is_default_allocator<Alloc>,
    _Has_no_alloc_construct<void, Alloc, Ptr, Args...>>;


template <typename Alloc, typename Ptr, typename = void>
struct _Has_no_alloc_destroy : true_type { };

template <typename Alloc, typename Ptr>
struct _Has_no_alloc_destroy<Alloc, Ptr, void_t<
    decltype(tiny_stl::declval<Alloc&>().destroy(tiny_stl::declval<Ptr>()))>>
    : false_type { };

template <typename Alloc, typename Ptr>
using _Use_default_destroy = disjunction<
    _Is_default_allocator<Alloc>,
    _Has_no_alloc_destroy<Alloc, Ptr>>;

template <typename FwdIter, typename Alloc>
inline void __Destroy_alloc_range(FwdIter first, FwdIter last,
                                  Alloc&, true_type) 
{
    // do nothing
}


template <typename FwdIter, typename Alloc>
inline void __Destroy_alloc_range(FwdIter first, FwdIter last,
                                Alloc& alloc, false_type) 
{
    for (; first != last; ++first) 
        alloc.destroy(tiny_stl::addressof(*first));
}


template <typename FwdIter, typename Alloc>
inline void _Destroy_alloc_range(FwdIter first, FwdIter last, Alloc& alloc) 
{
    using T = typename Alloc::value_type;
    __Destroy_alloc_range(first, last, alloc, typename conjunction<
        is_trivially_destructible<T>,
        _Use_default_destroy<Alloc, T*>>::type{});
}


template <typename FwdIter>
inline void __Destroy_range(FwdIter, FwdIter, true_type) 
{
    // do nothing
}

template <typename FwdIter>
inline void __Destroy_range(FwdIter first, FwdIter last, false_type) 
{
    for (; first != last; ++first)
        __Destroy_in_place(*first);
    
}

template <typename FwdIter>
inline void _Destroy_range(FwdIter first, FwdIter last) 
{
    __Destroy_range(first, last, 
        is_trivially_destructible<_Iterator_value_t<FwdIter>>{});
}

template <typename FwdIter>
inline void destroy(FwdIter first, FwdIter last)
{
    _Destroy_range(first, last);
}

template <typename FwdIter, typename Size, typename Alloc>
inline FwdIter __Uninit_alloc_fill_n(FwdIter first, Size n,
    const typename iterator_traits<FwdIter>::value_type& val,
    Alloc& alloc, false_type /* fill memset is not safe */) 
{

    for (; n > 0; --n, ++first) 
        alloc.construct(tiny_stl::addressof(*first), val);

    return first;
}

template <typename FwdIter, typename Size, typename Alloc>
inline FwdIter __Uninit_alloc_fill_n(FwdIter first, Size n,
    const typename iterator_traits<FwdIter>::value_type& val,
    Alloc&, true_type /* fill memset is safe */) 
{
    _CSTD memset(first, val, n);
    return first + n;
}

template <typename FwdIter, typename Size, typename Alloc>
inline FwdIter _Uninitialized_alloc_fill_n(FwdIter first, Size n,
    const typename iterator_traits<FwdIter>::value_type& val,
    Alloc& alloc) 
{    
    // if memset safe && (default allocator || not user allocator)
    // then use memset
    return __Uninit_alloc_fill_n(first, n, val, alloc,
        typename conjunction<decltype(tiny_stl::_Fill_memset_is_safe(first, val)),
        _Use_default_construct<Alloc, decltype(tiny_stl::addressof(*first)), 
        decltype(val)>>::type{});
}


template <typename FwdIter>
using _Use_memset_value_construct_t
    = typename conjunction<
        is_pointer<FwdIter>,
        is_scalar<_Iterator_value_t<FwdIter>>,
        negation<is_volatile<_Iterator_value_t<FwdIter>>>,
        negation<is_member_pointer<_Iterator_value_t<FwdIter>>>
    >;

template <typename FwdIter>
inline FwdIter __Zero_memset_range(FwdIter first, FwdIter last) 
{
    char* const first_ch = reinterpret_cast<char*>(first);
    char* const last_ch = reinterpret_cast<char*>(last);
    _CSTD memset(first_ch, 0, last_ch - first_ch);
    return last;
}

template <typename FwdIter, typename Size, typename Alloc>
inline FwdIter __Uninit_alloc_default_n(FwdIter first, Size n, 
                                        Alloc&, true_type) 
{
    return __Zero_memset_range(first, first + n);
}

template <typename FwdIter, typename Size, typename Alloc>
inline FwdIter __Uninit_alloc_default_n(FwdIter first, Size n,
                                        Alloc& alloc, false_type) 
{
    alloc.construct(tiny_stl::addressof(*first));
    return first;
}

template <typename FwdIter, typename Size, typename Alloc> 
inline FwdIter _Uninitialized_alloc_default_n(FwdIter first, Size n,
                                        Alloc& alloc) 
{
    return __Uninit_alloc_default_n(first, n, alloc,
        typename conjunction<_Use_memset_value_construct_t<FwdIter>,
        _Use_default_construct<Alloc, 
            decltype(tiny_stl::addressof(*first))>>::type{});
}

template <typename InIter, typename FwdIter, typename Alloc>
inline FwdIter __Uninitialized_alloc_copy_helper(InIter first, InIter last,
                                FwdIter newFirst, Alloc&, true_type/*pod*/)
{
    for (; first != last; ++first, ++newFirst) 
        *newFirst = *first;
    
    return newFirst;
}

template <typename InIter, typename FwdIter, typename Alloc>
inline FwdIter __Uninitialized_alloc_copy_helper(InIter first, InIter last,
                                FwdIter newFirst, Alloc& alloc, false_type) 
{

    for (; first != last; ++first, ++newFirst) 
        alloc.construct(newFirst, *first);
    
    return newFirst;
}

template <typename InIter, typename FwdIter, typename Alloc>
inline FwdIter _Uninitialized_alloc_copy(InIter first, InIter last,
                                        FwdIter newFirst, Alloc& alloc) 
{
    using T = typename iterator_traits<InIter>::value_type;
    return __Uninitialized_alloc_copy_helper(first, last, newFirst, alloc,
        bool_constant<is_pod<T>::value>{});
}

template <typename InIter, typename FwdIter, typename Alloc>
inline FwdIter _Uninitialized_alloc_move(InIter first, InIter last,
                                        FwdIter newFirst, Alloc& alloc) 
{

    for (; first != last; ++newFirst, ++first) 
    {
        allocator_traits<Alloc>::construct(alloc,
            tiny_stl::addressof(*newFirst),
            tiny_stl::move(*first));
    }

    return newFirst;
}



template <typename T>
struct _Get_first_parameter;

template <template <typename, typename...> class T,
    typename First,
    typename... Rest>
struct _Get_first_parameter<T<First, Rest...>> 
{
    using type = First;
};

template <typename T, typename = void>
struct _Get_element_type 
{
    using type = typename _Get_first_parameter<T>::type;
};

template <typename T>
struct _Get_element_type<T, void_t<typename T::element_type>>
{
    using type = typename T::element_type;
};

template <typename T, typename = void>
struct _Get_ptr_difference_type 
{
    using type = ptrdiff_t;
};

template <typename T>
struct _Get_ptr_difference_type<T, void_t<typename T::difference_type>> 
{
    using type = typename T::difference_type;
};


template <typename NewFirst, typename T> 
struct _Replace_first_parameter;

template <typename NewFirst, 
    template <typename, typename...> class T,
    typename First,
    typename... Rest>
struct _Replace_first_parameter<NewFirst, T<First, Rest...>> 
{
    using type = T<NewFirst, Rest...>;
};

template <typename T, typename Other, typename = void>
struct _Get_rebind_type 
{
    using type = typename _Replace_first_parameter<Other, T>::type;
};

template <typename T, typename Other>
struct _Get_rebind_type<T, Other, void_t<typename T::template rebind<Other>>> 
{
    using type = typename T::template rebind<Other>;
};
    

// deal with type of like pointer 
template <typename Ptr>
struct pointer_traits 
{
    using pointer         = Ptr;
    using element_type    = typename _Get_element_type<Ptr>::type;
    using difference_type = typename _Get_ptr_difference_type<Ptr>::type;

    template <typename Other>
    using rebind = typename _Get_rebind_type<Ptr, Other>::type;

    using _Reftype = conditional_t<is_void<element_type>::value,
        char&, add_lvalue_reference_t<element_type>>;

    static pointer pointer_to(_Reftype val) 
    {
        return Ptr::pointer_to(val);
    }
};

template <typename T>
struct pointer_traits<T*> 
{
    using element_type = T;
    using pointer = T*;
    using difference_type = ptrdiff_t;

    template <typename Other>
    using rebind = Other*;

    using _Reftype = conditional_t<is_void<T>::value,
        char&, add_lvalue_reference_t<T>>;

    static pointer pointer_to(_Reftype val)
    {
        return tiny_stl::addressof(val);
    }
};


// for allocator_traits type helper 
template <typename Alloc, typename = void>
struct _Get_pointer 
{
    using type = typename Alloc::value_type*;
};

template <typename Alloc>
struct _Get_pointer<Alloc, void_t<typename Alloc::pointer>> 
{
    using type = typename Alloc::pointer;
};

template <typename Alloc, typename = void>
struct _Get_const_pointer 
{
    using _Ptr = typename _Get_pointer<Alloc>::type;
    using _Val = typename Alloc::value_type;
    using type = typename pointer_traits<_Ptr>::template rebind<const _Val>;
};

template <typename Alloc>
struct _Get_const_pointer<Alloc, void_t<typename Alloc::const_pointer>> 
{
    using type = typename Alloc::const_pointer;
};

template <typename Alloc, typename = void>
struct _Get_void_pointer 
{
    using _Ptr = typename _Get_pointer<Alloc>::type;
    using type = typename pointer_traits<_Ptr>::template rebind<void>;
};

template <typename Alloc>
struct _Get_void_pointer<Alloc, void_t<typename Alloc::void_pointer>> 
{
    using type = typename Alloc::void_pointer;
};

template <typename Alloc, typename = void>
struct _Get_const_void_pointer 
{
    using _Ptr = typename _Get_pointer<Alloc>::type;
    using type = typename pointer_traits<_Ptr>::template rebind<const void>;
};

template <typename Alloc>
struct _Get_const_void_pointer<Alloc, void_t<typename Alloc::const_void_pointer>> 
{
    using type = typename Alloc::const_void_pointer;
};

template <typename Alloc, typename = void>
struct _Get_difference_type
{
    using _Ptr = typename _Get_pointer<Alloc>::type;
    using type = typename pointer_traits<_Ptr>::difference_type;
};

template <typename Alloc>
struct _Get_difference_type<Alloc, void_t<typename Alloc::difference_type>>
{
    using type = typename Alloc::difference_type;
};

template <typename Alloc, typename = void>
struct _Get_size_type 
{
    using _Diff = typename _Get_difference_type<Alloc>::type;
    using type = make_unsigned_t<_Diff>;
};

template <typename Alloc>
struct _Get_size_type<Alloc, void_t<typename Alloc::size_type>>
{
    using type = typename Alloc::size_type;
};

template <typename Alloc, typename = void>
struct _Get_propagate_on_container_copy 
{
    using type = false_type;
};

template <typename Alloc>
struct _Get_propagate_on_container_copy<Alloc, void_t<
        typename Alloc::propagate_on_container_copy_assignment>> 
{
    using type = typename Alloc::propagate_on_container_copy_assignment;
};

template <typename Alloc, typename = void>
struct _Get_propagate_on_container_move 
{
    using type = false_type;
};

template <typename Alloc>
struct _Get_propagate_on_container_move<Alloc, void_t<
        typename Alloc::propagate_on_container_move_assignment>> 
{
    using type = typename Alloc::propagate_on_container_move_assignment;
};

template<class Alloc, typename = void>
struct _Get_propagate_on_container_swap 
{
    using type = false_type;
};

template<class Alloc>
struct _Get_propagate_on_container_swap<Alloc, void_t<
        typename Alloc::propagate_on_container_swap>>
{
    using type = typename Alloc::propagate_on_container_swap;
};

template<class Alloc, typename = void>
struct _Get_is_always_equal 
{
    using type = typename is_empty<Alloc>::type;
};

template<class Alloc>
struct _Get_is_always_equal<Alloc, void_t<
        typename Alloc::is_always_equal>> 
{
    using type = typename Alloc::is_always_equal;
};

template <typename Alloc, typename Other, typename = void>
struct _Get_bind_type 
{
    using type = typename _Replace_first_parameter<Other, Alloc>::type;
};

template <typename Alloc, typename Other>
struct _Get_bind_type<Alloc, Other, void_t<
        typename Alloc::template rebind<Other>::other>>
{
    using type = typename Alloc::template rebind<Other>::other;
};

template <typename Alloc, typename Size_type,
    typename Const_void_pointer, typename = void>
struct _Has_allocate_hint : false_type { };

template <typename Alloc, typename Size_type,
    typename Const_void_pointer>
struct _Has_allocate_hint<Alloc, Size_type, Const_void_pointer, void_t<
    decltype(tiny_stl::declval<Alloc&>().allocate(
        tiny_stl::declval<const Size_type&>(),
        tiny_stl::declval<const Const_void_pointer&>()))>>
    : true_type { };


template <typename Alloc, typename = void>
struct _Has_max_size  : false_type { };

template <typename Alloc>
struct _Has_max_size<Alloc, void_t<
    decltype(tiny_stl::declval<const Alloc&>().max_size())>> 
    : true_type { };

template <typename Alloc, typename = void>
struct _Has_select_on_container_copy_construction : false_type { };

template <typename Alloc>
struct _Has_select_on_container_copy_construction<Alloc, void_t<
    decltype(tiny_stl::declval<const Alloc&>().select_on_container_copy_construction())>>  
    : true_type { };


template <typename Alloc>
struct allocator_traits 
{
    using allocator_type     = Alloc;
    using value_type         = typename Alloc::value_type;
    using pointer            = typename _Get_pointer<Alloc>::type;
    using const_pointer      = typename _Get_const_pointer<Alloc>::type;
    using void_pointer       = typename _Get_void_pointer<Alloc>::type;
    using const_void_pointer = typename _Get_const_void_pointer<Alloc>::type;
    using size_type          = typename _Get_size_type<Alloc>::type;
    using difference_type    = typename _Get_difference_type<Alloc>::type;

    using propagate_on_container_copy_assignment
        = typename _Get_propagate_on_container_copy<Alloc>::type;
    using propagate_on_container_move_assignment 
        = typename _Get_propagate_on_container_move<Alloc>::type;
    using propagate_on_container_swap
        = typename _Get_propagate_on_container_swap<Alloc>::type;
    using is_always_equal
        = typename _Get_is_always_equal<Alloc>::type;

    template <typename Other>
    using rebind_alloc = typename _Get_bind_type<Alloc, Other>::type;

    template <typename Other>
    using rebind_traits = allocator_traits<rebind_alloc<Other>>;

    static pointer allocate(Alloc& a, const size_type n) 
    {
        return a.allocate(n);
    }

    static pointer _Allocate(Alloc& a, const size_type n,
                    const_void_pointer hint, true_type)
    {
        return a.allocate(n, hint);
    }

    static pointer _Allocate(Alloc& a, const size_type n,
                    const_void_pointer hint, false_type) 
    {
        return a.allocate(n);
    }

    static pointer allocate(Alloc& a, const size_type n,
                            const_void_pointer hint) 
    {
        return _Allocate(a, n, hint, 
            _Has_allocate_hint<Alloc, size_type, const_void_pointer>{});
    }

    static void deallocate(Alloc& a, pointer ptr, size_type n) 
    {
        a.deallocate(ptr, n);
    }

    template <typename T, typename... Args>
    static void _Construct(true_type, Alloc&, T* ptr, Args&&... args)
    {
        new (static_cast<void*>(ptr)) T(tiny_stl::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    static void _Construct(false_type, Alloc& a, T* ptr, Args&&... args)
    {
        a.construct(ptr, tiny_stl::forward<Args>(args)...);
    }

    template <typename Obj, typename... Args>
    static void construct(Alloc& a, Obj* ptr, Args&&... args)
    {
        _Construct(_Use_default_construct<Alloc, Obj*, Args...>{}, a, ptr,
            tiny_stl::forward<Args>(args)...);
    }

    template <typename T>
    static void _Destroy(Alloc& a, T* ptr, true_type) 
    {
        ptr->~T();
    }

    template <typename T>
    static void _Destroy(Alloc& a, T* ptr, false_type) 
    {
        a.destroy(ptr);
    }

    template <typename T>
    static void destroy(Alloc& a, T* ptr) 
    {
        _Destroy(a, ptr, _Use_default_destroy<Alloc, T*>{});
    }

    static size_type _Max_size(const Alloc& a, true_type) 
    {
        return a.max_size();
    }

    static size_type _Max_size(const Alloc& a, false_type)
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    static size_type max_size(const Alloc& a) 
    {
        return _Max_size(a, _Has_max_size<Alloc>{});
    }


    static Alloc _Select_on_container_copy_construction(const Alloc& a, true_type)
    {
        return a.select_on_container_copy_construction();
    }

    static Alloc _Select_on_container_copy_construction(const Alloc& a, false_type)
    {
        return a;
    }

    static Alloc select_on_container_copy_construction(const Alloc& a)
    {
        return _Select_on_container_copy_construction(a, 
            _Has_select_on_container_copy_construction<Alloc>{});
    }
};  // class allocator_traits<Alloc>

template <typename Con, typename Alloc, typename = void>
struct _Has_allocator_type : false_type 
{
};

template <typename Con, typename Alloc>
struct _Has_allocator_type<Con, Alloc, 
    void_t<typename Con::allocator_type>> 
: is_convertible<Alloc, typename Con::allocator_type>::type
{
};


template <typename Con, typename Alloc>
struct uses_allocator : _Has_allocator_type<Con, Alloc>::type 
{    
};

template <typename Con, typename Alloc>
constexpr bool uses_allocator_value = uses_allocator<T, Alloc>::value;


template <typename T>
struct default_delete 
{
    constexpr default_delete() noexcept = default;

    template <typename U, 
        typename = enable_if_t<is_convertible<U*, T*>::value>>
    default_delete(const default_delete<U>&) noexcept { }

    void operator()(T* ptr) const
    {
        static_assert(sizeof(T) > 0, "delete a incomplete type, ub");
        delete ptr;
    }
};

template <typename T>
class default_delete<T[]> 
{
public:
    constexpr default_delete() noexcept = default;

    template <typename U, 
        typename = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    default_delete(const default_delete<T[]>&) noexcept { }

    template <typename U, 
        typename = enable_if_t<is_convertible<U(*)[], T(*)[]>::value>>
    void operator()(U* ptr) const 
    {
        static_assert(sizeof(T) > 0, "delete a incomplete type, ub");
        delete[] ptr;
    }
};

template <typename T, typename D = default_delete<T>>
class unique_ptr;

template <typename T, typename Deleter>
class unique_ptr 
{
public:
    using pointer       = T*;
    using element_type  = T;
    using delete_type   = Deleter;

private:
    extra::compress_pair<Deleter, pointer> m_pair;

    pointer& _Get_ptr() noexcept 
    {
        return m_pair.get_second();
    }

    const pointer& _Get_ptr() const noexcept 
    {
        return m_pair.get_second();
    }

public:
    constexpr unique_ptr(nullptr_t p = nullptr) noexcept        // (1)
    : m_pair()
    {
        static_assert(is_default_constructible<Deleter>::value
            && !is_pointer<Deleter>::value, "unique construct error");
    }

    explicit unique_ptr(pointer p) noexcept                     // (2)
    : m_pair(Deleter(), p) 
    {
        static_assert(is_default_constructible<Deleter>::value
            && !is_pointer<Deleter>::value, "unique construct error");
    }

    unique_ptr(pointer p,                                       // (3)
        conditional_t<is_reference<Deleter>::value,
        Deleter, const remove_reference_t<Deleter>&> del) noexcept 
    : m_pair(tiny_stl::forward<decltype(del)>(del), p) { }

    unique_ptr(pointer p,                                       // (4)
        remove_reference_t<Deleter>&& del) noexcept 
    : m_pair(tiny_stl::forward<decltype(del)>(del), p) 
    {
        static_assert(!is_reference<Deleter>::value,
            "unique construct error");
    }

    unique_ptr(unique_ptr&& rhs) noexcept                       // (5)
    : m_pair(tiny_stl::forward<delete_type>(rhs.get_deleter()), 
        rhs.release()) { }

    template <typename U, typename D,                           // (6)
        typename = enable_if_t<
            is_convertible<unique_ptr<U, D>::pointer, pointer>::value
            && !is_array<U>::value 
            && ((is_reference<Deleter>::value && is_same<Deleter, D>::value)
                || (!is_reference<Deleter>::value && is_convertible<D, Deleter>::value))>>
    unique_ptr(unique_ptr<U, D>&& rhs) noexcept 
    : m_pair(tiny_stl::forward<D>(rhs.get_deleter()), rhs.release()) { }

    unique_ptr(const unique_ptr&) = delete;

    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr& operator=(unique_ptr&& rhs) noexcept 
    {
        assert(this != tiny_stl::addressof(rhs));
        reset(rhs.release());
        get_deleter() = tiny_stl::forward<Deleter>(rhs.get_deleter());
        return *this;
    }

    template <typename U, typename D,
        typename = enable_if_t<is_convertible<unique_ptr<U, D>::pointer, pointer>::value
            && is_assignable<Deleter&, D&&>>>
    unique_ptr& operator=(unique_ptr<U, D>&& rhs) noexcept
    {
        reset(rhs.release());
        get_deleter() = tiny_stl::forward<D>(rhs.get_deleter());
        return *this;
    }

    unique_ptr& operator=(nullptr_t) noexcept 
    {
        reset();
        return *this;
    }


    delete_type& get_deleter() noexcept
    {
        return m_pair.get_first();
    }

    const delete_type& get_deleter() const noexcept
    {
        return m_pair.get_first();
    }

   
    pointer get() const noexcept
    {
        return _Get_ptr();
    }

    pointer release() noexcept 
    {
        pointer ret = get();
        _Get_ptr() = pointer();
        return ret;
    }

    void reset(pointer ptr = pointer()) noexcept 
    {
        auto old = get();
        m_pair.get_second() = ptr;
        if (old != nullptr)
            get_deleter()(old);
    }

    explicit operator bool() const noexcept 
    {
        return get() != nullptr;
    }

    void swap(unique_ptr& rhs) noexcept
    {
        _Swap_ADL(_Get_ptr(), rhs._Get_ptr());
        _Swap_ADL(get_deleter(), rhs.get_deleter());
    }

    ~unique_ptr() noexcept 
    {
        if (get() != nullptr)
            this->get_deleter()(get());
    }

    add_lvalue_reference_t<T> operator*() const 
    {
        assert(get() != nullptr);
        return *get();
    }

    pointer operator->() const noexcept 
    {
        return _Get_ptr();
    }
};  // unique_ptr<T, Deleter>

template <typename T, typename Deleter>
class unique_ptr<T[], Deleter> 
{
public:
    using pointer = T*;
    using element_type = T;
    using delete_type = Deleter;

private:
    extra::compress_pair<Deleter, pointer> m_pair;

    pointer& _Get_ptr() noexcept 
    {
        return m_pair.get_second();
    }

    const pointer& _Get_ptr() const noexcept 
    {
        return m_pair.get_second();
    }

    template <typename U,
        typename Is_nullptr = is_same<U, nullptr_t>>
    using _Enalble_ctor = enable_if_t<
        is_same<U, pointer>::value
        || Is_nullptr::value
        || (is_same<pointer, element_type*>::value
            && is_pointer<U>::value
            && is_convertible<remove_pointer_t<U>(*)[], element_type(*)[]>::value)>;


public:
    constexpr unique_ptr(nullptr_t p = nullptr) noexcept            // (1)
    : m_pair() 
    { 
        static_assert(is_default_constructible<Deleter>::value
            && !is_pointer<Deleter>::value, "unique construct error");
    }


    template <typename U, typename = _Enalble_ctor<U>>              // (2)
    explicit unique_ptr(U p) noexcept 
    : m_pair(delete_type(), p) 
    {
        static_assert(is_default_constructible<Deleter>::value
            && !is_pointer<Deleter>::value, "unique construct error");
    }

    template <typename U, typename = _Enalble_ctor<U>>              // (3)
    unique_ptr(U p, conditional_t<is_reference<Deleter>::value, Deleter,
        const remove_const_t<Deleter>&> del) noexcept 
    : m_pair(tiny_stl::forward<decltype(del)>(del), p) { }

    template <typename U, typename = _Enalble_ctor<U>>              // (4)
    unique_ptr(U p, remove_reference_t<Deleter>&& del) noexcept 
    : m_pair(tiny_stl::forward<decltype(del)>(del), p)
    {
        static_assert(!is_reference<Deleter>::value,
            "unique_ptr construct error");
    }

    unique_ptr(unique_ptr&& rhs) noexcept                           // (5)
    : m_pair(tiny_stl::forward<Deleter>(rhs.get_deleter()), rhs.release()) { }


    template <typename U, typename D,                               // (6)
        typename = enable_if_t<is_array<U>::value
            && is_same<pointer, element_type*>::value
            && is_same<unique_ptr<U, D>::pointer, unique_ptr<U, D>::element_type*>::value
            && is_convertible<unique_ptr<U, D>::element_type(*)[], element_type(*)[]>::value
            && (is_reference<Deleter>::value && is_same<D, Deleter>::value
                || !is_reference<Deleter>::value && is_convertible<D, Deleter>::value)>>
    unique_ptr(unique_ptr<U, D>&& rhs) noexcept 
    : m_pair(tiny_stl::forward<D>(rhs.get_deleter()), rhs.release()) { }

    unique_ptr(const unique_ptr&) = delete;

    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr& operator=(unique_ptr&& rhs) noexcept 
    {
        assert(this != tiny_stl::addressof(rhs));
        reset(rhs.release());
        get_deleter() = tiny_stl::move(rhs.get_deleter());
        return *this;
    }

    template <typename U, typename D,
        typename = enable_if_t<is_array<U>::value
            && is_same<pointer, element_type*>::value
            && is_same<unique_ptr<U, D>::pointer, unique_ptr<U, D>::element_type*>::value
            && is_convertible<unique_ptr<U, D>::element_type(*)[], element_type(*)[]>::value
            && is_assignable<Deleter&, D&&>::value>>
    unique_ptr& operator=(unique_ptr<U, D>&& rhs) noexcept
    {
        reset(rhs.release());
        get_deleter() = tiny_stl::forward<D>(rhs.get_deleter());
        return *this;
    }

    unique_ptr& operator=(nullptr_t) noexcept 
    {
        reset();
        return *this;
    }
    
    delete_type& get_deleter() noexcept 
    {
        return m_pair.get_first();
    }

    const delete_type& get_deleter() const noexcept
    {
        return m_pair.get_first();
    }

    pointer get() const noexcept
    {
        return _Get_ptr();
    }

    pointer release() noexcept
    {
        pointer ret = get();
        _Get_ptr() = pointer();
        return ret;
    }

    template <typename U, 
        typename = _Enalble_ctor<U, false_type>>
    void reset(U p) noexcept 
    {
        pointer old = get();
        _Get_ptr() = p;
        if (old != nullptr)
            get_deleter()(old);
    }

    void reset(nullptr_t) noexcept
    {
        reset(pointer());
    }

    explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

    void swap(unique_ptr& rhs) noexcept 
    {
        _Swap_ADL(_Get_ptr(), rhs._Get_ptr());
        _Swap_ADL(get_deleter(), rhs.get_deleter());
    }

    ~unique_ptr() noexcept
    {
        if (get() != nullptr)
            this->get_deleter()(get());
    }

    T& operator[](size_t i) const 
    {
        return get()[i];
    }
};  // class unique_ptr<T[], Deleter>

template <typename T, typename... Args, 
    typename = enable_if_t<!is_array<T>::value>>
inline unique_ptr<T> make_unique(Args&&... args) 
{
    return unique_ptr<T>(new T(tiny_stl::forward<Args>(args)...));
}

template <typename T, 
    typename = enable_if_t<is_array<T>::value && std::extent<T>::value == 0>>
inline unique_ptr<T> make_unique(size_t size) 
{
    using E = std::remove_extent_t<T>;
    return unique_ptr<T>(new E[size]);
}

template <typename T, typename... Args,
    typename = enable_if_t<std::extent<T>::value != 0>>
void make_unique(Args&&...) = delete;

template <typename T, typename D>
struct hash<unique_ptr<T, D>>
{
    using argument_type = unique_ptr<T, D>;
    using result_type = size_t;

    size_t operator()(const unique_ptr<T, D>& up) const noexcept
    {
        return hash<T>{}(*up);
    }
};

template <typename T1, typename D1, typename T2, typename D2>
inline bool operator==(const unique_ptr<T1, D1>& lhs, 
                       const unique_ptr<T2, D2>& rhs) 
{
    return lhs.get() == rhs.get();
}

template <typename T1, typename D1, typename T2, typename D2>
inline bool operator!=(const unique_ptr<T1, D1>& lhs, 
                       const unique_ptr<T2, D2>& rhs)
{
    return !(lhs == rhs);
}

template <typename T1, typename D1, typename T2, typename D2>
inline bool operator<(const unique_ptr<T1, D1>& lhs, 
                      const unique_ptr<T2, D2>& rhs)
{
    using Common_t = 
        typename std::common_type<
            typename unique_ptr<T1, D1>::pointer, 
            typename unique_ptr<T2, D2>::pointer>::type;
    return less<Common_t>(lhs.get(), rhs.get());
}

template <typename T1, typename D1, typename T2, typename D2>
inline bool operator>(const unique_ptr<T1, D1>& lhs, 
                      const unique_ptr<T2, D2>& rhs)
{
    return rhs < lhs;
}

template <typename T1, typename D1, typename T2, typename D2>
inline bool operator<=(const unique_ptr<T1, D1>& lhs, 
                       const unique_ptr<T2, D2>& rhs) {
    return !(rhs < lhs);
}

template <typename T1, typename D1, typename T2, typename D2>
inline bool operator>=(const unique_ptr<T1, D1>& lhs, 
                       const unique_ptr<T2, D2>& rhs) {
    return !(lhs < rhs);
}

template <typename T, typename D>
inline bool operator==(const unique_ptr<T, D>& lhs, nullptr_t) noexcept 
{
    return !lhs;
}

template <typename T, typename D>
inline bool operator==(nullptr_t, const unique_ptr<T, D>& rhs) noexcept 
{
    return !rhs;
}

template <typename T, typename D>
inline bool operator!=(const unique_ptr<T, D>& lhs, nullptr_t) noexcept 
{
    return (bool)lhs;
}

template <typename T, typename D>
inline bool operator!=(nullptr_t, const unique_ptr<T, D>& rhs) noexcept
{
    return (bool)rhs;
}

template <typename T, typename D>
inline bool operator<(const unique_ptr<T, D>& lhs, nullptr_t) 
{
    return less<typename unique_ptr<T, D>::pointer>()(lhs.get(), nullptr);
}

template <typename T, typename D>
inline bool operator<(nullptr_t, const unique_ptr<T, D>& rhs) 
{
    return less<typename unique_ptr<T, D>::pointer>()(nullptr, rhs.get());
}

template <typename T, typename D>
inline bool operator>(const unique_ptr<T, D>& lhs, nullptr_t) 
{
    return nullptr < lhs;
}

template <typename T, typename D>
inline bool operator>(nullptr_t, const unique_ptr<T, D>& rhs) 
{
    return rhs < nullptr;
}

template <typename T, typename D>
inline bool operator<=(const unique_ptr<T, D>& lhs, nullptr_t) 
{
    return !(nullptr < lhs);
}

template <typename T, typename D>
inline bool operator<=(nullptr_t, const unique_ptr<T, D>& rhs) 
{
    return !(rhs < nullptr);
}

template <typename T, typename D>
inline bool operator>=(const unique_ptr<T, D>& lhs, nullptr_t)
{
    return !(lhs < nullptr);
}

template <typename T, typename D>
inline bool operator>=(nullptr_t, const unique_ptr<T, D>& rhs)
{
    return !(nullptr < rhs);
}


template <typename T, typename D, 
    typename = enable_if_t<std::_Is_swappable<D>::value>>
inline void swap(unique_ptr<T, D>& lhs, unique_ptr<T, D>& rhs) noexcept 
{
    lhs.swap(rhs);
}

template <typename T>
class shared_ptr;

template <typename T>
class weak_ptr;

// reference MSVC implement

// reference count abstract base class
class _Ref_count_base
{
protected:
    using _Atomic_counter_t = unsigned long;
private:
    virtual void _Destroy() noexcept = 0;
    virtual void _Delete_this() noexcept = 0;
    
private:
    // assure use count operation is threads safe
    // why not use atomic
    _Atomic_counter_t mUses;
    _Atomic_counter_t mWeaks;

protected:
    _Ref_count_base()
        : mUses(1), mWeaks(1)   // no-atomic initialization
    {
        // constructor
    }

public:

    virtual ~_Ref_count_base() noexcept
    {
    }

    bool _IncRefNotZero()
    {
        // increment use count if not zero, return true if successful
        for (;;)
        {
            // loop until state is known
            _Atomic_counter_t count = 
                static_cast<volatile _Atomic_counter_t&>(mUses);
            if (count == 0)
            {
                return false;
            }

            // _InterlockedCompareExchange is Microsoft Specific
            // so the code is not able to cross platform
            if (static_cast<_Atomic_counter_t>(_InterlockedCompareExchange(
                    reinterpret_cast<volatile long*>(&mUses),
                    count + 1, count)) == count)
            {
                return true;
            }
        }
    }

    long _AtomicInc(_Atomic_counter_t& c)
    {
        return _InterlockedIncrement(reinterpret_cast<volatile long*>(&c));
    }

    long _AtomicDec(_Atomic_counter_t& c)
    {
        return _InterlockedDecrement(reinterpret_cast<volatile long*>(&c));
    }

    void _IncRef()
    {
        _AtomicInc(mUses);
    }

    void _IncWref()
    {
        _AtomicInc(mWeaks);
    }

    void _DecRef()
    {
        if (_AtomicDec(mUses) == 0)
        {
            // destroy managed resource, decrement the weak reference count
            _Destroy();
            _DecWref();
        }
    }

    void _DecWref()
    {
        if (_AtomicDec(mWeaks) == 0)
        {
            _Delete_this();
        }
    }

    long _Use_count() const noexcept
    {
        return static_cast<long>(mUses);
    }

    virtual void* _Get_deleter(const type_info&) const noexcept
    {
        return nullptr;
    }
};

template <typename T>
class _Ref_count : public _Ref_count_base
{
public:
    explicit _Ref_count(T* p)
        : _Ref_count_base(), mPtr(p)
    {
    }

private:
    virtual void _Destroy() noexcept override
    {
        delete mPtr;
    }

    virtual void _Delete_this() noexcept override
    {
        delete this;
    }

private:
    T* mPtr;
};

// handle reference counting for object with deleter
template <typename T, typename D>
class _Ref_count_resource : public _Ref_count_base
{
public:
    _Ref_count_resource(T p, D d)
        : _Ref_count_base(), mPair(tiny_stl::move(d), p)
    {
    }

    virtual void* _Get_deleter(const type_info& type) const noexcept override
    {
        if (type == typeid(D))
        {
            return const_cast<D*>(tiny_stl::addressof(mPair.get_first()));
        }

        return nullptr;
    }

private:
    virtual void _Destroy() noexcept override
    {
        mPair.get_first()(mPair.get_second());
    }

    virtual void _Delete_this() noexcept override
    {
        delete this;
    }

private:
    extra::compress_pair<D, T> mPair;
};


// handle refernece counting for object with deleter and allocator
template <typename T, typename D, typename Alloc>
class _Ref_count_resource_alloc : public _Ref_count_base
{
private:
    // allocator<_Ref_count_resource_alloc>
    using _MyAlty = _Get_bind_type<Alloc, _Ref_count_resource_alloc>;
public:
    _Ref_count_resource_alloc(T p, D d, const Alloc& alloc)
        : _Ref_count_base()
    {
    }

    virtual void* _Get_deleter(const type_info& type) const noexcept override
    {
        if (type == typeid(D))
        {
            return const_cast<D*>(tiny_stl::addressof(mPair.get_first()));
        }
        
        return nullptr;
    }

private:
    virtual void _Destroy() noexcept override
    {
        mPair.get_first()(mPair.get_second().get_second());
    }

    virtual void _Delete_this() noexcept override
    {
        _MyAlty alloc = mPair.get_second().get_first();
        allocator_traits<_MyAlty>::destroy(alloc, this);
        allocator_traits<_MyAlty>::deallocate(alloc, this, 1);
    }

private:
    extra::compress_pair<D, extra::compress_pair<_MyAlty, T>> mPair;
};

namespace 
{

template <typename T, typename = void>
struct _Can_enable_shared : false_type
{
};

// derived class is convertible to base class
template <typename T>
struct _Can_enable_shared<T, void_t<typename T::_Esft_unique_type>>
    : is_convertible<remove_cv_t<T>*, typename T::_Esft_unique_type*>::type
{
};

template <typename Other, typename U>
void _Enable_shared_from_this_base(const shared_ptr<Other>& sp, U* ptr, true_type)
{
    if (ptr && ptr->mWptr.expired())
    {
        ptr->mWptr = shared_ptr<remove_cv_t<U>>(sp, 
            const_cast<remove_cv_t<U>*>(ptr));
    }
}

template <typename Other, typename U>
void _Enable_shared_from_this_base(const shared_ptr<Other>&, U*, false_type)
{
}

} // unnamed namespace

  // no c++17 TODO
template <typename Other, typename U>
void _Enable_shared_from_this(const shared_ptr<Other>& sp, U* ptr)
{
    _Enable_shared_from_this_base(sp, ptr,
        _Can_enable_shared<U>{});
}

// base class for shared_ptr and weak_ptr 
template <typename T>
class _Ptr_base
{
public:
#ifdef TINY_STL_CXX17
    using element_type = remove_extent_t<T>;
#else
    using element_type = T;
#endif // TINY_STL_CXX17

public:
    long use_count() const noexcept
    {
        return mRep ? mRep->_Use_count() : 0;
    }

    template <typename U>
    bool owner_before(const _Ptr_base<U>& rhs) const noexcept
    {
        // compare the be managed object pointer
        return mPtr < rhs.mPtr;
    }

    element_type* get() const noexcept
    {
        return mPtr;
    }

    _Ptr_base(const _Ptr_base&) = delete;
    _Ptr_base& operator=(const _Ptr_base&) = delete;

protected:
    
    constexpr _Ptr_base() noexcept = default;
    ~_Ptr_base() = default;

    template <typename U>
    void _Move_construct_from(_Ptr_base<U>&& rhs)
    {
        // implement shared_ptr's move ctor and weak_ptr's move ctor
        mPtr = rhs.mPtr;
        mRep = rhs.mRep;

        rhs.mPtr = nullptr;
        rhs.mRep = nullptr;
    }

    template <typename U>
    void _Copy_construct_from(const shared_ptr<U>& rhs)
    {
        // implement shared_ptr's copy ctor
        if (rhs.mRep)
        {
            rhs.mRep->_IncRef();
        }
        
        mPtr = rhs.mPtr;
        mRep = rhs.mRep;
    }

    template <typename U>
    void _Alias_construct_from(const shared_ptr<U>& rhs, element_type* p)
    {
        // implement shared_ptr's aliasing ctor 
        if (rhs.mRep)
        {
            rhs.mRep->_IncRef();
        }

        mPtr = p;
        mRep = rhs.mRep;
    }

    template <typename U>
    friend class weak_ptr;

    template <typename U>
    bool _Construct_from_weak(const weak_ptr<U>& rhs)
    {
        if (rhs.mRep && rhs.mRep->_IncRefNotZero())
        {
            mPtr = rhs.mPtr;
            mRep = rhs.mRep;
            return true;
        }

        return false;
    }

    void _DecRef()
    {
        if (mRep)
        {
            mRep->_DecRef();
        }
    }
    
    void _DecWref()
    {
        if (mRep)
        {
            mRep->_DecWref();
        }
    }

    void _Swap(_Ptr_base& rhs) noexcept
    {
        tiny_stl::swap(mPtr, rhs.mPtr);
        tiny_stl::swap(mRep, rhs.mRep);
    }

    void _Set_ptr_rep(element_type* ptr, _Ref_count_base* rep)
    {
        mPtr = ptr;
        mRep = rep;
    }

    template <typename U>
    void _Weakly_construct_from(const _Ptr_base<U>& rhs)
    {
        // implement weak_ptr's ctor
        if (rhs.mRep)
        {
            rhs.mRep->_IncWref();
        }

        mPtr = rhs.mPtr;
        mRep = rhs.mRep;
    }

    template <typename U>
    friend class _Ptr_base;

private:
    element_type* mPtr{nullptr};
    _Ref_count_base* mRep{ nullptr };

    template <typename D, typename U>
    friend D* get_deleter(const shared_ptr<U>& sp) noexcept;
};

template <typename T>
class shared_ptr : public _Ptr_base<T>
{
public:
#ifdef TINY_STL_CXX17
    using weak_type = weak_ptr<T>;    
#endif // TINY_STL_CXX17
    using element_type = typename _Ptr_base<T>::element_type;

    constexpr shared_ptr() noexcept
    {
    }

    constexpr shared_ptr(nullptr_t) noexcept
    {
    }
    
    // no c++17, in other words, no shared_ptr<T[]>
    // I can't implement it
    // TODO: support c++17
    template <typename U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
    explicit shared_ptr(U* ptr)
    {
        _Setp(ptr);
    }

    // c++17
    template <typename U, typename D, enable_if_t<conjunction_v<
        std::is_move_constructible<D>, 
        is_convertible<U, T>,
        void_t<decltype(declval<D&>()(declval<T*&>()))>>, int> = 0>
    shared_ptr(U* ptr, D d)
    {
        _Setpd(ptr, tiny_stl::move(d));
    }

    template <typename D, enable_if_t<conjunction_v<
        std::is_move_constructible<D>,
        void_t<decltype(declval<D&>()(declval<T*&>()))>>, int> = 0>
    shared_ptr(std::nullptr_t, D d)
    {
        _Setpd(nullptr, tiny_stl::move(d));
    }

    template <typename U, typename D, typename Alloc, enable_if_t<conjunction_v<
        std::is_move_constructible<D>,
        is_convertible<U, T>,
        void_t<decltype(declval<D&>()(declval<U*&>()))>>, int> = 0>
    shared_ptr(U* ptr, D d, Alloc alloc)
    {
        _Setpda(ptr, tiny_stl::move(d), alloc);
    }

    template <typename D, typename Alloc, enable_if_t<conjunction_v<
        std::is_move_constructible<D>,
        void_t<decltype(declval<D&>()(declval<T*&>()))>>, int> = 0>
    shared_ptr(std::nullptr_t, D d, Alloc alloc)
    {
        _Setpda(nullptr, tiny_stl::move(d), alloc);
    }

    template <typename U>
    shared_ptr(const shared_ptr<U>& rhs, element_type* ptr) noexcept
    {
        this->_Alias_construct_from(rhs, ptr);
    }

    shared_ptr(const shared_ptr& rhs) noexcept
    {
        this->_Copy_construct_from(rhs);
    }

    // no c++17
    template <typename U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
    shared_ptr(const shared_ptr<U>& rhs) noexcept
    {
        this->_Copy_construct_from(rhs);
    }

    shared_ptr(shared_ptr&& rhs) noexcept
    {
        this->_Move_construct_from(tiny_stl::move(rhs));
    }

    // no c++17
    template <typename U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
    shared_ptr(shared_ptr<U>&& rhs) noexcept
    {
        this->_Move_construct_from(tiny_stl::move(rhs));
    }

    // no c++17
    template <typename U, enable_if_t<is_convertible_v<U*, T*>, int> = 0>
    explicit shared_ptr(const weak_ptr<U>& other)
    {
        if (!this->_Construct_from_weak(other))
        {
            throw std::bad_weak_ptr{};
        }
    }

    // no c++17
    template <typename U, typename D, enable_if_t<
        is_convertible_v<typename unique_ptr<U, D>::pointer, T*>, int> = 0>
    shared_ptr(unique_ptr<U, D>&& other)
    {
        const U* ptr = other.get();

        using Deleter = conditional_t<is_reference_v<D>,
            decltype(std::ref(rhs.get_deleter())),
            D>;

        if (ptr)
        {
            const auto pRcObj = new _Ref_count_resource<U*, Deleter>{ ptr, other.get_deleter() };
            _Set_ptr_rep_and_enable_shared(ptr, pRcObj);
            other.release();
        }
    }

    ~shared_ptr() noexcept
    {
        this->_DecRef();
    }

    shared_ptr& operator=(const shared_ptr& rhs) noexcept
    {
        shared_ptr{ rhs }.swap(*this);
        return *this;
    }

    template <typename U>
    shared_ptr& operator=(const shared_ptr<U>& rhs) noexcept
    {
        shared_ptr{ rhs }.swap(*this);
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& rhs) noexcept
    {
        shared_ptr{ tiny_stl::move(rhs) }.swap(*this);
        return *this;
    }

    template <typename U>
    shared_ptr& operator=(shared_ptr<U>&& rhs) noexcept
    {
        shared_ptr{ tiny_stl::move(rhs) }.swap(*this);
        return *this;
    }

    template <typename U, typename D>
    shared_ptr& operator=(unique_ptr<U, D>&& other)
    {
        shared_ptr{ tiny_stl::move(rhs) }.swap(*this);
        return *this;
    }

    void swap(shared_ptr& rhs) noexcept
    {
        this->_Swap(rhs);
    }

    void reset() noexcept
    {
        shared_ptr{}.swap(*this);
    }

    template <typename U>
    void reset(U* ptr)
    {
        shared_ptr{ptr}.swap(*this);
    }

    template <typename U, typename D>
    void reset(U* ptr, D d)
    {
        // d must be copy constructible
        shared_ptr{ ptr, d }.swap(*this);
    }

    template <typename U, typename D, typename Alloc>
    void reset(U* ptr, D d, Alloc alloc)
    {
        shared_ptr{ ptr, d, alloc }.swap(*this);
    }

    using _Ptr_base<T>::get;

    // no c++17 TODO
    T& operator*() const noexcept
    {
        return *get();
    }

    T* operator->() const noexcept
    {
        return get();
    }

    // TODO: element_type& operator[](std::ptrdiff_t idx) (c++17)


    bool unique() const noexcept
    {
        return this->use_count() == 1;
    }

    explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

private:
    // TODO, support shared_ptr<T[]>
    template <typename U>
    void _Setp(U* ptr)
    {
        // strong exception guarantee
        try
        {
            _Set_ptr_rep_and_enable_shared(ptr, new _Ref_count<U>(ptr));
        }
        catch (...)
        {
            delete ptr;
            throw;
        }
    }

    template <typename UptrOrNullptr, typename D>
    void _Setpd(UptrOrNullptr ptr, D d)
    {
        try
        {
            _Set_ptr_rep_and_enable_shared(ptr,
                new _Ref_count_resource<UptrOrNullptr, D>);
        }
        catch (...)
        {
            d(ptr);
            throw;
        }
    }

    template <typename UptrOrNullptr, typename D, typename Alloc>
    void _Setpda(UptrOrNullptr ptr, D d, Alloc alloc)
    {
        using RcObj = _Ref_count_resource_alloc<UptrOrNullptr, D, Alloc>;
        using Al_alloc = typename allocator_traits<Alloc>::template rebind_alloc<RcObj>;
        using Al_traits = allocator_traits<Al_alloc>;

        Al_alloc al_alloc{ alloc };

        try
        {
            const auto pRcObj = Al_traits::allocate(al_alloc, 1);
            RcObj* const pref = tiny_stl::addressof(*pRcObj);
            try
            {
                Al_traits::construct(alloc, pref, ptr, tiny_stl::move(d), alloc);
                _Set_ptr_rep_and_enable_shared(ptr, pref);
            }
            catch (...)
            {
                Al_traits::deallocate(al_alloc, pRcObj, 1);
                throw;
            }
        }
        catch (...)
        {
            d(ptr);
        }
    }

    template <typename U>
    void _Set_ptr_rep_and_enable_shared(U* ptr, _Ref_count_base* rep)
    {
        this->_Set_ptr_rep(ptr, rep);
        _Enable_shared_from_this(*this, ptr);
    }

    void _Set_ptr_rep_and_enable_shared(nullptr_t, _Ref_count_base* rep)
    {
        this->_Set_ptr_rep(nullptr, rep);
    }

    template <typename T0, typename... Args>
    friend shared_ptr<T0> make_shared(Args&&... args);

    template <typename T0, typename Alloc, typename... Args>
    friend shared_ptr<T0> allocate_shared(const Alloc& alloc, Args&&... args);

}; // class shared_ptr


namespace
{

template <typename T>
class _Ref_count_obj : public _Ref_count_base
{
public:
    template <typename... Args>
    explicit _Ref_count_obj(Args&&... args)
        : _Ref_count_base()
    {
        ::new (static_cast<void*>(&mStroage)) 
            T(tiny_stl::forward<T>(args)...);
    }

    T* _Get_ptr()
    {
        return reinterpret_cast<T*>(&mStroage);
    }
private:
    void _Destroy() noexcept override
    {
        _Get_ptr()->~T();
    }

    void _Delete_this() noexcept override
    {
        delete this;
    }

    std::aligned_union_t<1, T> mStroage;
};

} // unnamed namespace


template <typename T, typename... Args>
inline shared_ptr<T> make_shared(Args&&... args)
{
    const auto pRcX = new _Ref_count_obj<T>(tiny_stl::forward<Args>(args)...);

    shared_ptr<T> sp;
    sp._Set_ptr_rep_and_enable_shared(pRcX->_Get_ptr(), pRcX);
    return sp;
}

namespace
{

template <typename T, typename Alloc>
class _Ref_count_obj_alloc : public _Ref_count_base
{
public:
    template <typename... Args>
    explicit _Ref_count_obj_alloc(const Alloc& al, Args&&... args)
    {
        ::new (static_cast<void*>(&mPair.get_second())) 
            T(tiny_stl::forward<Args>(args)...);
    }

    T* _Get_ptr()
    {
        return reinterpret_cast<T*>(&mPair.get_second());
    }

private:
    using _MyAlty = typename allocator_traits<Alloc>::template 
        rebind_alloc<_Ref_count_obj_alloc>;

    void _Destroy() noexcept override
    {
        _Get_ptr()->~T();
    }

    void _Delete_this() noexcept override
    {
        _MyAlty al = mPair.get_first();
        allocator_traits<_MyAlty>::destroy(al, this);
        allocator_traits<_MyAlty>::deallocate(al, this, 1);
    }
    
    extra::compress_pair<_MyAlty, std::aligned_union_t<1, T>> mPair;
};

} // unnamed namespace

template <typename T, typename Alloc, typename... Args>
shared_ptr<T> allocate_shared(const Alloc& alloc, Args&&... args)
{
    using RcAlX = _Ref_count_obj_alloc<T, Alloc>;
    using Al_alloc = typename allocator_traits<Alloc>::template
        rebind_alloc<RcAlX>;
    using Al_traits = allocator_traits<Al_alloc>;

    Al_alloc al{ alloc };

    const auto pRcAlX = Al_traits::allocate(al, 1);

    try
    {
        Al_traits::construct(al, tiny_stl::addressof(*pRcAlX), 
            alloc, tiny_stl::forward<Args>(args)...);
    }
    catch (...)
    {
        Al_traits::deallocate(al, pRcAlX, 1);
    }

    shared_ptr<T> sp;
    sp._Set_ptr_rep_and_enable_shared(pRcAlX->_Get_ptr(), 
        tiny_stl::addressof(*pRcAlX));
    return sp;
}


template <typename T, typename U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U>& sp) noexcept
{
    const auto ptr = static_cast<typename shared_ptr<T>::element_type*>(sp.get());
    return shared_ptr<T>(sp, ptr);
}

template <typename T, typename U>
shared_ptr<T> const_pointer_cast(const shared_ptr<U>& sp) noexcept
{
    const auto ptr = const_cast<typename shared_ptr<T>::element_type*>(sp.get());
    return shared_ptr<T>(sp, ptr);
}

template <typename T, typename U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& sp) noexcept
{
    const auto ptr = dynamic_cast<typename shared_ptr<T>::element_type*>(sp.get());
    if (ptr)
    {
        return shared_ptr<T>(sp, ptr);
    }

    // dynamic_cast failed and <new_type> is pointer type
    // return nullptr
    return shared_ptr<T>{};
}

#ifdef TINY_STL_CXX17
template <typename T, typename U>
shared_ptr<T> reinterpret_pointer_cast(const shared_ptr<U>& sp) noexcept
{
    const auto ptr = reinterpret_cast<typename shared_ptr<T>::element_type*>(sp.get());
    return shared_ptr<T>(sp, ptr);
}
#endif

template <typename D, typename T>
D* get_deleter(const shared_ptr<T>& sp) noexcept
{
    if (sp->_Rep)
    {
        return static_cast<D*>(sp._Rep->_Get_deleter(typeid(D)));
    }

    return nullptr;
}

// shared_ptr<T1> compare with shared_ptr<T2>
template <typename T1, typename T2>
bool operator==(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) noexcept
{
    return lhs.get() == rhs.get();
}

template <typename T1, typename T2>
bool operator!=(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T1, typename T2>
bool operator<(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) noexcept
{
    return lhs.get() < rhs.get();
}

template <typename T1, typename T2>
bool operator>(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) noexcept
{
    return rhs < lhs;
}

template <typename T1, typename T2>
bool operator<=(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) noexcept
{
    return !(rhs < lhs);
}

template <typename T1, typename T2>
bool operator>=(const shared_ptr<T1>& lhs, const shared_ptr<T2>& rhs) noexcept
{
    return !(lhs < rhs);
}

// shared_ptr<T> compare with nullptr
template <typename T>
bool operator==(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
{
    return lhs.get() == nullptr;
}

template <typename T>
bool operator==(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
{
    return rhs.get() == nullptr;
}

template <typename T>
bool operator!=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
{
    return lhs.get() != nullptr;
}

template <typename T>
bool operator!=(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
{
    return rhs.get() != nullptr;
}

template <typename T>
bool operator<(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
{
    return less<typename shared_ptr<T>::element_type>()(lhs.get(), nullptr);
}

template <typename T>
bool operator<(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
{
    return less<typename shared_ptr<T>::element_type>()(nullptr, rhs.get());
}

template <typename T>
bool operator>(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
{
    return nullptr < lhs;
}

template <typename T>
bool operator>(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
{
    return rhs < nullptr;
}

template <typename T>
bool operator<=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
{
    return !(nullptr < lhs);
}

template <typename T>
bool operator<=(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
{
    return !(rhs < nullptr);
}

template <typename T>
bool operator>=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
{
    return !(lhs < nullptr);
}

template <typename T>
bool operator>=(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
{
    return !(nullptr < rhs);
}

template <typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os,
    const shared_ptr<T>& ptr)
{
    return os << ptr.get();
}

template <typename T>
void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) noexcept
{
    lhs.swap(rhs);
}

template <typename T>
class weak_ptr : public _Ptr_base<T>
{
public:
    constexpr weak_ptr() noexcept 
    {
    }

    weak_ptr(const weak_ptr& rhs) noexcept
    {
        this->_Weakly_construct_from(rhs);
    }

    // TODO
    template <typename U, enable_if_t<
        is_convertible_v<U*, T*>, int> = 0>
    weak_ptr(const weak_ptr<U>& rhs) noexcept
    {
        this->_Weakly_construct_from(rhs.lock());
    }

    template <typename U, enable_if_t<
        is_convertible_v<U*, T*>, int> = 0>
    weak_ptr(const shared_ptr<U>& rhs) noexcept
    {
        this->_Weakly_construct_from(rhs);
    }

    weak_ptr(weak_ptr&& rhs) noexcept
    {
        this->_Move_construct_from(tiny_stl::move(rhs));
    }

    template <typename U, enable_if_t<
        is_convertible_v<U*, T*>, int> = 0>
    weak_ptr(weak_ptr<U>&& rhs) noexcept
    {
        this->_Weakly_construct_from(rhs.lock());
        rhs.reset();
    }

    ~weak_ptr() noexcept
    {
        this->_DecWref();
    }

    weak_ptr& operator=(const weak_ptr& rhs) noexcept
    {
        weak_ptr(rhs).swap(*this);
        return *this;
    }

    template <typename U>
    weak_ptr& operator=(const weak_ptr<U>& rhs) noexcept
    {
        weak_ptr(rhs).swap(*this);
        return *this;
    }

    template <typename U>
    weak_ptr& operator=(const shared_ptr<U>& rhs) noexcept
    {
        weak_ptr(rhs).swap(*this);
        return *this;
    }

    weak_ptr& operator=(weak_ptr&& rhs) noexcept
    {
        weak_ptr(tiny_stl::move(rhs)).swap(*this);
        return *this;
    }

    template <typename U>
    weak_ptr& operator=(weak_ptr<U>&& rhs) noexcept
    {
        weak_ptr(tiny_stl::move(rhs)).swap(*this);
        return *this;
    }

    void reset() noexcept
    {
        weak_ptr().swap(*this);
    }

    void swap(weak_ptr& rhs) noexcept
    {
        this->_Swap(rhs);
    }

    // use_count() in base class

    // for checking validity of the pointer
    bool expired() const noexcept
    {
        return this->use_count() == 0;
    }

    shared_ptr<T> lock() const noexcept
    {
        return expired() ? shared_ptr<T>{} : shared_ptr<T>{ *this };
    }
};

template <typename T>
void swap(weak_ptr<T>& lhs, weak_ptr<T>& rhs) noexcept
{
    lhs.swap(rhs);
}


template <typename T>
class enable_shared_from_this
{
public:
    shared_ptr<T> shared_from_this()
    {
        return shared_ptr<T>(mWptr);
    }

    shared_ptr<const T> shared_from_this() const
    {
        return shared_ptr<const T>(mWptr);
    }

    weak_ptr<T> weak_from_this()
    {
        return mWptr;
    }

    weak_ptr<const T> weak_from_this() const
    {
        return mWptr;
    }
protected:
    constexpr enable_shared_from_this() noexcept
        : mWptr()
    {
    }

    enable_shared_from_this(const enable_shared_from_this&) noexcept
        : mWptr()
    {
    }

    enable_shared_from_this& operator=(const enable_shared_from_this&) noexcept
    {
        return *this;
    }

    ~enable_shared_from_this() = default;

private:
    // helper type to determine if TYPE has inherited enable_from_this
    using _Esft_unique_type = enable_shared_from_this;

    template <typename Other, typename U>
    friend void _Enable_shared_from_this(const shared_ptr<Other>& sp, U* ptr);

    mutable weak_ptr<T> mWptr;
};

}  // tiny_stl namespace