#pragma once

#include <initializer_list> // for std::initializer_list

#include "memory.h"

namespace tiny_stl 
{

// iterator e.g.:
//                           first    cur      last
//                            |        |        |
//                            v        v        v
//                            ___________________
//                    /----> |___________________ buffer(related to sizeof(T))
//                   /        ___________________
//                  /  /---> |___________________ buffer
//                 /  /       ___________________
//                /  /  /--> |___________________ buffer
//               /  /  /
//              /  /  /  /-> ...
//            _/__/__/__/___ 
// map_ptr-> |__|__|__|__|__ ...
//            ^
//            |
//           node
//

template <typename T>
struct _Deque_const_iterator 
{
    using iterator_category     = random_access_iterator_tag;
    using value_type            = T;
    using pointer               = const T*; 
    using reference             = const T&;
    using size_type             = size_t;
    using difference_type       = ptrdiff_t;

    using _Map_ptr              = T**;
    using _Self                 = _Deque_const_iterator<T>;

    constexpr static size_type buffer_size() 
    {
        constexpr size_type sz = sizeof(T);
        return sz < 512 ? static_cast<size_type>(512 / sz) 
                        : static_cast<size_type>(1);
    }

    T* cur;             // point to current element 
    T* first;           // point to first element in current buffer
    T* last;            // point to last element in current buffer
    _Map_ptr node;      // point to the control center

    _Deque_const_iterator() = default;
    _Deque_const_iterator(T* c, T* f, T* l, _Map_ptr n)
        : cur(c), first(f), last(l), node(n) { }
    _Deque_const_iterator(const _Self&) = default;

    _Deque_const_iterator(_Deque_const_iterator&& rhs) noexcept
        : cur(rhs.cur), first(rhs.first),
        last(rhs.last), node(rhs.node)  
    { 
        rhs.cur = nullptr;
        rhs.first = nullptr;
        rhs.last = nullptr;
        rhs.node = nullptr;
    }

    _Deque_const_iterator& operator=(const _Deque_const_iterator&) = default;

    void _Set_node(_Map_ptr new_node) 
    {
        node = new_node;
        first = *new_node;
        last = first + static_cast<difference_type>(buffer_size());
    }

    reference operator*() const 
    {
        return *cur;
    }

    pointer operator->() const 
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _Self& operator++() // pre
    {   
        ++cur;
        if (cur == last) 
        {          // cur point to the tail of buffer
            _Set_node(node + 1);    // switch to the next buffer
            cur = first;            // cur point to the first element
        }
        return *this;
    }

    _Self operator++(int) // post
    { 
        _Self tmp;
        ++*this;
        return tmp;
    }

    _Self& operator--() // pre
    { 
        if (cur == first) 
        {
            _Set_node(node - 1);
            cur = last;
        }
        --cur;
        return *this;
    }

    _Self operator--(int) // post
    { 
        _Self tmp;
        --*this;
        return tmp;
    }

    _Self& operator+=(difference_type n) 
    {
        difference_type offset = n + (cur - first);
        if (offset >= 0 && offset < static_cast<difference_type>(buffer_size())) 
        {
            // the target is in the same buffer
            cur += n;
        }
        else 
        {  
            // the target isn't in the same buffer
            difference_type node_offset = offset > 0 
                ? offset / static_cast<difference_type>(buffer_size())
                : -static_cast<difference_type>((-offset - 1) / buffer_size()) - 1;

            _Set_node(node + node_offset);
            cur = first + (offset - node_offset * static_cast<difference_type>(buffer_size()));
        }

        return *this;
    }

    _Self operator+(difference_type n) const 
    {
        _Self tmp = *this;
        tmp += n;

        return tmp;
    }

    _Self& operator-=(difference_type n) 
    {
        return *this += (-n);
    }

    _Self operator-(difference_type n) const 
    {
        _Self tmp = *this;
        tmp -= n;

        return tmp;
    }

    difference_type operator-(const _Self& rhs) const 
    {
        return static_cast<difference_type>(buffer_size()) * (node - rhs.node - 1)
            + (cur - first) + (rhs.last - rhs.cur);
    }

