#pragma once

#include <initializer_list> // for std::initializer_list

#include "memory.h"

namespace tiny_stl 
{

template <typename T>
struct LNode 
{
    T data;

    LNode<T>* prev;
    LNode<T>* next;
};

template <typename T>
struct _List_const_iterator 
{
    using iterator_category = bidirectional_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = const T*;
    using reference         = const T&;
    using Ptr               = LNode<T>*;

    Ptr ptr;

    _List_const_iterator() : ptr(nullptr) { }
    _List_const_iterator(Ptr x) : ptr(x) { }
    _List_const_iterator(const _List_const_iterator& rhs) : ptr(rhs.ptr) { }


    reference operator*() const 
    {
        return ptr->data;
    }

    pointer operator->() const 
    {    
        // &ptr->data, avoid overloading operator&
        return pointer_traits<pointer>::pointer_to(**this); 
    }

    _List_const_iterator& operator++() 
    { 
        ptr = ptr->next;
        return *this;
    }

    _List_const_iterator& operator--() 
    {   
        ptr = ptr->prev;
        return *this;
    }

    _List_const_iterator& operator++(int) 
    { 
        _List_const_iterator tmp = *this;
        ptr = ptr->next;
        return tmp;
    }

    _List_const_iterator& operator--(int) 
    { 
        _List_const_iterator tmp = *this;
        ptr = ptr->prev;
        return tmp;
    }

    bool operator==(const _List_const_iterator& rhs) const 
    {
        return ptr == rhs.ptr;
    }

    bool operator!=(const _List_const_iterator& rhs) const 
    {
        return ptr != rhs.ptr;
    }
};  // class _List_const_iterator<T>


template <typename T>
struct _List_iterator : _List_const_iterator<T> 
{
    using iterator_category = bidirectional_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;
    using Ptr               = LNode<T>*;
    using _Base             = _List_const_iterator<T>;

    _List_iterator() : _Base() {}
    _List_iterator(Ptr x) : _Base(x) { }
    _List_iterator(const _List_iterator& rhs) : _Base(rhs.ptr) { }

    reference operator*() const 
    {
        return this->ptr->data;
    }

    pointer operator->() const 
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _List_iterator& operator++()
    {    
        this->ptr = this->ptr->next;
        return *this;
    }

    _List_iterator& operator--() 
    {  
        this->ptr = this->ptr->prev;
        return *this;
    }

    _List_iterator& operator++(int) 
    { 
        _List_const_iterator tmp = *this;
        ptr = ptr->next;
        return tmp;
    }

    _List_iterator& operator--(int) 
    { 
        _List_const_iterator tmp = *this;
        ptr = ptr->prev;
        return tmp;
    }
};  // class _List_iterator


template <typename T, typename Alloc> 
class _List_base 
{ 
public:
    using value_type             = T;
    using allocator_type         = Alloc;   // In face, will be not be used
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using reference              = T&;
    using const_reference        = const T&;
    using pointer                = typename allocator_traits<Alloc>::pointer;
    using const_pointer          = typename allocator_traits<Alloc>::const_pointer;
    using iterator               = _List_iterator<T>;
    using const_iterator         = _List_const_iterator<T>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;

    using _Node                  = LNode<T>;
    using _Nodeptr               = LNode<T>*;
    using _Alnode = typename allocator_traits<Alloc>::template rebind_alloc<_Node>;
protected:
    _Nodeptr   head;
    size_type  count;
    _Alnode    alloc;

    _List_base() : count(0) 
    {
        _Construct_head_node();
    }

    template <typename Any_alloc, typename = 
        enable_if_t<!is_same<decay_t<Any_alloc>, _List_base>::value>>
    _List_base(Any_alloc anyAlloc) : count(0), alloc(anyAlloc) 
    {
        _Construct_head_node();
    }

    void _Construct_head_node() 
    {
        try
        {
            head = alloc.allocate(1);
            head->prev = head;
            head->next = head;
        }
        catch (...)
        {
            _Free_head_node();
            throw;
        }
    }

    void _Free_head_node() 
    {
        alloc.deallocate(head, 1);
    }

