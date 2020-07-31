#pragma once

#include "deque.hpp"
#include "vector.hpp"

namespace tiny_stl
{

template <typename T, typename Container = tiny_stl::deque<T>>
class queue
{
public:
    using container_type    = Container;
    using value_type        = typename Container::value_type;
    using size_type         = typename Container::size_type;
    using reference         = typename Container::reference;
    using const_reference   = typename Container::const_reference;

public:
    static_assert(is_same<T, value_type>::value,
        "container_type::value_type error");

private:
    container_type cont;

public:
    explicit queue(const container_type& c)                     // (1)
    : cont(c) { }

    explicit queue(container_type&& c = container_type())       // (2)
    : cont(tiny_stl::move(c)) { }

    queue(const queue& rhs) : cont(rhs.cont) { }                // (3)

    queue(queue&& rhs) : cont(tiny_stl::move(rhs.cont)) { }     // (4)

    template <typename Alloc, typename =                        // (5)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    explicit queue(const Alloc& alloc) : cont(alloc) { }

    template <typename Alloc, typename =                        // (6)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    queue(const container_type& c, const Alloc& alloc) 
    : cont(c, alloc) { }

    template <typename Alloc, typename =                        // (7)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    queue(container_type&& c, const Alloc& alloc) 
    : cont(tiny_stl::move(c), alloc) { }

    // copy construct 
    template <typename Alloc, typename =                        // (8)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    queue(const queue& rhs, const Alloc& alloc) 
    : cont(rhs.cont, alloc) { }

    // move construct
    template <typename Alloc, typename =                        // (9)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    queue(queue&& rhs, const Alloc& alloc) 
    : cont(tiny_stl::move(rhs.cont), alloc) { }

    reference front() 
    {
        return cont.front();
    }

    const_reference front() const 
    {
        return cont.front();
    }

    reference back() 
    {
        return cont.back();
    }

    const_reference back() const
    {
        return cont.back();
    }

    bool empty() const
    {
        return cont.empty();
    }

    size_type size() const 
    {
        return cont.size();
    }

    void push(const value_type& val)
    {
        cont.push_back(val);
    }

    void push(value_type&& val) 
    {
        cont.push_back(tiny_stl::move(val));
    }

    template <typename... Args>
    void emplace(Args&&... args) 
    {
        cont.emplace_back(tiny_stl::forward<Args>(args)...);
    }

    void pop() 
    {
        cont.pop_front();
    }

    void swap(queue& rhs) 
        noexcept(is_nothrow_swappable<Container>::value)
    {
        swapADL(cont, rhs.cont);
    }

    const container_type& getContainer() const 
    {
        return this->cont;
    }
};  // class queue<T, Container>

template <typename T, typename Container>
inline bool operator==(const queue<T, Container>& lhs,
                       const queue<T, Container>& rhs)
{
    return lhs.getContainer() == rhs.getContainer();
}


template <typename T, typename Container>
inline bool operator!=(const queue<T, Container>& lhs,
                       const queue<T, Container>& rhs) 
{
    return !(lhs == rhs);
}


template <typename T, typename Container>
inline bool operator<(const queue<T, Container>& lhs,
                      const queue<T, Container>& rhs) 
{
    return lhs.getContainer() < rhs.getContainer();
}

template <typename T, typename Container>
inline bool operator>(const queue<T, Container>& lhs,
                      const queue<T, Container>& rhs)
{
    return rhs < lhs;
}

template <typename T, typename Container>
inline bool operator<=(const queue<T, Container>& lhs,
                       const queue<T, Container>& rhs) 
{
    return !(rhs < lhs);
}

template <typename T, typename Container>
inline bool operator>=(const queue<T, Container>& lhs,
                       const queue<T, Container>& rhs) 
{
    return !(lhs < rhs);
}

template <typename T, typename Container>
inline void swap(queue<T, Container>& lhs, 
                 queue<T, Container>& rhs)
    noexcept(noexcept(lhs.swap(rhs))) 
{
    lhs.swap(rhs);
}

template <typename T, typename Container, typename Alloc>
struct uses_allocator<queue<T, Container>, Alloc>
    : uses_allocator<Container, Alloc>::type 
{
};

template <typename T, typename Container = tiny_stl::vector<T>, 
    typename Compare = tiny_stl::less<typename Container::value_type>>
class priority_queue 
{
public:
    using container_type    = Container;
    using value_type        = typename Container::value_type;
    using size_type         = typename Container::size_type;
    using reference         = typename Container::reference;
    using const_reference   = typename Container::const_reference;
    using value_compare     = Compare;

public:
    static_assert(is_same<T, value_type>::value,
        "container_type::value_type error");

private:
    value_compare comp;
    container_type cont;

public:
    priority_queue(const value_compare& cmp, const container_type& c)   // (1)
    : comp(cmp), cont(c) 
    { 
        tiny_stl::make_heap(cont.begin(), cont.end(), comp);
    }