    reference operator[](difference_type n) const 
    {
        return *(*this + n);
    }

    bool operator==(const _Self& rhs) const 
    {
        return cur == rhs.cur;
    }

    bool operator!=(const _Self& rhs) const 
    {
        return !(*this == rhs);
    }

    bool operator<(const _Self& rhs) const 
    {
        return node == rhs.node ? cur < rhs.cur : node < rhs.node;
    }

    bool operator>(const _Self& rhs) const 
    {
        return rhs < *this;
    }

    bool operator<=(const _Self& rhs) const 
    {
        return !(rhs < *this);
    }

    bool operator>=(const _Self& rhs) const 
    {
        return !(*this < rhs);
    }
};  // class _Deque_const_iterator<T>

template <typename T>
inline _Deque_const_iterator<T> operator+(
    typename _Deque_const_iterator<T>::difference_type n,
    _Deque_const_iterator<T> iter) 
{
    return iter += n;
}

template <typename T>
struct _Deque_iterator : _Deque_const_iterator<T> 
{
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;

    using _Map_ptr          = T**;
    using _Base             = _Deque_const_iterator<T>;
    using _Self             = _Deque_iterator<T>;

    using _Base::cur;

    _Deque_iterator() : _Base() { }
    _Deque_iterator(T* c, T* f, T* l, _Map_ptr n)
        : _Base(c, f, l, n) { }
    _Deque_iterator(const _Self&) = default;

    _Deque_iterator(_Deque_iterator&& rhs) noexcept
    : _Base(rhs.cur, rhs.first, rhs.last, rhs.node) 
    {
        rhs.cur = nullptr;
        rhs.first = nullptr;
        rhs.last = nullptr;
        rhs.node = nullptr;
    }

    _Deque_iterator& operator=(const _Deque_iterator&) = default;

    reference operator*() const 
    {
        return *cur;
    }

    pointer operator->() const 
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _Self& operator++()  // pre
    {
        ++*static_cast<_Base*>(this);
        return *this;
    }

    _Self operator++(int)  // post
    {
        _Self tmp;
        ++*this;
        return tmp;
    }

    _Self& operator--() // pre
    { 
        --*static_cast<_Base*>(this);
        return *this;
    }

    _Self operator--(int) // post
    { 
        _Self tmp;
        --*this;
        return tmp;
    }

    _Self& operator+=(difference_type n) 
    {
        *static_cast<_Base*>(this) += n;
        return *this;
    }

    _Self operator+(difference_type n) const 
    {
        _Self tmp = *this;
        tmp += n;
        return tmp;
    }

    _Self& operator-=(difference_type n) 
    {
        return *this += (-n);
    }

    _Self operator-(difference_type n) const 
    {
        _Self tmp = *this;
        tmp -= n;

        return tmp;
    }

    difference_type operator-(const _Self& rhs) const 
    {
        return *static_cast<const _Base*>(this) - rhs;
    }

    reference operator[](difference_type n) const 
    {
        return *(*this + n);
    }
};  // class _Deque_iterator<T>

template <typename T>
inline _Deque_iterator<T> operator+(
    typename _Deque_iterator<T>::difference_type n,
    _Deque_iterator<T> iter) 
{
    return iter += n;
}

template <typename T, typename Alloc>
class _Deque_base 
{
public:
    using value_type             = T;
    using allocator_type         = Alloc;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using iterator               = _Deque_iterator<T>;
    using const_iterator         = _Deque_const_iterator<T>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;
protected:
    using _Map_ptr               = pointer*;
    using _Alptr                 = typename allocator_traits<Alloc>::template rebind_alloc<pointer>;

protected:
    iterator  start;
    iterator  finish;
    _Map_ptr  map_ptr;
    size_type map_size;
    Alloc     alloc;
    _Alptr    alloc_map;

    constexpr static const size_type kBufferSize = iterator::buffer_size();
protected:
    _Map_ptr _Allocate_map(size_type n) 
    {
        _Map_ptr p = nullptr;
        try
        {
            p = alloc_map.allocate(n);
            return p;
        }
        catch (...)
        {
            _Deallocate_map(p, n);
            throw;
        }
    }