    ~_List_base() 
    {
        _Free_head_node();
    }
};  // class _List_base


// cycle linked list
//   _______________________________________________
//  |                                               |
//  |   head(end)    begin                          |
//  |    ______     ______     ______     ______    |
//  |   | null |   | data |   | data |   | data |   |
//  ----| prev |<--| prev |<--| prev |<--| prev |<---
//  --->|_next_|-->|_next_|-->|_next_|-->|_next_|----
//  |                                               |
//  |_______________________________________________|
//    

template <typename T, typename Alloc = allocator<T>>
class list : public _List_base<T, Alloc> 
{
public:
    using value_type             = T;
    using allocator_type         = Alloc;   // In face, will be not be used
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using reference              = T&;
    using const_reference        = const T&;
    using pointer                = typename allocator_traits<Alloc>::pointer;
    using const_pointer          = typename allocator_traits<Alloc>::const_pointer;
    using iterator               = _List_iterator<T>;
    using const_iterator         = _List_const_iterator<T>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;

    using _Base                  = _List_base<T, Alloc>;
    using _Node                  = LNode<T>;
    using _Nodeptr               = LNode<T>*;
    using _Alnode                = typename allocator_traits<Alloc>::template rebind_alloc<_Node>;
    using _Alnode_traits         = allocator_traits<_Alnode>;
private:

    template <typename... Args>
    _Nodeptr _Alloc_construct(Args&&... args) 
    {
        assert(this->count < max_size() - 1);

        _Nodeptr p = nullptr;

        try
        {
            p = this->alloc.allocate(1);
            _Alnode_traits::construct(this->alloc,
                tiny_stl::addressof(p->data),
                tiny_stl::forward<Args>(args)...);
        }
        catch (...)
        {
            this->alloc.deallocate(p, 1);
            throw;
        }

        return p;
    }

    void _Destroy_free(_Nodeptr p) 
    {
        this->alloc.destroy(tiny_stl::addressof(p->data));
        this->alloc.deallocate(p, 1);
    }

    void _Construct_n(size_type n, const T& val)
    {
        try
        {
            _Insert_n(begin(), n, val);
        }
        catch (...)
        {
            clear();
            throw;
        }
    }
    
