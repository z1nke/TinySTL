#pragma once

#include <initializer_list> // for std::initializer_list

#include "memory.h"

namespace tiny_stl 
{

template <typename T>
struct FLNode 
{
    T data;
    FLNode<T>* next;

    FLNode() = default;

    FLNode(T val, FLNode<T>* p) : data(val), next(p) { }
};

template <typename T>
struct _FList_const_iterator 
{
    using iterator_category = forward_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = const T*;
    using reference         = const T&;
    using Ptr               = FLNode<T>*;

    Ptr ptr;

    _FList_const_iterator() = default;
    _FList_const_iterator(Ptr x) : ptr(x) { }
    _FList_const_iterator(const _FList_const_iterator& rhs) : ptr(rhs.ptr) { }

    reference operator*() const 
    {
        return ptr->data;
    }

    pointer operator->() const 
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _FList_const_iterator& operator++() 
    {
        ptr = ptr->next;
        return *this;
    }

    _FList_const_iterator operator++(int) 
    {
        _FList_const_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const _FList_const_iterator& rhs) const 
    {
        return ptr == rhs.ptr;
    }

    bool operator!=(const _FList_const_iterator& rhs) const 
    {
        return ptr != rhs.ptr;
    }
};  // class _FList_const_iterator<T>

template <typename T>
struct _FList_iterator : _FList_const_iterator<T> 
{
    using iterator_category = forward_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;
    using Ptr               = FLNode<T>*;
    using _Base             = _FList_const_iterator<T>;

    _FList_iterator() : _Base() { }
    _FList_iterator(Ptr x) : _Base(x) { }
    _FList_iterator(const _FList_iterator& rhs) : _Base(rhs.ptr) { }

    reference operator*() const 
    {
        return this->ptr->data;
    }

    pointer operator->() const 
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _FList_iterator& operator++() 
    {
        this->ptr = this->ptr->next;
        return *this;
    }

    _FList_iterator operator++(int) 
    {
        _FList_iterator tmp = *this;
        ++*this;
        return tmp;
    }
};  // class _FList_iterator<T>

template <typename T, typename Alloc>
class _FList_base 
{
public:
    using _Node     = FLNode<T>;
    using _Nodeptr  = FLNode<T>*;
    using _Alnode   = typename allocator_traits<Alloc>::template rebind_alloc<_Node>;

protected:
    extra::compress_pair<_Alnode, _Nodeptr> m_pair;


    _Alnode& _Get_Alloc() noexcept 
    {
        return m_pair.get_first();
    }

    const _Alnode& _Get_Alloc() const noexcept 
    {
        return m_pair.get_first();
    }

    _Nodeptr& _Get_head() noexcept 
    {
        return m_pair.get_second();
    }

    const _Nodeptr& _Get_head() const noexcept 
    {
        return m_pair.get_second();
    }
public:
    _FList_base() : m_pair() 
    {
        _Create_head_node();
    }

    template <typename Any_alloc, 
        typename = enable_if_t<!is_same<decay_t<Any_alloc>, _FList_base>::value>>
    _FList_base(Any_alloc&& a) 
    : m_pair(tiny_stl::forward<Any_alloc>(a)) 
    {
        _Create_head_node();
    }


    void _Create_head_node() 
    {
        auto& alloc = _Get_Alloc();
        try
        {
            _Get_head() = alloc.allocate(1);
            alloc.construct(tiny_stl::addressof(_Get_head()->data));
            _Get_head()->next = nullptr;
        }
        catch (...)
        {
            alloc.destroy(tiny_stl::addressof(_Get_head()->data));
            alloc.deallocate(_Get_head(), 1);
            throw;
        }
    }

    template <typename... U>
    _Nodeptr _Create_node(_Nodeptr _next, U&&... val) 
    {
        auto& alloc = _Get_Alloc();
        _Nodeptr p = nullptr;
        try
        {
            p = alloc.allocate(1);
            alloc.construct(tiny_stl::addressof(p->data),
                tiny_stl::forward<U>(val)...);
        }
        catch (...)
        {
            _Free_node(p);
            throw;
        }

        p->next = _next;

        return p;
    }

    void _Free_node(_Nodeptr p) 
    {
        auto& alloc = _Get_Alloc();
        alloc.destroy(tiny_stl::addressof(p->data));
        alloc.deallocate(p, 1);
    }

