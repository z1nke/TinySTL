﻿#pragma once

#include "type_traits.h"
#include <type_traits>   // for the need to use but not implement

#if _MSVC_LANG >= 201402L
#   define TINY_STL_CXX14
#endif // _MSVC_LANG >= 201402L

#if _MSVC_LANG >= 201703L
#   define TINY_STL_CXX17
#endif // _MSVC_LANG >= 201703L

#ifdef TINY_STL_CXX14
#   define XCONSTEXPR14 constexpr
#else 
#   define XCONSTEXPR14 inline
#endif // TINY_STL_CXX14

namespace tiny_stl
{

template <typename T>
constexpr remove_reference_t<T>&&
move(T&& param) noexcept 
{
    return static_cast<remove_reference_t<T>&&>(param);
}

template <typename T>
constexpr T&& forward(remove_reference_t<T>& param) noexcept 
{
    return static_cast<T&&>(param);
}

template <typename T>
constexpr T&& forward(remove_reference_t<T>&& param) noexcept 
{
    return static_cast<T&&>(param);
}


template <typename T>
constexpr T* addressof(T& val) noexcept 
{
    return reinterpret_cast<T*>(
        &const_cast<char&>(
            reinterpret_cast<const volatile char&>(val)));
}

template <typename T>
const T* addressof(const T&&) = delete;

template <typename T>
inline void swap(T& lhs, T& rhs) 
        noexcept(is_nothrow_move_constructible<T>::value
            && std::is_nothrow_move_assignable<T>::value) 
{
    T tmp = tiny_stl::move(lhs);
    lhs   = tiny_stl::move(rhs);
    rhs   = tiny_stl::move(tmp);
}

template <typename FwdIter1, typename FwdIter2>
inline void iter_swap(FwdIter1 lhs, FwdIter2 rhs) 
{
    swap(*lhs, *rhs);
}

// swap array lhs and rhs
template <typename T, size_t N>
inline void swap(T(&lhs)[N], T(&rhs)[N])
        noexcept(std::_Is_nothrow_swappable<T>::value) 
{
    if (&lhs != &rhs) 
    {
        T* first1 = lhs;
        T* last1  = lhs + N;
        T* first2 = rhs;

        for (; first1 != last1; ++first1, ++first2) 
        {
            tiny_stl::iter_swap(first1, first2);
        }
    }
}

template <typename T>
inline void _Swap_ADL(T& lhs, T& rhs) 
    noexcept(std::_Is_nothrow_swappable<T>::value) 
{
    // ADL: argument-dependent lookup
    swap(lhs, rhs);
    // std::swap(a, b);
    // maybe => 
    // using std::swap;
    // swap(a, b);
}


struct piecewise_construct_t 
{
    explicit piecewise_construct_t() = default;
};

constexpr piecewise_construct_t piecewise_construct{ };


template <typename...>
class tuple;

template <typename T1, typename T2>
struct pair 
{
    using first_type  = T1;
    using second_type = T2;

    first_type first;
    second_type second;

    // Constructor
    template <typename U1 = T1,             // (1)
        typename U2 = T2,
        typename = enable_if_t<is_default_constructible<U1>::value
                        && is_default_constructible<U2>::value>>
    /*explicit*/constexpr pair() 
        : first(), second() { }

    template <typename U1 = T1,             // (2)
        typename U2 = T2,
        typename = enable_if_t<is_copy_constructible<U1>::value
                        && is_copy_constructible<U2>::value>,
        enable_if_t<!is_convertible<const U1&, U1>::value
            || !is_convertible<const U2&, U2>::value, int> = 0>
    constexpr explicit pair(const T1& f, const T2& s)
        : first(f), second(s) {}
    
    template <typename U1 = T1,             // (2)
        typename U2 = T2,
        typename = enable_if_t<is_copy_constructible<U1>::value
                        && is_copy_constructible<U2>::value>,
        enable_if_t<is_convertible<const U1&, U1>::value
            && is_convertible<const U2&, U2>::value, int> = 0>
    constexpr pair(const T1& f, const T2& s) 
        : first(f), second(s) {}

    template <typename U1,                  // (3)
        typename U2,
        typename = enable_if_t<is_constructible<T1, U1&&>::value
                        && is_constructible<T2, U2&&>::value>,
        enable_if_t<!is_convertible<U1&&, T1>::value
            || !is_convertible<U2&&, T2>::value, int> = 0>
    constexpr explicit pair(U1&& f, U2&& s) 
        : first(tiny_stl::forward<U1>(f)),
            second(tiny_stl::forward<U2>(s)) {}
    