    template <typename InIter>
    void _Construct_range(InIter first, InIter last)
    {
        try
        {
            insert(begin(), first, last);
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    void _Move_construct(list&& rhs, true_type)
    {
        tiny_stl::swap(this->head, rhs.head);
        tiny_stl::swap(this->count, rhs.count);
    }

    void _Move_construct(list&& rhs, false_type) 
    {
        if (this->alloc == rhs.alloc)
            _Move_construct(tiny_stl::move(rhs), true_type{});
        else
            _Construct_range(tiny_stl::make_move_iterator(rhs.begin()),
                tiny_stl::make_move_iterator(rhs.end()));
    }

public:
    list() : list(Alloc()) { }                                  // (1)

    explicit list(const Alloc& _alloc)                          // (1)
    : _Base(_alloc) { }

    list(size_type n, const T& val,                             // (2)
        const Alloc& _alloc = Alloc()) : _Base(_alloc) 
    {
        _Construct_n(n, val);
    }

    explicit list(size_type n, const Alloc& _alloc = Alloc())   // (3)
    : _Base(_alloc)
    {
        resize(n);
    }

    template <typename InIter, typename = enable_if_t<is_iterator<InIter>::value>>
    list(InIter first, InIter last,                             // (4)
        const Alloc& _alloc = Alloc()) 
    : _Base(_alloc)
    {
        _Construct_range(first, last);
    }


    list(const list& rhs)                                       // (5)
    : _Base(_Alnode_traits::select_on_container_copy_construction(rhs.alloc))
    {
        try
        {
            insert(begin(), rhs.begin(), rhs.end());
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    list(const list& rhs, const Alloc& _alloc)                  // (5)
    : _Base(_alloc)
    {
        try
        {
            insert(begin(), rhs.begin(), rhs.end());
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    list(list&& rhs) : _Base(tiny_stl::move(rhs.alloc))          // (6)
    {      
        _Move_construct(tiny_stl::move(rhs), true_type{});
    }

    list(list&& rhs, const Alloc& _alloc) : _Base(_alloc)       // (7)
    {     
        _Move_construct(tiny_stl::move(rhs), false_type{});
    }

    list(std::initializer_list<T> ilist,                        // (8) 
        const Alloc& _alloc = Alloc()) : _Base(_alloc) 
    {
        try
        {
            insert(begin(), ilist.begin(), ilist.end());
        }
        catch (...)
        {
            clear();
            throw;
        }
    }

    template <typename Arg>
    void _Reuse_node(iterator pos, Arg&& arg) 
    {
        _Alnode_traits::destroy(this->alloc, tiny_stl::addressof(pos.ptr->data));

        try
        {
            _Alnode_traits::construct(this->alloc,
                tiny_stl::addressof(pos.ptr->data),
                tiny_stl::forward<Arg>(arg));
        }
        catch (...)
        {
            clear();    // FIXME, only unlink the node
            throw;
        }
    }

    void assign(size_type n, const T& val) 
    {
        iterator old = begin();
        size_type i;

        try
        {
            for (i = 0; i < n && old != end(); ++i, ++old)
                _Reuse_node(old, val);   // no alloc, reuse nodes
            for (; i < n; ++i)
                _Insert(end(), val);  // last - first > count, alloc
        }
        catch (...)
        {
            clear();
            throw;
        }
       
        erase(old, end());  // last - first < count, erase
    }

    template <typename Iter, 
        typename = enable_if_t<is_iterator<Iter>::value>>
    void assign(Iter first, Iter last) 
    {
        iterator old = begin();

        try
        {
            for (; first != last && old != end(); ++first, ++old)
                _Reuse_node(old, *first);   // no alloc, reuse nodes
            for (; first != last; ++first)
                _Insert(end(), *first);     // last - first > count, alloc
        }
        catch (...)
        {
            clear();
            throw;
        }

        erase(old, end());  // last - first < count, erase
    }

    void assign(std::initializer_list<T> ilist) 
    {
        assign(ilist.begin(), ilist.end());
    }


    list& operator=(const list& rhs) 
    {
        assert(this != tiny_stl::addressof(rhs));
        if (this->alloc != rhs.alloc) 
            clear();
#pragma warning(push)
#pragma warning(disable : 4984) // if constexpr
        if constexpr (_Alnode_traits::propagate_on_container_copy_assignment::value) 
            this->alloc = rhs.alloc;
#pragma warning(pop)
        assign(rhs.begin(), rhs.end());
        
        return *this;
    }

    list& operator=(list&& rhs) {
        assert(this != tiny_stl::addressof(rhs));
        clear();
#pragma warning(push)
#pragma warning(disable : 4984) // if constexpr
        if constexpr (_Alnode_traits::propagate_on_container_move_assignment::value) 
            this->alloc = rhs.alloc;
#pragma warning(pop)
        
        _Move_construct(tiny_stl::move(rhs), 
            typename _Alnode_traits::propagate_on_container_move_assignment{});
        return *this;
    }

    list& operator=(std::initializer_list<T> ilist) 
    {
        assign(ilist.begin(), ilist.end());
        return *this;
    }


    allocator_type get_allocator() const 
    {
        return this->alloc;
    }

    ~list() noexcept 
    {
        clear();
    }

public:
    T& front()
    {
        assert(!this->empty());
        return *begin();
    }

    const T& front() const 
    {
        assert(!this->empty());
        return *begin();
    }

    T& back()
    {
        assert(!this->empty());
        return *(--end());
    }

    const T& back() const
    {
        assert(!this->empty());
        return *(--end());
    }


    iterator begin() noexcept 
    {
        return iterator(this->head->next);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(this->head->next);
    }

    const_iterator cbegin() const noexcept 
    {
        return begin();
    }

    iterator end() noexcept 
    {
        return iterator(this->head);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(this->head);
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
        return rbegin();
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

    bool empty() const noexcept 
    {
        return this->head->next == this->head;
    }

    size_type size() const noexcept 
    {
        return this->count;
    }

    size_type max_size() const noexcept 
    {
        return this->alloc.max_size();
    }

    void clear() noexcept 
    {
        _Nodeptr p = this->head->next;
        this->head->next = this->head;
        this->head->prev = this->head;
        this->count = 0;

        for (_Nodeptr pNext = p->next; p != this->head; p = pNext, pNext = p->next)
            _Destroy_free(p);
    }


private:
    void _IncreCount(size_type n) 
    {
#ifdef _DEBUG
        if (this->count > max_size() - n - 1)
            _Xlength();
#endif // _DEBUG
        this->count += n;
    }

    template <typename... Args>
    void _Insert(const_iterator pos, Args&&... args) 
    {
        _Nodeptr newNode = _Alloc_construct(tiny_stl::forward<Args>(args)...);
        const _Nodeptr nextNode = pos.ptr;
        const _Nodeptr prevNode = nextNode->prev;
        newNode->prev = prevNode;
        newNode->next = nextNode;
        _IncreCount(1);
        nextNode->prev = newNode;
        prevNode->next = newNode;
    }

    void _Insert_n(const_iterator pos, size_type n, const T& val)
    {
        size_type origin_n = n;     // for exception

        for (; n > 0; --n)
            _Insert(pos, val);
    }

    template <typename InIter>
    void _Insert_range(const_iterator pos, InIter first, InIter last) 
    {
        for (; first != last; ++first)
            _Insert(pos, *first);
    }


    iterator _Make_iter(const_iterator pos) const
    {
        return iterator(pos.ptr);
    }
public:
    iterator insert(const_iterator pos, const T& val)           // (1)
    {         
        _Insert(pos, val);
        
        return _Make_iter(--pos);
    }

    iterator insert(const_iterator pos, T&& val)                // (2)
    {              
        return emplace(pos, tiny_stl::move(val));
    }

    iterator insert(const_iterator pos, size_type n,            // (3)
        const T& val) 
    {

        iterator p = _Make_iter(pos);   // for return value
        if (p == begin())
        {
            _Insert_n(pos, n, val);
            return begin();
        }
        else 
        {
            --p;
            _Insert_n(pos, n, val);
            return ++p;
        }
    }

    template <typename InIter,                                  // (4)
        typename = enable_if_t<is_iterator<InIter>::value>>
    iterator insert(const_iterator pos, InIter first, InIter last) 
    {

        iterator p = _Make_iter(pos);   
        
        if (p == begin()) 
        {
            _Insert_range(pos, first, last);
            return begin();
        }
        else 
        {
            --p;
            _Insert_range(pos, first, last);
            return ++p;
        }
    }

    iterator insert(const_iterator pos,                         // (5)
        std::initializer_list<T> ilist) 
    {
        return insert(pos, ilist.begin(), ilist.end());
    }


    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        _Insert(pos, tiny_stl::forward<Args>(args)...);

        return _Make_iter(--pos);
    }

    iterator erase(const_iterator pos) 
    {
        iterator ret(pos.ptr->next);
        pos.ptr->next->prev = pos.ptr->prev;
        pos.ptr->prev->next = pos.ptr->next;
        _Destroy_free(pos.ptr);
        --this->count;
        return ret;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        if (first == begin() && last == end())
        {
            clear();
            return end();
        }
        
        while (first != last) 
        {
            first = erase(first);
        }
        return _Make_iter(last);
    }

    template <typename... Args>
    void emplace_back(Args&&... args) 
    {
        _Insert(end(), tiny_stl::forward<Args>(args)...);
    }

    template <typename... Args>
    void emplace_front(Args&&... args) 
    {
        _Insert(begin(), tiny_stl::forward<Args>(args)...);
    }

    void push_back(const T& val) 
    {
        _Insert(end(), val);
    }

    void push_back(T&& val) 
    {
        _Insert(end(), tiny_stl::move(val));
    }

    void push_front(const T& val) 
    {
        _Insert(begin(), val);
    }

    void push_front(T&& val) 
    {
        _Insert(end(), tiny_stl::move(val));
    }

    void pop_back() 
    {
        erase(--end());
    }

    void pop_front() 
    {
        erase(begin());
    }

    void resize(size_type newSize) 
    {
        // add the nodes at the tail
        if (this->count < newSize) 
        {    
            while (this->count < newSize)
                _Insert(end());
        }
        else // remove the nodes at the tail
        {      
            while (this->count > newSize)
                pop_back();
        }
    }

    void resize(size_type newSize, const T& val) 
    {
        // add the nodes at the tail
        if (this->count < newSize) 
        {    
            _Insert_n(end(), newSize - this->count, val);
        }
        else   // remove the nodes at the tail
        {    
            while (this->count > newSize)
                pop_back();
        }
    }

    void swap(list& rhs) noexcept(noexcept(
        allocator_traits<allocator_type>::propagate_on_container_swap::value
        || allocator_traits<allocator_type>::is_always_equal::value)) 
    {
        _Swap_alloc(this->alloc, rhs.alloc);
        _Swap_ADL(this->head, rhs.head);
        tiny_stl::swap(this->count, rhs.count);
    }

private:
    // move [first, last) to the front of pos
    void transfer(const_iterator pos, const_iterator first, const_iterator last) 
    {
        if (pos != last) 
        {
            iterator prev = pos.ptr->prev;
            prev.ptr->next = first.ptr;
            first.ptr->prev->next = last.ptr;
            last.ptr->prev->next = pos.ptr;
            pos.ptr->prev = last.ptr->prev;
            last.ptr->prev = first.ptr->prev;
            first.ptr->prev = prev.ptr;
        }
    }


    template <typename Cmp>
    void _Merge(list& rhs, const Cmp& cmp) 
    {
        if (this != tiny_stl::addressof(rhs)) 
        {
            const_iterator first1 = this->begin(), last1 = this->end();
            const_iterator first2 = rhs.begin(), last2 = rhs.end();

            assert(this->alloc == rhs.alloc);
            assert(tiny_stl::is_sorted(this->begin(), this->end()));
            assert(tiny_stl::is_sorted(rhs.begin(), rhs.end()));

            while (first1 != last1 && first2 != last2) 
            {
                if (cmp(*first2, *first1))          // *first2 cmp *first1
                {        
                    const_iterator next = first2;   // move *first2 to the front of first1 
                    ++next;
                    this->transfer(first1, first2, next);
                    first2 = next;
                }
                else 
                {
                    ++first1;
                }
            }
            
            if (first2 != last2)
                this->transfer(last1, first2, last2);
        }

        this->count += rhs.count;
        rhs.count = 0;
    }

public:
    void merge(list& rhs) 
    {
        _Merge(rhs, tiny_stl::less<>{});
    }

    void merge(list&& rhs) 
    {
        _Merge(rhs, tiny_stl::less<>{});
    }

    template <typename Cmp>
    void merge(list& rhs, Cmp cmp) 
    {
        _Merge(rhs, cmp);
    }

    template <typename Cmp>
    void merge(list&& rhs, Cmp cmp) 
    {
        _Merge(rhs, cmp);
    }


    void splice(const_iterator pos, list& rhs) 
    {  
        // move all elements from other into *this
        assert(this->alloc == rhs.alloc);
        assert(this != tiny_stl::addressof(rhs));
        if (!rhs.empty()) 
        {
            this->transfer(pos, rhs.begin(), rhs.end());    // modify pointer
            this->count += rhs.count;
            rhs.count = 0;
        }
    }

    void splice(const_iterator pos, list&& rhs) 
    {                       
        splice(pos, rhs);
    }

    void splice(const_iterator pos, list& rhs, const_iterator iter)
    {   
        // move the element pointed to by it from other into *this
        assert(this->alloc == rhs.alloc);
        assert(this != tiny_stl::addressof(rhs));

        if (!rhs.empty())
        {
            const_iterator last = iter;
            ++last;
            this->transfer(pos, iter, last);
            ++this->count;
            --rhs.count;
        }
    }

    void splice(const_iterator pos, list&& rhs, const_iterator iter)
    {
        splice(pos, rhs, iter);
    }

    void splice(const_iterator pos, list& rhs, const_iterator first, 
                const_iterator last) 
    {
        // move the elements in the range [first, last) from other into *this
        assert(this->alloc == rhs.alloc);
        assert(this != tiny_stl::addressof(rhs));

        difference_type n = tiny_stl::distance(first, last);

        if (!rhs.empty()) 
        {
            this->transfer(pos, first, last);
            this->count += n;
            rhs.count -= n;
        }
    }

    void splice(const_iterator pos, list&& rhs, const_iterator first,
                const_iterator last) 
    {
        splice(pos, rhs, first, last);
    }

    void remove(const T& val) 
    {
        for (iterator first = begin(); first != end(); ) 
        {
            if (*first == val)  // remove element
                first = erase(first);
            else                // ++iterator 
                ++first;
        }
    }


    template <typename UnaryPred>
    void remove_if(UnaryPred pred)
    {
        for (iterator first = begin(); first != end(); ) 
        {
            if (pred(*first))  // remove element
                first = erase(first);
            else                // ++iterator 
                ++first;
        }
    }

    void reserve() noexcept 
    {
        const _Nodeptr head = this->head;
        _Nodeptr p = head;

        for (;;) 
        {
            const _Nodeptr next = p->next;
            p->next = p->prev;
            p->prev = next;

            if (next == head)
                break;
            p = next;
        }
    }

    // Removes all *consecutive duplicate* elements from the container.
    void unique() 
    {
        unique(tiny_stl::equal_to<>{});
    }

    template <typename BinPred>
    void unique(BinPred pred) 
    {
        iterator first = begin();
        iterator next = first;
        
        while (++next != end()) 
        {
            if (pred(*first, *next))    // match, erase it
                erase(next);
            else
                first = next;

            next = first;
        }
    }

private:

    template <typename Cmp>
    iterator _Sort(iterator first, iterator last, 
                        Cmp& cmp, size_type size)
    {
        if (size < 2)
            return first;

        size_type mid_size = size >> 1;

        iterator mid = first;
        tiny_stl::advance(mid, mid_size);

        first = _Sort(first, mid, cmp, mid_size);
        mid = _Sort(mid, last, cmp, size - mid_size);
        iterator ret = first;   // return iter of min *iter

        bool is_once = true;

        while (true) 
        {
            if (cmp(*mid, *first))  // *mid < *first
            {   
                if (is_once)
                    ret = mid;
                iterator next = mid;
                ++next;
                this->transfer(first, mid, next);
                mid = next;
                if (mid == last)
                    return ret;
            }
            else        // *mid >= *first
            {      
                ++first;
                if (first == mid)
                    return ret;
            }

            is_once = false;
        }
       
    }

public:
    void sort() 
    {
        sort(tiny_stl::less<>{});
    }

    template <typename Cmp>
    void sort(Cmp cmp) 
    {
        _Sort(begin(), end(), cmp, this->count);
    }

private:
    [[noreturn]] static void _Xlength() 
    {
        throw "list<T> too long";
    }
};  // class list<T, Alloc>


template <typename T, typename Alloc>
inline bool operator==(const list<T, Alloc>& lhs,
                       const list<T, Alloc>& rhs) 
{
    return lhs.size() == rhs.size()
        && tiny_stl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Alloc>
inline bool operator!=(const list<T, Alloc>& lhs,
                       const list<T, Alloc>& rhs) 
{
    return !(lhs == rhs);
}

template <typename T, typename Alloc>
inline bool operator<(const list<T, Alloc>& lhs,
                      const list<T, Alloc>& rhs) 
{
    return tiny_stl::lexicographical_compare(lhs.begin(), lhs.end(),
                                            rhs.begin(), rhs.end());
}

template <typename T, typename Alloc>
inline bool operator>(const list<T, Alloc>& lhs,
                      const list<T, Alloc>& rhs)
{
    return rhs < lhs;
}

template <typename T, typename Alloc>
inline bool operator<=(const list<T, Alloc>& lhs,
                       const list<T, Alloc>& rhs) 
{
    return !(rhs < lhs);
}

template <typename T, typename Alloc>
inline bool operator>=(const list<T, Alloc>& lhs,
                       const list<T, Alloc>& rhs)
{
    return !(lhs < rhs);
}


template <typename T, typename Alloc>
inline void swap(list<T, Alloc>& lhs, list<T, Alloc>& rhs) 
    noexcept(noexcept(lhs.swap(rhs))) 
{
    lhs.swap(rhs);
}


}   // namespace tiny_stl