    ~_FList_base() noexcept 
    {
        auto& alloc = _Get_Alloc();

        alloc.destroy(tiny_stl::addressof(_Get_head()->data));
        alloc.deallocate(_Get_head(), 1);
    }
};

// 
// head(no element)
//   before_begin    begin                  end
//    _______     ________     ________
//   |  data  |   |  data  |   |  data  |
//   |__next__|-->|__next__|-->|__next__|-->null 
// 

template <typename T, typename Alloc = allocator<T>>
class forward_list : public _FList_base<T, Alloc> 
{
public:
    static_assert(is_same<T, typename Alloc::value_type>::value,
        "Alloc::value_type is not the same as T");

public:
    using value_type        = T;
    using allocator_type    = Alloc;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;
    using reference         = T&;
    using const_reference   = const T&;
    using pointer           = typename allocator_traits<Alloc>::pointer;
    using const_pointer     = typename allocator_traits<Alloc>::pointer;
    using iterator          = _FList_iterator<T>;
    using const_iterator    = _FList_const_iterator<T>;

    using _Base             = _FList_base<T, Alloc>;
    using _Self             = forward_list<T, Alloc>;
    using _Node             = FLNode<T>;
    using _Nodeptr          = FLNode<T>*;
    using _Alnode           = typename _Base::_Alnode;
    using _Alnode_traits    = allocator_traits<allocator_type>;

public:
    forward_list() : forward_list(Alloc()) { }                      // (1)
    explicit forward_list(const Alloc& alloc) : _Base(alloc) { }

    forward_list(size_type count, const T& val,                     // (2)
                const Alloc& alloc = Alloc()) : _Base(alloc) 
    {
        insert_after(before_begin(), count, val);
    }

    explicit forward_list(size_type count,                          // (3)
                const Alloc& alloc = Alloc()) : _Base(alloc) 
    {
        T val = T{};
        insert_after(before_begin(), count, val);
    }

    template <typename InIter,                                      // (4)
        typename = enable_if_t<is_iterator<InIter>::value>> 
    forward_list(InIter first, InIter last, const Alloc& alloc = Alloc()) 
    : _Base(alloc) 
    {
        insert_after(before_begin(), first, last);
    }

    forward_list(const forward_list& rhs)                           // (5)
    : _Base(_Alnode_traits::select_on_container_copy_construction(
            rhs.get_allocator())) 
    {
        insert_after(before_begin(), rhs.begin(), rhs.end());
    }

    forward_list(const forward_list& rhs, const Alloc& alloc)       // (5)
    : _Base(alloc) 
    {
        insert_after(before_begin(), rhs.begin(), rhs.end());
    }

private:
    void _Construct_rv(forward_list&& rhs, true_type) 
    {
        _Swap_ADL(this->_Get_head(), rhs._Get_head());
    }

    void _Construct_rv(forward_list&& rhs, false_type) 
    {
        if (this->_Get_Alloc() == rhs._Get_Alloc())
            _Construct_rv(tiny_stl::move(rhs), true_type{});
        else
            insert_after(before_begin(), tiny_stl::make_move_iterator(rhs.begin()),
                tiny_stl::make_move_iterator(rhs.end()));
    }

public:
    forward_list(forward_list&& rhs) noexcept                       // (6)
    : _Base(tiny_stl::move(rhs._Get_Alloc())) 
    {
        _Construct_rv(tiny_stl::move(rhs), true_type{});
    }

    forward_list(forward_list&& rhs, const Alloc& alloc)            // (7)
    : _Base(alloc) 
    {
        _Construct_rv(tiny_stl::move(rhs), false_type{});
    }

    forward_list(std::initializer_list<T> ilist,                    // (8)
                const Alloc& alloc = Alloc()) : _Base(alloc)
    {
        insert_after(before_begin(), ilist.begin(), ilist.end());
    }

    forward_list& operator=(const forward_list& rhs) 
    {
        assert(this != tiny_stl::addressof(rhs));

#pragma warning(push)   // if constexpr
#pragma warning(disable : 4984)
        if constexpr (_Alnode_traits::propagate_on_container_copy_assignment::value) 
            this->_Get_Alloc() = rhs._Get_Alloc();
#pragma warning(pop)

        assign(rhs.begin(), rhs.end());
        return *this;
    }

    forward_list& operator=(forward_list&& rhs)
                noexcept(_Alnode_traits::is_always_equal::value) 
    {
        assert(this != tiny_stl::addressof(rhs));

        clear();

#pragma warning(push)   // if constexpr
#pragma warning(disable : 4984)
        if constexpr (_Alnode_traits::propagate_on_container_move_assignment::value)
            this->_Get_Alloc() = rhs._Get_Alloc();
#pragma warning(pop)

        _Construct_rv(tiny_stl::move(rhs),
            typename _Alnode_traits::propagate_on_container_move_assignment{});

        return *this;
    }