    template <typename U1,                  // (3)
        typename U2,
        typename = enable_if_t<is_constructible<T1, U1&&>::value
                        && is_constructible<T2, U2&&>::value>,
        enable_if_t<is_convertible<U1&&, T1>::value
            && is_convertible<U2&&, T2>::value, int> = 0>
    constexpr pair(U1&& f, U2&& s)
        : first(tiny_stl::forward<U1>(f)),
            second(tiny_stl::forward<U2>(s)) {}

    template <typename U1,                  // (4) 
        typename U2,
        typename = enable_if_t<is_constructible<T1, const U1&>::value
                        && is_constructible<T2, const U2&>::value>,
        enable_if_t<!is_convertible<const U1&, T1>::value
            || !is_convertible<const U2&, T2>::value, int> = 0>
    constexpr explicit pair(const pair<U1, U2>& rhs)
        : first(rhs.first), second(rhs.second) {}

    template <typename U1,                  // (4) 
        typename U2,
        typename = enable_if_t<is_constructible<T1, const U1&>::value
                        && is_constructible<T2, const T2&>::value>,
        enable_if_t<is_convertible<const U1&, T1>::value
            && is_convertible<const U2&, T2>::value, int> = 0>
    constexpr pair(const pair<U1, U2>& rhs) 
        : first(rhs.first), second(rhs.second) {}

    template <typename U1,                  // (5)
        typename U2,
        typename = enable_if_t<is_constructible<T1, U1&&>::value
                        && is_constructible<T2, U2&&>::value>,
        enable_if_t<!is_convertible<U1&&, T1>::value
            || !is_convertible<U1&&, T1>::value, int> = 0>
    constexpr explicit pair(pair<U1, U2>&& rhs) 
        : first(tiny_stl::forward<U1>(rhs.first)),
            second(tiny_stl::forward<U2>(rhs.second)) {}
    
    template <typename U1,                  // (5)
        typename U2,
        typename = enable_if_t<is_constructible<T1, U1&&>::value
                        && is_constructible<T2, U2&&>::value>,
        enable_if_t<is_convertible<U1&&, T1>::value
            && is_convertible<U1&&, T1>::value, int> = 0>
    constexpr pair(pair<U1, U2>&& rhs) 
        : first(tiny_stl::forward<U1>(rhs.first)),
            second(tiny_stl::forward<U2>(rhs.second)) {}

    template <typename... Args1,            // (6)  
        typename... Args2>
    pair(piecewise_construct_t,
        tuple<Args1...> t1,
        tuple<Args2...> t2);

    pair(const pair& p) = default;          // (7)

    pair(pair&&) = default;                 // (8)


    // operator=()
    pair& operator=(const pair& rhs)        // (1)
    {     
        first  = rhs.first;
        second = rhs.second;
        return *this;
    }

    template <typename U1, typename U2>     // (2)
    pair& operator=(const pair<U1, U2>& rhs) 
    {
        first  = rhs.first;
        second = rhs.second;
        return *this;
    }

    pair& operator=(pair&& rhs)             // (3)
        noexcept(std::is_nothrow_move_assignable<T1>::value
            && std::is_nothrow_move_assignable<T2>::value) 
    {
        first  = tiny_stl::forward<T1>(rhs.first);
        second = tiny_stl::forward<T2>(rhs.second);
        return *this;
    }

    template <typename U1, typename U2>     // (4)
    pair& operator=(pair<U1, U2>&& rhs) 
    {
        first  = tiny_stl::forward<T1>(rhs.first);
        second = tiny_stl::forward<T2>(rhs.second);
    }