    void _Deallocate_map(_Map_ptr m, size_type n) 
    {
        alloc_map.deallocate(m, n);
    }

    T* _Allocate_node() 
    {
        T* node = nullptr;
        try
        {
            node = alloc.allocate(kBufferSize);
            return node;
        }
        catch (...)
        {
            _Deallocate_node(node);
            throw;
        }
    }

    void _Deallocate_node(T* p) 
    {
        alloc.deallocate(p, kBufferSize);
    }

    void _Initializer_map(size_type n)
    {
        size_type num_nodes = n / kBufferSize + 1;

        map_size = tiny_stl::max(static_cast<size_type>(8), num_nodes + 2);            // 8 is min init size

        _Map_ptr nStart = nullptr, nFinish = nullptr;
        try
        {
            map_ptr = _Allocate_map(map_size);

            nStart = map_ptr + (map_size - num_nodes) / 2;          // centre position
            nFinish = nStart + num_nodes;

            _Create_nodes(nStart, nFinish);                         // allocate buffer
        }
        catch (...)
        {
            _Deallocate_map(nStart, nFinish - nStart);
            throw;
        }

        start._Set_node(nStart);                                    // set iterator
        start.cur = start.first;    

        finish._Set_node(nFinish - 1);
        finish.cur = finish.first + n % kBufferSize;
    }

    void _Create_nodes(_Map_ptr nstart, _Map_ptr nfinish) 
    {
        for (_Map_ptr cur = nstart; cur != nfinish; ++cur) 
            *cur = _Allocate_node();
    }

    void _Dealloc_nodes(_Map_ptr nstart, _Map_ptr nfinish) 
    {
        for (_Map_ptr cur = nstart; cur != nfinish; ++cur) 
            if (cur != nullptr)
                _Deallocate_node(*cur);
    }

public:
    _Deque_base(const Alloc& a)             
    : start(), finish(), map_ptr(), map_size(0), alloc(a), alloc_map() 
    { }

    _Deque_base(const Alloc& a, size_type num_elements) 
    : start(), finish(), map_ptr(), map_size(0), alloc(a), alloc_map() 
    { 
        _Initializer_map(num_elements);
    }

    ~_Deque_base() 
    {
        if (map_ptr != nullptr) 
        {
            _Dealloc_nodes(start.node, finish.node + 1);
            _Deallocate_map(map_ptr, map_size);
        }
    }
};  // class _Deque_base<T, Alloc>

template <typename T, typename Alloc = allocator<T>>
class deque : public _Deque_base<T, Alloc> 
{
public:
    static_assert(is_same<T, typename Alloc::value_type>::value,
        "Allocator::value_type is not the same as T");
public:
    using value_type             = T;
    using allocator_type         = Alloc;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using iterator               = _Deque_iterator<T>;
    using const_iterator         = _Deque_const_iterator<T>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;

protected:
    using _Map_ptr               = pointer*;
    using _Alptr                 = typename allocator_traits<Alloc>::template rebind_alloc<pointer>;
    using _Base                  = _Deque_base<T, Alloc>;
    using _Self                  = deque<T, Alloc>;

private:
    using _Base::start;
    using _Base::finish;
    using _Base::map_ptr;
    using _Base::map_size;
    using _Base::alloc;
    using _Base::alloc_map;
    using _Base::_Allocate_node;
    using _Base::kBufferSize;

private:
    void _Fill_initialize(const T& val) 
    {
        for (_Map_ptr node = this->start.node; node != this->finish.node; ++node) 
            uninitialized_fill(*node, *node + kBufferSize, val); // fill complete buffer
        uninitialized_fill(this->finish.first, this->finish.cur, val);
    }

    void _Tidy() 
    {
        destroy(this->start, this->finish);
    }

public:
    deque() : deque(Alloc()) { }                    // (1)  
    explicit deque(const Alloc& _alloc) 
    : _Base(_alloc, 0) { }

    deque(size_type count, const T& val,            // (2)
        const Alloc& _alloc = Alloc()) 
    : _Base(_alloc, count) 
    {
        _Fill_initialize(val);
    }