    forward_list& operator=(std::initializer_list<T> ilist) 
    {
        assign(ilist.begin(), ilist.end());
        return *this;
    }

    void assign(size_type count, const T& val) 
    {
        clear();
        insert_after(before_begin(), count, val);
    }

    template <typename InIter, 
        typename = enable_if_t<is_iterator<InIter>::value>>
    void assign(InIter first, InIter last) 
    {
        clear();
        insert_after(before_begin(), first, last);
    }


    void assign(std::initializer_list<T> ilist) 
    {
        assign(ilist.begin(), ilist.end());
    }


    allocator_type get_allocator() const 
    {
        return this->_Get_Alloc();
    }

    void clear() noexcept 
    {
        _Nodeptr pNext;
        _Nodeptr p = this->_Get_head()->next;
        this->_Get_head()->next = nullptr;

        for (; p != nullptr; p = pNext) 
        {
            pNext = p->next;
            this->_Free_node(p);
        }
    }

    reference front() 
    {
        return *begin();
    }

    const_reference front() const 
    {
        return *begin();
    }

    iterator before_begin() noexcept 
    {
        return iterator(this->_Get_head());
    }

    const_iterator before_begin() const noexcept 
    {
        return const_iterator(this->_Get_head());
    }

    const_iterator cbefore_begin() const noexcept 
    {
        return before_begin();
    }

    iterator begin() noexcept 
    {
        return iterator(this->_Get_head()->next);
    }

    const_iterator begin() const noexcept 
    {
        return const_iterator(this->_Get_head()->next);
    }

    const_iterator cbegin() const noexcept 
    {
        return begin();
    }

    iterator end() noexcept 
    {
        return iterator();
    }

    const_iterator end() const noexcept 
    {
        return const_iterator();
    }

    const_iterator cend() const noexcept 
    {
        return end();
    }

    bool empty() const noexcept 
    {
        return begin() == end();
    }

    size_type max_size() const noexcept 
    {
        return allocator_traits<_Alnode>::max_size(this->_Get_Alloc());
    }


    ~forward_list() noexcept 
    {
        clear();
    }

private:
    size_type _Size() const 
    {
        size_type n = 0;
        for (const_iterator p = begin(); p != end(); ++p, ++n);
        return n;
    }

    const_iterator _Before_end() const 
    {
        const_iterator p = before_begin();
        for (const_iterator _next = p; ++_next != end(); p = _next);
        return p;
    }

public:
    iterator _Make_iter(const_iterator iter) const
    {
        return iterator(iter.ptr);
    }

    template <typename... Args>
    iterator emplace_after(const_iterator pos, Args&&... args) 
    {
        _Nodeptr p = pos.ptr;
        _Nodeptr newNode = this->_Create_node(pos.ptr->next, 
                            tiny_stl::forward<Args>(args)...);
        p->next = newNode;

        return _Make_iter(++pos);
    }

    iterator insert_after(const_iterator pos, const T& val) 
    {
        return emplace_after(pos, val);
    }

    iterator insert_after(const_iterator pos, T&& val) 
    {
        return emplace_after(pos, tiny_stl::move(val));
    }

    iterator insert_after(const_iterator pos, size_type count, const T& val) 
    {
        while (count--) 
            pos = emplace_after(pos, val);
        return _Make_iter(pos);
    }

    template <typename InIter,
        typename = enable_if_t<is_iterator<InIter>::value>>
    iterator insert_after(const_iterator pos, InIter first, InIter last) 
    {
        for (; first != last; ++first, ++pos) 
            emplace_after(pos, *first);
        return _Make_iter(pos);
    }

    iterator insert_after(const_iterator pos, std::initializer_list<T> ilist) 
    {
        return insert_after(pos, ilist.begin(), ilist.end());
    }

    void push_front(const T& val) 
    {
        emplace_after(before_begin(), val);
    }

    void push_front(T&& val) 
    {
        emplace_after(before_begin(), tiny_stl::move(val));
    }

    iterator erase_after(const_iterator pos) 
    {
        _Nodeptr cur = pos.ptr;

        assert(pos.ptr->next != nullptr);

        ++pos;
        _Nodeptr pFree = pos.ptr;
        cur->next = pFree->next;
        ++pos;                      // for return value
        this->_Free_node(pFree);

        return _Make_iter(pos);
    }