    explicit priority_queue(const value_compare& cmp = value_compare{}, // (2)
        container_type&& c = container_type{}) 
    : comp(cmp), cont(tiny_stl::move(c))
    { 
        tiny_stl::make_heap(cont.begin(), cont.end(), comp);
    }

    priority_queue(const priority_queue& rhs)                           // (3)
    : comp(rhs.comp), cont(rhs.cont) { }

    priority_queue(priority_queue&& rhs)                                // (4)
    : comp(tiny_stl::move(rhs.comp)), cont(tiny_stl::move(rhs.cont)) { }

    template <typename Alloc, typename =                                // (5)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    explicit priority_queue(const Alloc& alloc) 
    : comp(), cont(alloc) { }

    template <typename Alloc, typename =                                // (6)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    priority_queue(const value_compare& cmp, const Alloc& alloc) 
    : comp(cmp), cont(alloc) { }

    template <typename Alloc, typename =                                // (7)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    priority_queue(const value_compare& cmp, 
        const container_type& c, const Alloc& alloc) 
    : comp(cmp), cont(c, alloc) 
    {
        tiny_stl::make_heap(cont.begin(), cont.end(), comp);
    }

    template <typename Alloc, typename =                                // (8)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    priority_queue(const value_compare& cmp, 
        container_type&& c, const Alloc& alloc) 
    : comp(cmp), cont(tiny_stl::move(c), alloc) 
    {
        tiny_stl::make_heap(cont.begin(), cont.end(), comp);
    }

    template <typename Alloc, typename =                                // (9)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    priority_queue(const priority_queue& rhs, const Alloc& alloc) 
    : comp(rhs.comp), cont(rhs.cont, alloc) { }

    template <typename Alloc, typename =                                // (10)
        enable_if_t<uses_allocator<container_type, Alloc>::value>>
    priority_queue(priority_queue&& rhs, const Alloc& alloc) 
    : comp(tiny_stl::move(rhs.comp)), 
        cont(tiny_stl::move(rhs.cont), alloc) { }

    template <typename InIter>                                          // (11)
    priority_queue(InIter first, InIter last, 
                const value_compare& cmp, const container_type& c) 
    : comp(cmp), cont(c)
    {
        cont.insert(cont.end(), first, last);
        tiny_stl::make_heap(cont.begin(), cont.end(), comp);
    }

    template <typename InIter>                                          // (12)
    priority_queue(InIter first, InIter last,
        const value_compare& cmp = value_compare{},
        container_type&& c = container_type{}) 
    : comp(cmp), cont(tiny_stl::move(c)) 
    {
        cont.insert(cont.end(), first, last);
        tiny_stl::make_heap(cont.begin(), cont.end(), comp);
    }

    const_reference top() const 
    {
        return cont.front();
    }

    bool empty() const
    {
        return cont.empty();
    }

    size_type size() const 
    {
        return cont.size();
    }

    void push(const value_type& val) 
    {
        cont.push_back(val);
        tiny_stl::push_heap(cont.begin(), cont.end(), comp);
    }

    void push(value_type&& val) 
    {
        cont.push_back(tiny_stl::move(val));
        tiny_stl::push_heap(cont.begin(), cont.end(), comp);
    }

    template <typename... Args>
    void emplace(Args&&... args) 
    {
        cont.emplace_back(tiny_stl::forward<Args>(args)...);
        tiny_stl::push_heap(cont.begin(), cont.end(), comp);
    }

    void pop() 
    {
        tiny_stl::pop_heap(cont.begin(), cont.end(), comp);
        cont.pop_back();
    }

    void swap(priority_queue& rhs)
        noexcept(is_nothrow_swappable<Container>::value
            && is_nothrow_swappable<Compare>::value)
    {
        swapADL(comp, rhs.comp);
        swapADL(cont, rhs.cont);
    }
};  // class priority_queue<T, Container>

template <typename T, typename Container, typename Compare>
void swap(priority_queue<T, Container, Compare>& lhs,
          priority_queue<T, Container, Compare>& rhs) 
    noexcept(noexcept(lhs.swap(rhs))) 
{
    lhs.swap(rhs);
}

template <typename T, typename Container, typename Compare, typename Alloc>
struct uses_allocator<priority_queue<T, Container, Compare>, Alloc>
    : tiny_stl::uses_allocator<Container, Alloc>::type { };

}   // namespace tiny_stl