    explicit deque(size_type count,                 // (3)
        const Alloc& _alloc = Alloc()) 
    : _Base(_alloc, count) 
    {
        _Fill_initialize(T{});
    }

    template <typename InIter,                      // (4)
        typename = enable_if_t<is_iterator<InIter>::value>>
    deque(InIter first, InIter last, const Alloc& _alloc = Alloc()) 
    : _Base(_alloc, last - first) 
    {
        try
        {
            uninitialized_copy(first, last, this->start);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

    deque(const deque& rhs)                         // (5)
    : _Base(allocator_traits<Alloc>::select_on_container_copy_construction(rhs.alloc), 
        rhs.size()) 
    {
        try
        {
            uninitialized_copy(rhs.begin(), rhs.end(), this->start);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

    deque(const deque& rhs, const Alloc& _alloc)    // (5)
    : _Base(_alloc, rhs.size()) 
    {
        try
        {
            uninitialized_copy(rhs.begin(), rhs.end(), this->start);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
        
    }

private:
    void _Assign_rv(deque&& rhs, true_type) 
    {
        this->map_ptr = rhs.map_ptr;
        this->map_size = rhs.map_size;
        this->start = tiny_stl::move(rhs.start);
        this->finish = tiny_stl::move(rhs.finish);
        
        rhs.map_ptr = nullptr;
        rhs.map_size = 0;
        rhs.start = iterator();
        rhs.finish = iterator();
    }

    template <typename Iter>
    void _Construct(Iter first, Iter last) 
    {
        while (first != last) 
            emplace_back(*first);
    }

    void _Assign_rv(deque&& rhs, false_type) 
    {
        if (this->alloc == rhs.alloc) 
            _Assign_rv(tiny_stl::move(rhs), true_type{});
 
        else 
            _Construct(tiny_stl::make_move_iterator(begin()),
                tiny_stl::make_move_iterator(end()));
    }
public:

    deque(deque&& rhs) noexcept                     // (6)
    : _Base(tiny_stl::move(rhs.alloc)) 
    {
        _Assign_rv(tiny_stl::move(rhs), true_type{});
    }

    deque(deque&& rhs, const Alloc& _alloc)         // (7)
    : _Base(_alloc) 
    { 
        _Assign_rv(tiny_stl::move(rhs), false_type{});
    }

    deque(std::initializer_list<T> ilist,           // (8)
        const Alloc& _alloc = Alloc()) 
    : _Base(_alloc, ilist.size()) 
    {
        try
        {
            uninitialized_copy(ilist.begin(), ilist.end(), this->start);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

    ~deque() 
    {
        _Tidy();
    }
    
    deque& operator=(const deque& rhs) 
    {
        assert(this != tiny_stl::addressof(rhs));

        if (this->alloc != rhs.alloc) 
            _Tidy();

#pragma warning(push)   // if constexpr
#pragma warning(disable : 4984)
        if constexpr (allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) 
        {
            this->alloc = rhs.alloc;
            this->alloc_map = rhs.alloc_map;
        }
#pragma warning(pop)

        try
        {
            if (this->size() >= rhs.size()) // too many
            {
                iterator mid = tiny_stl::copy(rhs.begin(), rhs.end(), begin());
                erase(mid, end());
            }
            else // too little
            {
                const_iterator mid = rhs.begin() + this->size();
                tiny_stl::copy(rhs.begin(), mid, begin());
                insert(end(), mid, rhs.end());
            }
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
        
        return *this;
    }

    deque& operator=(deque&& rhs) 
        noexcept(noexcept(allocator_traits<Alloc>::is_always_equal::value)) 
    {
        assert(this != tiny_stl::addressof(rhs));
        _Tidy();
        this->alloc = tiny_stl::move(rhs.alloc);
        this->alloc_map = tiny_stl::move(rhs.alloc_map);
        _Assign_rv(tiny_stl::move(rhs),
            typename allocator_traits<Alloc>::propagate_on_container_move_assignment{});

        return *this;
    }

    deque& operator=(std::initializer_list<T> ilist) 
    {
        assign(ilist.begin(), ilist.end());
        return *this;
    }

    void assign(size_type count, const T& val) 
    {
        clear();
        _Insert_n(begin(), count, val);
    }

    template <typename InIter, 
        typename = enable_if_t<is_iterator<InIter>::value>>
    void assign(InIter first, InIter last) 
    {
        clear();
        for (; first != last; ++first) 
            emplace_back(*first);
    }

    void assign(std::initializer_list<T> ilist) 
    {
        assign(ilist.begin(), ilist.end());
    }

public:
    reference at(size_type pos) 
    {
        assert(pos < this->size());
        return start[static_cast<difference_type>(pos)];
    }

    const_reference at(size_type pos) const 
    {
        assert(pos < this->size());
        return start[static_cast<difference_type>(pos)];
    }

    reference operator[](size_type pos) 
    {
        return start[static_cast<difference_type>(pos)];
    }

    const_reference operator[](size_type pos) const 
    {
        return start[static_cast<difference_type>(pos)];
    }

    reference front() 
    {
        assert(!empty());
        return *start;
    }

    const_reference front() const 
    {
        assert(!empty());
        return *start;
    }

    reference back() 
    {
        assert(!empty());
        return *(finish - 1);
    }

    const_reference back() const 
    {
        assert(!empty());
        return *(finish - 1);
    }

    allocator_type get_allocator() const 
    {
        return this->alloc;
    }

public:
    iterator begin() noexcept 
    {
        return start;
    }

    const_iterator begin() const noexcept 
    {
        return const_iterator(start.cur, start.first,
                            start.last, start.node);
    }

    const_iterator cbegin() const noexcept 
    {
        return begin();
    }

    iterator end() noexcept 
    {
        return finish;
    }

    const_iterator end() const noexcept 
    {
        return const_iterator(finish.cur, finish.first,
                            finish.last, finish.node);
    }

    const_iterator cend() const noexcept 
    {
        return end();
    }

    reverse_iterator rbegin() noexcept 
    {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept 
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept 
    {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept 
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept 
    {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept 
    {
        return const_reverse_iterator(begin());
    }


public:
    bool empty() const noexcept 
    {
        return start == finish;
    }

    size_type size() const noexcept 
    {
        return finish - start;
    }

    size_type max_size() const noexcept 
    {
        return static_cast<size_type>(-1);
    }

    void shrink_to_fit() 
    {
        deque tmp{ tiny_stl::make_move_iterator(begin()),
            tiny_stl::make_move_iterator(end()) };
        swap(tmp);
    }

public:
    void clear() noexcept 
    {
        // Except for the first and last buffers
        for (_Map_ptr p = start.node + 1; p < finish.node; ++p) 
        {
            destroy(*p, *p + kBufferSize);
            this->alloc.deallocate(*p, kBufferSize);
        }

        // There are at least two buffers
        if (start.node != finish.node) 
        {  
            destroy(start.cur, start.last);
            destroy(finish.first, finish.cur);

            // Release the last buffer, reserve the first buffer
            this->alloc.deallocate(finish.first, kBufferSize);
        }
        else  // There is only one buffer   
        {  
            destroy(start.cur, finish.cur);
            // Reserve the buffer, no deallocate
        }

        finish = start;
    }

private:
    // change the map_ptr at the front
    void _Reallocate_map(size_type num_add, true_type) 
    {    
        size_type old_num_nodes = finish.node - start.node + 1;
        size_type new_num_nodes = old_num_nodes + num_add;

        _Map_ptr new_nstart;

        // no reallocate
        if (map_size > 2 * new_num_nodes) 
        {
            new_nstart = map_ptr + (map_size - new_num_nodes) / 2 + num_add;

            // Avoid coverage
            try
            {
                if (new_nstart < start.node)
                    copy(start.node, finish.node + 1, new_nstart);
                else
                    copy_backward(start.node, finish.node + 1,
                        new_nstart + old_num_nodes);
            }
            catch (...)
            {
                _Tidy();
                throw;
            }
        }
        else // reallocate
        {      
            size_type new_map_size = map_size + tiny_stl::max(map_size, num_add) + 2;

            _Map_ptr new_map = nullptr;
            try
            {
                new_map = this->alloc_map.allocate(new_map_size);  // reallocate
                new_nstart = new_map + (new_map_size - new_num_nodes) / 2 + num_add;

                copy(start.node, finish.node + 1, new_nstart);  // copy origin node to new map
            }
            catch (...)
            {
                
                _Tidy();
                throw;
            }

            this->alloc_map.deallocate(map_ptr, map_size);

            map_ptr = new_map;
            map_size = new_map_size;
        }

        // reset the iterators
        start._Set_node(new_nstart);
        finish._Set_node(new_nstart + old_num_nodes - 1);
    }

    // change the map_ptr at the front
    void _Reallocate_map(size_type num_add, false_type) 
    {    
        size_type old_num_nodes = finish.node - start.node + 1;
        size_type new_num_nodes = old_num_nodes + num_add;

        size_type new_map_size = map_size + tiny_stl::max(map_size, num_add) + 2;
        
        _Map_ptr new_map = nullptr, new_nstart = nullptr;
        try
        {
            new_map = this->alloc_map.allocate(new_map_size);  // reallocate
            new_nstart = new_map + (new_map_size - new_num_nodes) / 2;

            copy(start.node, finish.node + 1, new_nstart);  // copy origin node to new map
        }
        catch (...)
        {
            this->alloc_map.deallocate(new_map, new_map_size);
            throw;
        }
        
        this->alloc_map.deallocate(map_ptr, map_size);

        map_ptr = new_map;
        map_size = new_map_size;

        start._Set_node(new_nstart);
        finish._Set_node(new_nstart + old_num_nodes - 1);
    }

    void _Reserv_map_at_front(size_type num_add)    // change the map_ptr
    {  
        if (static_cast<difference_type>(num_add) > start.node - map_ptr) 
            _Reallocate_map(num_add, true_type{});
    }

    void _Reserv_map_at_back(size_type num_add)     // change the map_ptr
    {   
        if (num_add + 1 + (finish.node - map_ptr) > map_size) 
            _Reallocate_map(num_add, false_type{});
    }

    template <typename... Args>
    void _Emplace_front_aux(Args&&... args) 
    {
        _Reserv_map_at_front(1);

        try
        {
            *(start.node - 1) = _Allocate_node();   // allocate a new node

            start._Set_node(start.node - 1);
            start.cur = start.last - 1;
            this->alloc.construct(start.cur, tiny_stl::forward<Args>(args)...);
        }
        catch (...)
        {

        }
    }


    template <typename... Args>
    void _Emplace_back_aux(Args&&... args) 
    {
        _Reserv_map_at_back(1);

        try
        {
            *(finish.node + 1) = _Allocate_node();  // allocate a new node
            this->alloc.construct(finish.cur, tiny_stl::forward<Args>(args)...);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }

        finish._Set_node(finish.node + 1);
        finish.cur = finish.first;
    }


public:
    template <typename... Args>
    void emplace_back(Args&&... args) 
    {
        assert(size() < max_size() - 1);
        try
        {
            if (finish.cur != finish.last - 1)  // There are two or more spaces
            {
                alloc.construct(finish.cur, tiny_stl::forward<Args>(args)...);
                ++finish.cur;
            }
            else    // There is only one space
            {
                _Emplace_back_aux(tiny_stl::forward<Args>(args)...);
            }
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

    void push_back(const T& val) 
    {
        emplace_back(val);
    }


    void push_back(T&& val) 
    {
        emplace_back(tiny_stl::move(val));
    }

    template <typename... Args>
    void emplace_front(Args&&... args) 
    {
        assert(size() < max_size() - 1);
        if (start.cur != start.first) 
        {
            this->alloc.construct(start.cur - 1,
                tiny_stl::forward<Args>(args)...);
            --start.cur;
        }
        else 
            _Emplace_front_aux(tiny_stl::forward<Args>(args)...);
        
    }

    void push_front(const T& val) 
    {
        emplace_front(val);
    }

    void push_front(T&& val) 
    {
        emplace_front(tiny_stl::move(val));
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) 
    {
        iterator iter = _Make_iter(pos);
        assert(iter >= begin() && iter <= end());
        size_type offset = iter - begin();

        if (offset <= size() / 2)   // front
        {     
            emplace_front(tiny_stl::forward<Args>(args)...);
            tiny_stl::rotate(begin(), begin() + 1, begin() + 1 + offset);
        }
        else    // back
        {                              
            emplace_back(tiny_stl::forward<Args>(args)...);
            tiny_stl::rotate(begin() + offset, end() - 1, end());
        }

        return begin() + offset;
    }

    iterator insert(const_iterator pos, const T& val) 
    {
        return emplace(pos, val);
    }

    iterator insert(const_iterator pos, T&& val) 
    {
        return emplace(pos, tiny_stl::move(val));
    }

private:
    void _Insert_n(const_iterator pos, size_type count, const T& val) 
    {
        iterator iter = _Make_iter(pos);
        assert(iter >= begin() && iter <= end());
        size_type offset = iter - begin();
        size_type reoffset = size() - offset;
        size_type oldSize = size();

        if (offset < oldSize / 2)   // front
        { 
            if (offset < count)     // prefix < count 
            {   
                for (size_type i = count - offset; i > 0; --i)
                    push_front(val);                // insert excess val
                for (size_type i = offset; i > 0; --i)
                    push_front(begin()[count - 1]); // insert prefix
                iterator mid = begin() + count;
                tiny_stl::fill(mid, mid + offset, val);
            }
            else                    // prefix >= count
            {                  
                for (size_type i = count; i > 0; --i) 
                    push_front(begin()[count - 1]); // push part of prefix

                iterator mid = begin() + count;
                tiny_stl::move(mid + count, mid + offset, mid);
                tiny_stl::fill(begin() + offset, mid + offset, val);
            }
        }
        else                        // back
        {    
            if (reoffset < count)   // suffix < count 
            { 
                for (size_type i = count - reoffset; i > 0; --i)
                    push_back(val); // insert excess val
                for (size_type i = 0; i < reoffset; ++i)
                    push_back(begin()[offset + i]);
                iterator mid = begin() + offset;
                tiny_stl::fill(mid, mid + reoffset, val);
            }
            else                    // suffix >= count
            {      
                for (size_type i = 0; i < count; ++i)
                    push_back(begin()[oldSize - count + i]);    // push part of suffix
                iterator mid = begin() + offset;
                tiny_stl::move_backward(mid, mid + reoffset - count, mid + reoffset);
                tiny_stl::fill(mid, mid + count, val);
            }
        }
    }

public:
    iterator insert(const_iterator pos, size_type count, const T& val) 
    {
        size_type offset = _Make_iter(pos) - begin();
        _Insert_n(pos, count, val);
        return begin() + offset;
    }

    template <typename InIter,
        typename = enable_if_t<is_iterator<InIter>::value>>
    iterator insert(const_iterator pos, InIter first, InIter last) 
    {
        iterator iter = _Make_iter(pos);
        assert(iter >= begin() && iter <= end());
        size_type offset = iter - begin();
        size_type oldSize = size();

        if (last <= first);
        else if (offset <= oldSize / 2)      // front
        {  
            for (; first != last; ++first)
                push_front(*first);

            size_type num = size() - oldSize;
            tiny_stl::reverse(begin(), begin() + num);
            tiny_stl::rotate(begin(), begin() + num, begin() + num + offset);
        }
        else                                // back
        {  
            for (; first != last; ++first)
                push_back(*first);
            tiny_stl::rotate(begin() + offset, begin() + oldSize, end());
        }

        return begin() + offset;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) 
    {
        return insert(pos, ilist.begin(), ilist.end());
    }

private:
    void _Pop_back_aux() 
    {
        this->_Deallocate_node(finish.first);   // release the last buffer
        finish._Set_node(finish.node - 1);
        finish.cur = finish.last - 1;
        this->alloc.destroy(finish.cur);
    }

    void _Pop_front_aux() 
    {
        this->alloc.destroy(start.cur);
        this->_Deallocate_node(start.first);
        start._Set_node(start.node + 1);
        start.cur = start.first;
    }

public:
    void pop_back() 
    {
        assert(!empty());
        if (finish.cur != finish.first)     // Have one or more elements
        {   
            --finish.cur;
            this->alloc.destroy(finish.cur);
        }
        else 
        {
            _Pop_back_aux();
        }
    }

    void pop_front() 
    {
        assert(!empty());
        if (start.cur != start.last - 1) 
        {
            this->alloc.destroy(start.cur);
            ++start.cur;
        }
        else 
        {
            _Pop_front_aux();
        }

    }

private:
    iterator _Make_iter(const_iterator citer) const 
    {
        return iterator(citer.cur, citer.first, citer.last, citer.node);
    }
public:
    iterator erase(const_iterator pos) 
    {
        assert(_Make_iter(pos) != end());
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last) 
    {
        iterator f = _Make_iter(first);
        iterator l = _Make_iter(last);

        assert(f == l || (f < l && f >= begin() && l <= end()));
        
        if (f == l && last == finish) 
        {
            clear();
            return end();
        }
        else 
        {
            size_type num_erase = l - f;  
            size_type num_before = f - start;

            if (num_before < (size() - num_erase) / 2)  // front
            {  
                move_backward(start, f, l);
                iterator new_start = start + num_erase;
                destroy(start, new_start);

                for (_Map_ptr cur = start.node; cur < new_start.node; ++cur) 
                    this->_Deallocate_node(*cur);
                start = new_start;
            }
            else                                        // back
            {          
                tiny_stl::move(l, finish, f);
                iterator new_finish = finish - num_erase;
                destroy(new_finish, finish);

                for (_Map_ptr cur = new_finish.node + 1; cur <= finish.node; ++cur) 
                    this->_Deallocate_node(*cur);
          
                finish = new_finish;
            }

            return start + num_before;
        }
    }

    void resize(size_type count) 
    {
        while (this->size() < count) // this too little
            emplace_back();
        while (this->size() > count)
            pop_back();
    }

    void resize(size_type count, const T& val) 
    {
        while (this->size() < count) // this too little
            emplace_back(val);
        while (this->size() > count)
            pop_back();
    }


    void swap(deque& rhs) noexcept(noexcept(allocator_traits<Alloc>::is_always_equal::value)) 
    {
        _Swap_alloc(this->alloc, rhs.alloc);
        _Swap_alloc(this->alloc_map, rhs.alloc_map);

        tiny_stl::swap(this->map_ptr, rhs.map_ptr);
        tiny_stl::swap(this->start, rhs.start);
        tiny_stl::swap(this->finish, rhs.finish);
        tiny_stl::swap(this->map_size, rhs.map_size);
    }
    
};  // class deque<T, Alloc>

template <typename T, typename Alloc>
inline bool operator==(const deque<T, Alloc>& lhs,
                       const deque<T, Alloc>& rhs) 
{
    return lhs.size() == rhs.size()
        && tiny_stl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Alloc>
inline bool operator!=(const deque<T, Alloc>& lhs,
                       const deque<T, Alloc>& rhs) 
{
    return !(lhs == rhs);
}

template <typename T, typename Alloc>
inline bool operator<(const deque<T, Alloc>& lhs,
                      const deque<T, Alloc>& rhs) 
{
    return tiny_stl::lexicographical_compare(lhs.begin(), 
                    lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, typename Alloc>
inline bool operator>(const deque<T, Alloc>& lhs,
                      const deque<T, Alloc>& rhs) 
{
    return rhs < lhs;
}

template <typename T, typename Alloc>
inline bool operator<=(const deque<T, Alloc>& lhs,
                       const deque<T, Alloc>& rhs) 
{
    return !(rhs < lhs);
}

template <typename T, typename Alloc>
inline bool operator>=(const deque<T, Alloc>& lhs,
                       const deque<T, Alloc>& rhs) 
{
    return !(lhs < rhs);
}

template <typename T, typename Alloc>
inline void swap(deque<T, Alloc>& lhs, deque<T, Alloc>& rhs)
    noexcept(noexcept(lhs.swap(rhs))) 
{
    lhs.swap(rhs);
}

}   // namespace tiny_stl