    void swap(pair& rhs) 
        noexcept(std::_Is_nothrow_swappable<first_type>::value
            && std::_Is_nothrow_swappable<second_type>::value) 
    {
        if (this != tiny_stl::addressof(rhs)) 
        {
            _Swap_ADL(first, rhs.first);
            _Swap_ADL(second, rhs.second);
        }
    }
};  // class pair<T1, T2>

// if reference_wrap<T> -> T&
// else                 -> decay_t<T>

template <typename T1, typename T2>
constexpr pair<typename _Unrefwrap<T1>::type,
               typename _Unrefwrap<T2>::type>
make_pair(T1&& t1, T2&& t2) 
{
    using Type_pair = pair<
        typename _Unrefwrap<T1>::type,
        typename _Unrefwrap<T2>::type>;
    return Type_pair(tiny_stl::forward<T1>(t1),
        tiny_stl::forward<T2>(t2));
}

template <typename T1, typename T2>
constexpr bool operator==(const pair<T1, T2>& lhs, 
                          const pair<T1, T2>& rhs) 
{
    return (lhs.first == rhs.first && lhs.second == lhs.second);
}

template <typename T1, typename T2>
constexpr bool operator!=(const pair<T1, T2>& lhs,
                          const pair<T1, T2>& rhs) 
{
    return (!(lhs == rhs));
}

template <typename T1, typename T2>
constexpr bool operator<(const pair<T1, T2>& lhs,
                         const pair<T1, T2>& rhs) 
{
    return (lhs.first < rhs.first) ||
        (!(lhs.first < rhs.first) && lhs.second < rhs.second);
}

template <typename T1, typename T2>
constexpr bool operator<=(const pair<T1, T2>& lhs, 
                          const pair<T1, T2>& rhs) 
{
    return (!(rhs < lhs));
}

template <typename T1, typename T2>
constexpr bool operator>(const pair<T1, T2>& lhs,
                         const pair<T1, T2>& rhs) 
{
    return (rhs < lhs);
}

template <typename T1, typename T2>
constexpr bool operator>=(const pair<T1, T2>& lhs, 
                          const pair<T1, T2>&) 
{
    return (!(lhs < rhs));
}

template <typename T1, typename T2>
inline void swap(pair<T1, T2>& lhs, pair<T1, T2>& rhs) 
    noexcept(noexcept(lhs.swap(rhs))) 
{
    lhs.swap(rhs);
}


template <typename T>
struct tuple_size;

// tuple_size to pair
template <typename T1, typename T2>
struct tuple_size<pair<T1, T2>> 
    : integral_constant<size_t, 2> {};

// tuple_size to tuple
template <typename... Ts>
struct tuple_size<tuple<Ts...>> 
    : integral_constant<size_t, sizeof...(Ts)> { };

template <typename T>
struct tuple_size<const T>
    : integral_constant<size_t, tuple_size<T>::value> { };

template <typename T>
struct tuple_size<volatile T>
    : integral_constant<size_t, tuple_size<T>::value> { };

template <typename T>
struct tuple_size<const volatile T>
    : integral_constant<size_t, tuple_size<T>::value> { };


// tuple element
template <typename T>
struct _Always_false : false_type { };


template <size_t Idx, typename Tuple>
struct tuple_element;


// tuple_element to tuple
template <size_t Idx>
struct tuple_element<Idx, tuple<>> 
{
    static_assert(_Always_false<integral_constant<size_t, Idx>>::value, 
        "tuple index out of range");
};

template <typename Head, typename... Tail> 
struct tuple_element<0, tuple<Head, Tail...>>
{
    using type = Head;
    using TupleType = tuple<Head, Tail...>;
};

template <size_t Idx, typename Head, typename... Tail>
struct tuple_element<Idx, tuple<Head, Tail...>>
    : tuple_element<Idx - 1, tuple<Tail...>> { };

template <size_t Idx, typename Tuple>
struct tuple_element<Idx, const Tuple>
    : tuple_element<Idx, Tuple> 
{
    using _BaseType = tuple_element<Idx, Tuple>;
    using type = add_const_t<typename _BaseType::type>;
};

template <size_t Idx, typename Tuple>
struct tuple_element<Idx, volatile Tuple> 
    : tuple_element<Idx, Tuple>
{
    using _BaseType = tuple_element<Idx, Tuple>;
    using type = add_volatile_t<typename _BaseType::type>;
};

template <size_t Idx, typename Tuple>
struct tuple_element<Idx, const volatile Tuple>
    : tuple_element<Idx, Tuple> 
{
    using _BaseType = tuple_element<Idx, Tuple>;
    using type = add_cv_t<typename _BaseType::type>;
};


// tuple_element to pair
template <typename T1, typename T2>
struct tuple_element<0, pair<T1, T2>> 
{
    using type = T1;
};

template <typename T1, typename T2>
struct tuple_element<1, pair<T1, T2>> 
{
    using type = T2;
};

template<size_t Idx, typename T>
using tuple_element_t = typename tuple_element<Idx, T>::type;

template <typename Ret, typename Pair>
constexpr Ret _Pair_get_helper(Pair& p,
    integral_constant<size_t, 0>) noexcept 
{
    return (p.first);
}

template <typename Ret, typename Pair>
constexpr Ret _Pair_get_helper(Pair& p,
    integral_constant<size_t, 1>) noexcept
{
    return (p.second);
}

// get<Idx>(pair<T1, T2>)
template <size_t Idx, typename T1, typename T2>     // (1) C++ 14
constexpr tuple_element_t<Idx, pair<T1, T2>>& 
get(pair<T1, T2>& p) noexcept 
{
    using Ret = tuple_element_t<Idx, pair<T1, T2>>&;
    return _Pair_get_helper<Ret>(p, integral_constant<size_t, Idx>{});
}

template <size_t Idx, typename T1, typename T2>     // (2) C++ 14
constexpr const tuple_element_t<Idx, pair<T1, T2>>&
get(const pair<T1, T2>& p) noexcept 
{
    using Ret = const tuple_element_t<Idx, pair<T1, T2>>&;
    return _Pair_get_helper<Ret>(p, integral_constant<size_t, Idx>{});
}

template <size_t Idx, typename T1, typename T2>     // (3) C++ 14
constexpr tuple_element_t<Idx, pair<T1, T2>>&&
get(pair<T1, T2>&& p) noexcept 
{
    using Ret = tuple_element_t<Idx, pair<T1, T2>>&&;
    return tiny_stl::forward<Ret>(get<Idx>(p));
}

template <size_t Idx, typename T1, typename T2>     // (4) C++ 17
constexpr const tuple_element_t<Idx, pair<T1, T2>>&&
get(const pair<T1, T2>&& p) noexcept 
{
    using Ret = const tuple_element_t<Idx, pair<T1, T2>>&&;
    return tiny_stl::forward<Ret>(get<Idx>(p));
}

template <typename T1, typename T2>                 // (5) C++ 14
constexpr T1& get(pair<T1, T2>& p) noexcept 
{
    return get<0>(p);
}

template <typename T1, typename T2>                 // (6) C++ 14
constexpr const T1& get(const pair<T1, T2>& p) noexcept 
{
    return get<0>(p);
}

template <typename T1, typename T2>                 // (7) C++ 14
constexpr T1&& get(pair<T1, T2>&& p) noexcept 
{
    return get<0>(tiny_stl::move(p));
}

template <typename T1, typename T2>                 // (8) C++ 17
constexpr const T1&& get(const pair<T1, T2>&& p) noexcept 
{
    return get<0>(tiny_stl::move(p));
}

template <typename T1, typename T2>                 // (9) C++ 14
constexpr T1& get(pair<T2, T1>& p) noexcept 
{
    return get<1>(p);
}

template <typename T1, typename T2>                 // (10) C++ 14
constexpr const T1& get(const pair<T2, T1>& p) noexcept 
{
    return get<1>(p);
}

template <typename T1, typename T2>                 // (11) C++ 14
constexpr T1&& get(pair<T2, T1>&& p) noexcept 
{
    return get<1>(tiny_stl::move(p));
}

template <typename T1, typename T2>                 // (12) C++ 17
constexpr const T1&& get(const pair<T2, T1>&& p) noexcept 
{
    return get<1>(tiny_stl::move(p));
}

namespace extra 
{ 

// compress pair
template <typename T1, typename T2, 
    bool = is_empty<T1>::value && !is_final<T1>::value>
class compress_pair final : private T1 
{
public:
    using first_type  = T1;
    using second_type = T2;
private:
    T2 second;
    using _Base = T1;

public:
    constexpr explicit compress_pair() : T1(), second() { }