    // erase the elements in the range (first, last)
    iterator erase_after(const_iterator first, const_iterator last) 
    {
        if (first == before_begin() && last == end()) 
        {
            clear();
            return end();
        }
        else 
        {
            assert(first != end());
            if (first != last) 
            {
                const_iterator _next = first;
                for (++_next; _next != last; ) 
                    _next = erase_after(first);
            }
            return _Make_iter(last);
        }
    }

    void pop_front() 
    {
        erase_after(before_begin());
    }

    void resize(size_type count, const T& val) 
    {
        size_type curSize = this->_Size();
        if (curSize < count)
            insert_after(this->_Before_end(), count - curSize, val);
        else if (curSize > count) 
        {
            const_iterator tail = before_begin();
            while (count--)
                ++tail;
            erase_after(tail, end());
        }
    }

    void resize(size_type count) 
    {
        T val = T{};
        resize(count, val);
    }

    void swap(forward_list& rhs) noexcept(allocator_traits<Alloc>::is_always_equal::value) 
    {
        if (this != tiny_stl::addressof(rhs)) 
        {
            _Swap_alloc(this->_Get_Alloc(), rhs._Get_Alloc());
            _Swap_ADL(this->_Get_head(), rhs._Get_head());
        }
    }

private:
    void _Splice_after(const_iterator pos, forward_list& rhs,
                       const_iterator first, const_iterator last) 
    {
        assert(this->get_allocator() == rhs.get_allocator());
        const_iterator rhsBeforeEnd = first;
        const_iterator rhsEnd = rhsBeforeEnd;

        // Find the element before last iterator
        for (++rhsEnd; rhsEnd != last; ++rhsBeforeEnd, ++rhsEnd) 
        {
            if (rhsEnd == rhs.end()) 
            {
                assert(true);
                return;
            }
        }

        rhsBeforeEnd.ptr->next = pos.ptr->next;
        pos.ptr->next = first.ptr->next;
        first.ptr->next = last.ptr;
    }

    template <typename Cmp>
    void _Merge(forward_list& rhs, const Cmp& cmp) 
    {
        if (this != tiny_stl::addressof(rhs)) 
        {       
            const_iterator first1 = this->before_begin(), 
                        next1 = this->begin(), last1 = this->end();   // no prev pointer
            const_iterator first2 = rhs.before_begin(), 
                        next2 = rhs.begin(), last2 = rhs.end();

            assert(tiny_stl::is_sorted(next1, last1));
            assert(tiny_stl::is_sorted(next2, last2));

            for (; next1 != last1 && next2 != last2; ++first1) 
            {
                if (cmp(*next2, *next1))     // move *first2 to the front of *first
                    _Splice_after(first1, rhs, first2, ++next2);
                else 
                    ++next1;
            }

            if (next2 != last2)
                _Splice_after(first1, rhs, first2, last2);
        }
    }

    
    // merge sort
    template <typename BinPred>
    void _Sort(iterator before_first, iterator last, 
               BinPred pred, size_type size) 
    {                         
        if (size < 2)
            return;

        size_type mid_size = size >> 1;

        iterator mid = tiny_stl::next(before_first, 1 + mid_size);      // init mid
        _Sort(before_first, mid, pred, mid_size);                       // sort first half

        iterator before_mid = tiny_stl::next(before_first, mid_size);   // init before_mid
        _Sort(before_mid, last, pred, size - mid_size);                 // sort second half

        // Because the iterator mid does not fail
        // the previous sort will change the mid
        // therefore, update the itertor mid
        mid = tiny_stl::next(before_mid);   
        iterator first = tiny_stl::next(before_first);

        for (;;)    // sort [first, mid) and [mid, last)
        {  
            if (pred(*mid, *first))  // *mid < *first
            {      
                splice_after(before_first, *this, before_mid);
                ++before_first;
                mid = tiny_stl::next(before_mid);

                if (mid == last)
                    return;
            }
            else  // *mid >= *first
            { 
                ++before_first;
                ++first;

                if (first == mid)
                    return;
            }
        }
    }
public:
    void merge(forward_list& rhs) 
    {
        _Merge(rhs, tiny_stl::less<>{});
    }

    void merge(forward_list&& rhs) 
    {
        _Merge(rhs, tiny_stl::less<>{});
    }

    template <typename Cmp>
    void merge(forward_list& rhs, Cmp cmp) 
    {
        _Merge(rhs, cmp);
    }

    template <typename Cmp>
    void merge(forward_list&& rhs, Cmp cmp) 
    {
        _Merge(rhs, cmp);
    }