    template <typename U1, typename... U2>
    compress_pair(U1&& f, U2&&... s) 
    : T1(tiny_stl::forward<U1>(f)), second(tiny_stl::forward<U2>(s)...) { }

    T1& get_first() noexcept 
    {
        return *this;
    }

    const T1& get_first() const noexcept 
    {
        return *this;
    }

    T2& get_second() noexcept 
    {
        return second;
    }

    const T2& get_second() const noexcept
    {
        return second;
    }
};

template <typename T1, typename T2>
class compress_pair<T1, T2, false>
{
public:
    using first_type  = T1;
    using second_type = T2;
private:
    T1 first;
    T2 second;
public:
    constexpr explicit compress_pair() : first(), second() { }

    template <typename U1, typename... U2>
    compress_pair(U1&& f, U2&&... s) 
    : first(tiny_stl::forward<U1>(f)), second(tiny_stl::forward<U2>(s)...) { }


    T1& get_first() noexcept 
    {
        return first;
    }

    const T1& get_first() const noexcept 
    {
        return first;
    }

    T2& get_second() noexcept 
    {
        return second;
    }

    const T2& get_second() const noexcept 
    {
        return second;
    }
};  // class extra::compress_pair<T1, T2>

}   // extra namespace


}   // tiny_stl namespace