    // move all elements from rhs into after the pos
    void splice_after(const_iterator pos, forward_list& rhs) 
    {
        assert(this != tiny_stl::addressof(rhs));
        _Splice_after(pos, rhs, rhs.before_begin(), rhs.end());
    }

    void splice_after(const_iterator pos, forward_list&& rhs) 
    {
        splice_after(pos, rhs);
    }

    // move (first, first + 2) into after the pos
    void splice_after(const_iterator pos, forward_list& rhs, 
                      const_iterator first) 
    {
        const_iterator _next = first;
        ++_next;
        if (pos == first || pos == _next)
            return;

        _Splice_after(pos, rhs, first, ++_next);
    }

    void splice_after(const_iterator pos, forward_list&& rhs,
                      const_iterator first) 
    {
        splice_after(pos, rhs, first);
    }

    // move (first, last) into after the pos
    void splice_after(const_iterator pos, forward_list& rhs, 
                      const_iterator first, const_iterator last) 
    {
        // pos can't be an element of (first, last)
#ifdef _DEBUG
        const_iterator _next = first;
        for (++_next; _next != last; ++_next) 
            assert(pos != _next);
#endif // _DEBUG

        _Splice_after(pos, rhs, first, last);
    }

    void splice_after(const_iterator pos, forward_list&& rhs,
                      const_iterator first, const_iterator last) 
    {
        splice_after(pos, rhs, first, last);
    }

    template <typename UnaryPred>
    void remove_if(UnaryPred pred) 
    {
        iterator before = before_begin();

        for (iterator first = begin(); first != end(); ) 
        {
            if (pred(*first))
                first = erase_after(before);
            else 
            {
                ++before;
                ++first;
            }
        }
    }

    void remove(const T& val) 
    {
        remove_if([&val](const T& elem) { return val == elem; });
    }

    void reverse() noexcept 
    {
        if (!empty()) 
        {
            const_iterator beforeEnd = _Before_end();
            const_iterator first = begin();
            const_iterator _next = first;
            for (; first != beforeEnd; first = _next) 
            {
                ++_next;
                this->_Get_head()->next = _next.ptr;
                first.ptr->next = beforeEnd.ptr->next;
                beforeEnd.ptr->next = first.ptr;
            }
        }
    }

    template <typename BinPred>
    void unique(BinPred pred) 
    {
        if (empty())
            return;
        const_iterator first = begin();
        const_iterator _next = first;
        
        for (++_next; _next != end();) 
        {
            if (pred(*first, *_next)) 
                _next = erase_after(first);
            else 
                first = _next++;
        }
    }

    void unique() 
    {
        unique(tiny_stl::equal_to<>{});
    }

    template <typename Cmp>
    void sort(Cmp cmp) 
    {
        _Sort(before_begin(), end(), cmp, this->_Size());
    }

    void sort() 
    {
        sort(tiny_stl::less<>{});
    }
};  // class forward_list<T, Alloc>


template <typename T, typename Alloc>
inline bool operator==(const forward_list<T, Alloc>& lhs, 
                       const forward_list<T, Alloc>& rhs) 
{
    return tiny_stl::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, typename Alloc>
inline bool operator!=(const forward_list<T, Alloc>& lhs, 
                       const forward_list<T, Alloc>& rhs) 
{
    return (!(lhs == rhs));
}

template <typename T, typename Alloc>
inline bool operator<(const forward_list<T, Alloc>& lhs, 
                      const forward_list<T, Alloc>& rhs) 
{
    return tiny_stl::lexicographical_compare(lhs.begin(), lhs.end(),
                                            rhs.begin(), rhs.end());
}

template <typename T, typename Alloc>
inline bool operator<=(const forward_list<T, Alloc>& lhs, 
                       const forward_list<T, Alloc>& rhs) 
{
    return (!(rhs < lhs));
}

template <typename T, typename Alloc>
inline bool operator>(const forward_list<T, Alloc>& lhs, 
                      const forward_list<T, Alloc>& rhs) 
{
    return rhs < lhs;
}

template <typename T, typename Alloc>
inline bool operator>=(const forward_list<T, Alloc>& lhs, 
                       const forward_list<T, Alloc>& rhs) 
{
    return (!(lhs < rhs));
}

template <typename T, typename Alloc>
void swap(forward_list<T, Alloc>& lhs, forward_list<T, Alloc>& rhs)
    noexcept(noexcept(lhs.swap(rhs))) 
{
    lhs.swap(rhs);
}


}   // namespace tiny_stl 