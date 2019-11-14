#pragma once

#include "memory.h"
#include <initializer_list> // for std::initializer_list

namespace tiny_stl 
{

template <typename T>
struct _Vector_const_iterator 
{
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = const T*;
    using reference         = const T&;
    using _Self             = _Vector_const_iterator<T>;

    T* ptr;

    _Vector_const_iterator() : ptr() { }

    _Vector_const_iterator(T* p) : ptr(p) { }

    reference operator*() const 
    {
        return *ptr;
    }

    pointer operator->() const 
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _Self& operator++()  // pre
    {  
        ++ptr;
        return *this;
    }

    _Self operator++(int) const  // post
    {   
        _Self tmp = *this;
        ++*this;
        return tmp;
    }

    _Self& operator--() // pre
    {   
        --ptr;
        return *this;
    }

    _Self operator--(int) const // post
    { 
        _Self tmp = *this;
        --*this;
        return tmp;
    }

    _Self& operator+=(difference_type offset) 
    {
        ptr += offset;
        return *this;
    }

    _Self operator+(difference_type offset) const 
    {
        _Self tmp = *this;
        return tmp += offset;
    }

    _Self& operator-=(difference_type offset) 
    {
        ptr -= offset;
        return *this;
    }

    _Self operator-(difference_type offset) const 
    {
        _Self tmp = *this;
        return tmp -= offset;
    }

    difference_type operator-(const _Self& rhs) const 
    {
        return ptr - rhs.ptr;
    }

    reference operator[](difference_type offset) const 
    {
        return *(ptr + offset);
    }

    bool operator==(const _Self& rhs) const 
    {
        return this->ptr == rhs.ptr;
    }

    bool operator!=(const _Self& rhs) const 
    {
        return !(*this == rhs);
    }

    bool operator<(const _Self& rhs) const 
    {
        return this->ptr < rhs.ptr;
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
};  // class _Vector_const_iterator<T>

template <typename T>
inline _Vector_const_iterator<T> operator+(
            typename _Vector_const_iterator<T>::difference_type offset,
            _Vector_const_iterator<T> iter) 
{
    return iter += offset;
}

template <typename T>
struct _Vector_iterator : _Vector_const_iterator<T> 
{
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;

    using _Base             = _Vector_const_iterator<T>;
    using _Self             = _Vector_iterator<T>;

    _Vector_iterator() : _Base() { }

    _Vector_iterator(T* p) : _Base(p) { }

    reference operator*() const 
    {
        return const_cast<reference>(_Base::operator*());
    }

    pointer operator->() const
    {
        return const_cast<pointer>(_Base::operator->());
    }

    _Self& operator++() 
    {
        ++*static_cast<_Base*>(this);
        return *this;
    }

    _Self operator++(int)
    {
        _Self tmp = *this;
        ++*this;
        return tmp;
    }

    _Self& operator--() 
    {
        --*static_cast<_Base*>(this);
        return *this;
    }

    _Self operator--(int) 
    {
        _Self tmp = *this;
        --*this;
        return tmp;
    }

    _Self& operator+=(difference_type offset)
    {
        *static_cast<_Base*>(this) += offset;
        return *this;
    }

    _Self operator+(difference_type offset) const
    {
        _Self tmp = *this;
        return tmp += offset;
    }

    _Self& operator-=(difference_type offset) 
    {
        return (*this += (-offset));
    }

    _Self operator-(difference_type offset) const
    {
        _Self tmp = *this;
        return tmp -= offset;
    }

    difference_type operator-(const _Self& rhs) const
    {
        return this->ptr - rhs.ptr;
    }

    reference operator[](ptrdiff_t offset) const
    {
        return *(*this + offset);
    }
};  // class _Vector_iterator<T>

template <typename T>
inline _Vector_iterator<T> operator+(
            typename _Vector_iterator<T>::difference_type offset,
            _Vector_iterator<T> iter) 
{
    return iter += offset;
}

// no polymorphic_allocator
template <typename T, typename Alloc>
class _Vector_base 
{
public:
    using value_type             = T;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using iterator               = _Vector_iterator<T>;
    using const_iterator         = _Vector_const_iterator<T>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;
    using allocator_type         = Alloc;
protected:
    allocator_type alloc;

    T* first;
    T* last;
    T* end_of_storage;
public:

    _Vector_base(const Alloc& a)
        : alloc(a), first(), 
        last(), end_of_storage()
         { }

    T* _Allocate(size_t n)
    {
        return alloc.allocate(n);
    }

    void _Deallocate(T* p, size_t n) 
    {
        if (p != pointer()) 
            alloc.deallocate(p, n);
    }

    allocator_type& _Get_Allocator() noexcept
    {
        return alloc;
    }

    const allocator_type& _Get_Allocator() const noexcept 
    {
        return alloc;
    }

    ~_Vector_base() 
    {
        _Deallocate(first, end_of_storage - first);
    }
};  // class _Vector_base<T, Alloc>


template <typename T, typename Alloc = allocator<T>>
class vector : public _Vector_base<T, Alloc>
{
public:
    static_assert(tiny_stl::is_same_v<T, typename Alloc::value_type>, 
        "Alloc::value_type is not the same as T");

public:
    using value_type             = T;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using iterator               = _Vector_iterator<T>;
    using const_iterator         = _Vector_const_iterator<T>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;
    using allocator_type         = Alloc;

private:
    using _Base                  = _Vector_base<T, Alloc>;

private:
    using _Base::alloc;
    using _Base::first;
    using _Base::last;
    using _Base::end_of_storage;

private:
    // only allocate
    bool _Alloc(size_type newCapacity) 
    {
        this->first = pointer();
        this->last = pointer();
        this->end_of_storage = pointer();

        if (newCapacity == 0) 
            return false;

        if (newCapacity >= max_size()) 
            _Xlength();
        
        this->first = this->_Allocate(newCapacity);
        this->last = this->first;
        this->end_of_storage = this->first + newCapacity;
        return true;
    }

    //  constrcut
    pointer _Fill(pointer dest, size_type count, const_reference val)
    {
        return _Uninitialized_alloc_fill_n(dest, count, val, this->alloc);
    }

    // default construct
    pointer _Default(pointer dest, size_type count) 
    {
        return _Uninitialized_alloc_default_n(dest, count, this->alloc);
    }


    // move construct
    void __Move_or_copy(pointer _first, pointer _last, 
                        pointer newFirst, true_type) 
    {
        _Uninitialized_alloc_move(_first, _last, newFirst, this->alloc);
    }

    // copy construct
    void __Move_or_copy(pointer _first, pointer _last, 
                        pointer newFirst, false_type) 
    {
        _Uninitialized_alloc_copy(_first, _last, newFirst, this->alloc);
    }

    void _Move_or_copy(pointer _first, pointer _last, pointer newFirst) 
    {
        __Move_or_copy(_first, _last, newFirst,
            typename tiny_stl::disjunction<is_nothrow_move_constructible<T>,
                negation<is_copy_constructible<T>>>::type());
    }

    pointer _Move(pointer _first, pointer _last, pointer newFirst) 
    {
        return _Uninitialized_alloc_move(_first, _last, newFirst, this->alloc);
    }

    template <typename Iter>    // ? Iter
    pointer _Copy(Iter _first, Iter _last, pointer newFirst) 
    {
        return _Uninitialized_alloc_copy(_first, _last, newFirst, this->alloc);
    }

    // destory and deallocate old array
    void _Tidy() 
    {
        if (this->first != pointer()) 
        {
            _Destroy_alloc_range(this->first, this->last, this->alloc);
            this->alloc.deallocate(this->first, capacity());

            this->first = pointer();
            this->last = pointer();
            this->end_of_storage = pointer();
        }
    }

public:
    vector() noexcept(noexcept(Alloc()))                        // (1)
    : vector(Alloc()) { }
    explicit vector(const Alloc& alloc) noexcept 
    : _Base(alloc) { }

    vector(size_type count, const T& val,                       // (2)
        const Alloc& alloc = Alloc()) 
    : _Base(alloc) 
    {
        try
        {
            if (_Alloc(count))
                this->last = _Fill(this->first, count, val);
        }
        catch (...) 
        {
            _Tidy();
            throw;
        }
    }

    explicit vector(size_type count,                            // (3)
        const Alloc& alloc = Alloc()) 
    : _Base(alloc) 
    {
        try
        {
            if (_Alloc(count))
                this->last = _Default(this->first, count);
        }
        catch (...) 
        {
            _Tidy();
            throw;
        }
    }

private:
    template <typename InIter>
    void _Range_construct(InIter _first, InIter _last, input_iterator_tag) 
    {
        // input_iterator has no distance function
        try
        {
            for (; _first != _last; ++_first)
                emplace_back(*_first);   // copy
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }
    
    template <typename FwdIter>
    void _Range_construct(FwdIter _first, FwdIter _last, forward_iterator_tag) 
    {
        size_type size = static_cast<size_type>(tiny_stl::distance(_first, _last));
        try
        {
            if (_Alloc(size))
                this->last = _Copy(_first, _last, this->first);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

public:

    template <typename InIter,                                  // (4)
        typename = enable_if_t<is_iterator<InIter>::value>>
    vector(InIter _first, InIter _last, const Alloc& alloc = Alloc()) 
    : _Base(alloc) 
    {
        _Range_construct(_first, _last,
            typename iterator_traits<InIter>::iterator_category{});
    }

    
    vector(const vector& rhs)                                   // (5)
    : _Base(allocator_traits<Alloc>::select_on_container_copy_construction(rhs.alloc)) 
    {
        try
        {
            if (_Alloc(rhs.size()))
                this->last = _Copy(rhs.first, rhs.last, this->first);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

    vector(const vector& rhs, const Alloc& alloc)               // (5) alloc
    : _Base(alloc) 
    {
        try
        {
            if (_Alloc(rhs.size()))
                this->last = _Copy(rhs.first, rhs.last, this->first);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

private:
    void _Move_construct(vector&& rhs, true_type) noexcept
    {
        this->first = rhs.first;
        this->last = rhs.last;
        this->end_of_storage = rhs.end_of_storage;

        rhs.first = pointer();
        rhs.last = pointer();
        rhs.end_of_storage = pointer();
    }

    void _Move_construct(vector&& rhs, false_type) 
    {
        if (this->alloc == rhs.alloc)
            _Move_construct(tiny_stl::move(rhs), true_type{});
        else if (_Alloc(rhs.size())) 
            this->last = _Move(rhs.first, rhs.last, this->first);
    }

public:
    vector(vector&& rhs) noexcept                               // (6)
    : _Base(tiny_stl::move(rhs.alloc)) 
    {                     
        _Move_construct(tiny_stl::move(rhs), true_type{});
    }

    vector(vector&& rhs, const Alloc& alloc)                    // (7)
    : _Base(alloc) 
    {
        // FIXME, no strong exception
        _Move_construct(rhs.first, rhs.last,
            typename allocator_traits<Alloc>::is_always_equal{});
    }

    vector(std::initializer_list<T> ilist,                      // (8)
        const Alloc& alloc = Alloc()) 
    : _Base(alloc)
    {
        _Range_construct(ilist.begin(), ilist.end(),
            random_access_iterator_tag{});
    }

    ~vector()
    {
        _Tidy();
    }

private:
    template <typename InIter>
    void _Assign_copy_range(InIter _first, InIter _last, input_iterator_tag)
    {
        _Destroy_range(this->first, this->last);

        InIter newLast = this->first;
        for (; _first != _last && newLast != this->last; ++_first, ++newLast)
            *newLast = *_first;

        this->last = newLast;
        for (; _first != _last; ++_first) 
            emplace_back(*_first);
    }

    template <typename FwdIter>
    void _Assign_copy_range(FwdIter _first, FwdIter _last, forward_iterator_tag) 
    {
        const size_type newSize = static_cast<size_type>(tiny_stl::distance(_first, _last));
        const size_type oldCapacity = capacity();

        try
        {
            if (newSize > oldCapacity)   // reallocate
            {
                if (newSize > max_size())
                    _Xlength();

                if (this->first != pointer())
                    _Tidy();

                _Alloc(newSize);
                this->last = _Copy(_first, _last, this->first);
            }
            else    // no reallocate
            {
                _Destroy_range(this->first, this->last);
                this->last = this->_Copy(_first, _last, this->first);
            }
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }


    void _Assign_move(vector&& rhs, true_type) noexcept
    {
        _Move_construct(tiny_stl::move(rhs), true_type{});
    }

    void _Assign_move(vector&& rhs, false_type) 
    {
        if (this->alloc == rhs.alloc)
            _Move_construct(tiny_stl::move(rhs), true_type{});

        // Move individually
        const size_type newSize = rhs.size();
        const size_type oldCapacity = this->capacity();

        try
        {
            if (newSize > oldCapacity)   // reallocate
            {
                if (newSize > max_size())
                    _Xlength();

                if (this->first != pointer())
                    _Tidy();

                _Alloc(newSize);
                this->last = _Move(rhs.first, rhs.last, this->first);
            }
            else     // no reallocate
            {
                _Destroy_range(this->first, this->last);
                this->last = _Move(rhs.first, rhs.last, this->first);
            }
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }
public:
    void assign(size_type n, const T& val) 
    {
        const size_type oldCapacity = capacity();

        try
        {
            if (n > oldCapacity)   // reallocate
            {
                if (n > max_size())
                    _Xlength();

                if (this->first != pointer())
                    _Tidy();

                _Alloc(n);
                this->last = _Fill(this->first, n, val);
            }
            else    // no reallocate
            {
                _Destroy_range(this->first, this->last);
                this->last = _Fill(this->first, n, val);
            }
        }
        catch (...)
        {
            _Tidy();
            throw;
        }
    }

    template <typename InIter, typename = enable_if_t<is_iterator<InIter>::value>> 
    void assign(InIter _first, InIter _last) 
    {
        _Assign_copy_range(_first, _last,
            typename iterator_traits<InIter>::iterator_category{});
    }

    void assign(std::initializer_list<T> ilist)
    {
        _Assign_copy_range(ilist.begin(), ilist.end(), random_access_iterator_tag{});
    }

    vector& operator=(const vector& rhs) 
    {
        assert(this != tiny_stl::addressof(rhs));

        if (this->alloc != rhs.alloc) 
            _Tidy();    // this->alloc deallocate elements

#pragma warning(push)   // if constexpr
#pragma warning(disable : 4984)
        if constexpr (allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) 
            this->alloc = rhs.alloc;
#pragma warning(pop)
       
        assign(rhs.first, rhs.last);
        return *this;
    }


    vector& operator=(vector&& rhs) 
        noexcept(allocator_traits<Alloc>::propagate_on_container_move_assignment::value
            || allocator_traits<Alloc>::is_always_equal::value) 
    {
        assert(this != tiny_stl::addressof(rhs));

#pragma warning(push)   // if constexpr
#pragma warning(disable : 4984)
        if constexpr (allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            this->alloc = rhs.alloc;
#pragma warning(pop)

        _Assign_move(tiny_stl::move(rhs), disjunction<
            typename allocator_traits<Alloc>::propagate_on_container_move_assignment,
            typename allocator_traits<Alloc>::is_always_equal>{});
        
        return *this;
    }

    vector& operator=(std::initializer_list<T> ilist) 
    {
        _Assign_copy_range(ilist.begin(), ilist.end(), random_access_iterator_tag{});
        return *this;
    }

    allocator_type get_allocator() const 
    {
        return static_cast<allocator_type>(this->alloc);
    }

    T& at(size_type pos) 
    {
        if (pos >= size()) 
            _Xrange();

        return this->first[pos];
    }

    const T& at(size_type pos) const 
    {
        if (pos >= size()) 
            _Xrange();

        return this->first[pos];
    }

    T& operator[](size_type pos)
    {
        assert(pos < size());
        return this->first[pos];
    }

    const T& operator[](size_type pos) const 
    {
        assert(pos < size());
        return this->first[pos];
    }

    T& front()
    {
        assert(!empty());
        return *this->first;
    }

    const T& front() const 
    {
        assert(!empty());
        return *this->first;
    }

    T& back()
    {
        assert(!this->empty());
        return this->last[-1];
    }

    const T& back() const
    {
        assert(!this->empty());
        return this->last[-1];
    }

    T* data() noexcept
    {
        return this->first;
    }

    const T* data() const noexcept
    {
        return this->first;
    }
    
    iterator begin() noexcept
    {
        return iterator(first);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(first);
    }

    const_iterator cbegin() const noexcept 
    {
        return begin();
    }

    iterator end() noexcept 
    {
        return iterator(last);
    }

    const_iterator end() const noexcept
    {
        return iterator(last);
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
        return rend();
    }


private:
    void _Update_pointer(const pointer newFirst, size_type newSize, 
                         size_type newCapacity) 
    {
        if (this->first != pointer()) 
        {
            _Destroy_alloc_range(this->first, this->last, this->alloc);
            this->alloc.deallocate(this->first, capacity());
        }

        this->first = newFirst;
        this->last = newFirst + newSize;
        this->end_of_storage = newFirst + newCapacity;
    }

    void _Realloc_and_init(size_type newCapacity) 
    {
        const size_type newSize = size(); 
        const pointer newFirst = this->alloc.allocate(newCapacity);

        try
        {
            _Move_or_copy(this->first, this->last, newFirst);
        }
        catch (...)
        {
            _Tidy();
            throw;
        }

        // destroy/deallocate old elements, update new pointer
        _Update_pointer(newFirst, newSize, newCapacity);   
    }

    size_type _Capacity_growth(size_type newSize) const 
    {
        const size_type oldCapacity = capacity();

        if ((oldCapacity << 1) > max_size()) 
            return newSize;

        const size_type newCapacity = (oldCapacity << 1);

        return newCapacity < newSize ? newSize : newCapacity;
    }

public:
    bool empty() const noexcept 
    {
        return begin() == end();
    }

    size_type size() const noexcept
    {
        return static_cast<size_type>(tiny_stl::distance(begin(), end()));
    }

    size_type max_size() const noexcept
    {
        return sizeof(T) == 1 ? 
            static_cast<size_type>(-1) >> 1: 
            static_cast<size_type>(-1) / sizeof(T);
    }

    void reserve(size_type newcapacity)
    {
        if (newcapacity > capacity()) 
        {
            if (newcapacity > max_size()) 
                _Xlength();

            _Realloc_and_init(newcapacity);
        }
        // else do nothing
    }

    size_type capacity() const noexcept 
    {
        return this->end_of_storage - this->first;
    }

    void shrink_to_fit() 
    {
        if (size() < capacity()) 
        {
            if (empty()) 
                _Tidy(); // no destroy, only deallocate
            else 
                _Realloc_and_init(size());
        }
    }
public:
    void clear() noexcept 
    {
        _Destroy_alloc_range(this->first, this->last, this->alloc);
        this->last = this->first;
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        if (this->last != this->end_of_storage)  // has unused capacity
        {
            allocator_traits<Alloc>::construct(this->alloc,
                this->last, tiny_stl::forward<Args>(args)...);
            ++this->last;
        }
        else   // reallocate
        {
            const size_type oldSize = size();
            if (oldSize == max_size()) 
                _Xlength();

            const size_type newSize = oldSize + 1;

            // normal: capacity <<= 1
            const size_type newCapacity = _Capacity_growth(newSize);

            try
            {
                const pointer newFirst = this->alloc.allocate(newCapacity);

                allocator_traits<Alloc>::construct(this->alloc,
                    tiny_stl::addressof(*(newFirst + oldSize)),
                    tiny_stl::forward<Args>(args)...);

                _Move_or_copy(this->first, this->last, newFirst);
                _Update_pointer(newFirst, newSize, newCapacity);
            }
            catch (...)
            {
                _Tidy();
                throw;
            }
        }
    }

    void push_back(const_reference val) 
    {
        emplace_back(val);
    }

    void push_back(T&& val)
    {
        emplace_back(tiny_stl::move(val));
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        assert(pos.ptr >= this->first && pos.ptr <= this->last);
        const bool is_back = pos.ptr == this->last;
        const size_type offset = pos.ptr - this->first;
        
        if (this->last == this->end_of_storage)  // reallocate
        {  
            const size_type oldSize = size();
            if (oldSize == max_size()) 
                _Xlength();

            // reallocate
            const size_type newSize = oldSize + 1;
            const size_type newCapacity = _Capacity_growth(newSize);

            try
            {
                const pointer newFirst = this->alloc.allocate(newCapacity);

                // construct
                allocator_traits<Alloc>::construct(this->alloc,
                    tiny_stl::addressof(*(newFirst + offset)),
                    tiny_stl::forward<Args>(args)...);

                if (is_back) // Strong exception guarantee
                {
                    _Move_or_copy(this->first, this->last, newFirst);
                }
                else
                {
                    _Move(this->first, pos.ptr, newFirst);
                    _Move(pos.ptr, this->last, newFirst + offset + 1);
                }

                _Update_pointer(newFirst, newSize, newCapacity);
            }
            catch (...)
            {
                _Tidy();
                throw;
            }
        }
        else if (is_back)   // no reallocate, and emplace at back
        {
            allocator_traits<Alloc>::construct(this->alloc,
                tiny_stl::addressof(*this->last), 
                tiny_stl::forward<Args>(args)...);
            ++this->last;
        }
        else      // no reallocate, move old elements
        {                
            pointer oldLast = this->last;
            for (; pos.ptr != oldLast; --oldLast) 
                *oldLast = tiny_stl::move(oldLast[-1]);
            
            T obj(tiny_stl::forward<Args>(args)...);
            ++this->last;
            *oldLast = tiny_stl::move(obj);
        }
        return begin() + offset;
    }

    iterator insert(const_iterator pos, const T& val)                   // (1)
    {                 
        return emplace(pos, val);
    }

    iterator insert(const_iterator pos, T&& val)                        // (2)
    {   
        return emplace(pos, tiny_stl::move(val));
    }

    iterator insert(const_iterator pos, size_type n, const T& val)      // (3)
    {    
        assert(pos.ptr >= this->first && pos.ptr <= this->last);

        if (n == 1 && pos.ptr == this->last) 
            return emplace(pos, val);     // Strong exception guarantee

        const size_type offset = pos.ptr - this->first;
        if (n == 0) 
        {
            // do nothing
        }
        else if (n > static_cast<size_type>(this->end_of_storage - this->last)) 
        {
            // reallocate
            const size_type oldSize = size();
            if (n + oldSize > max_size()) 
                _Xlength();
           
            const size_type newSize = oldSize + n;
            const size_type newCapacity = _Capacity_growth(newSize);

            try
            {
                const pointer newFirst = this->alloc.allocate(newCapacity);
                _Fill(newFirst + offset, n, val);
                _Move(this->first, pos.ptr, newFirst);
                _Move(pos.ptr, this->last, newFirst + offset + n);
                _Update_pointer(newFirst, newSize, newCapacity);
            }
            catch (...)
            {
                _Tidy();
                throw;
            }
        }
        else    // no reallocate
        {  
            const pointer oldLast = this->last;
            const size_type number_move = oldLast - pos.ptr;

            try
            {
                if (n >= number_move)   // no move backward
                {
                    this->last = _Fill(oldLast, n - number_move, val);
                    this->last = _Move(pos.ptr, pos.ptr + number_move, this->last);
                    fill(pos.ptr, oldLast, val);
                }
                else     // move backward
                {
                    this->last = oldLast + n;
                    pointer p = const_cast<pointer>(oldLast + (n - 1));
                    for (size_type i = 0; i < number_move; ++i, --p)
                        *p = tiny_stl::move(p[-static_cast<difference_type>(n)]);

                    fill(pos.ptr, p, val);
                }
            }
            catch (...)
            {
                _Tidy();
                throw;
            }
        }

        return begin() + offset;
    }

private:
    template <typename InIter>
    void _Insert_range(const_iterator pos, InIter _first, InIter _last, input_iterator_tag) 
    {
        if (_first == _last)    // empty range
            return;

        // 1 2 3 6 insert 4 5 => 1 2 3 4 5 6
        for (; _first != _last; ++_first) 
            emplace_back(*_first);  // 1 2 3 6 4 5

        const size_type offset = pos.ptr - this->first;
        const size_type oldSize = size();
        // 1 2 3 6 4 5
        //       f m   l   
        // rotate =>  1 2 3 4 5 6
        rotate(this->first + offset, this->first + oldSize, this->last);
    }

    template <typename FwdIter>
    void _Insert_range(const_iterator pos, FwdIter _first, FwdIter _last, forward_iterator_tag) 
    {
        const size_type n = static_cast<size_type>(tiny_stl::distance(_first, _last));
        const size_type offset = pos.ptr - this->first;

        if (n == 1 && pos.ptr == this->last) 
            emplace(pos.ptr, *_first);
    
        if (n == 0) 
        {
            // do nothing
        }
        else if (n > static_cast<size_type>(this->end_of_storage - this->last)) 
        {
            // reallocate
            const size_type oldSize = size();
            if (n + oldSize > max_size()) 
                _Xlength();
            
            const size_type newSize = oldSize + n;
            const size_type newCapacity = _Capacity_growth(newSize);

            try
            {
                const pointer newFirst = this->alloc.allocate(newCapacity);

                _Copy(_first, _last, newFirst + offset);
                _Move(this->first, pos.ptr, newFirst);
                _Move(pos.ptr, this->last, newFirst + offset + n);

                _Update_pointer(newFirst, newSize, newCapacity);
            }
            catch (...)
            {
                _Tidy();
                throw;
            }
        }
        else     // no reallocate
        {      
            const pointer oldLast = this->last;
            const size_type number_move = oldLast - pos.ptr;
            if (n >= number_move)   // no move backward
            {  
                const pointer newPos = this->first + (offset + n);
                this->last = _Move(pos.ptr, oldLast, newPos);

                try
                {
                    _Copy(_first, _last, pos.ptr);
                }
                catch (...)
                {
                    // FIXME, move rollback
                    _Tidy();
                    throw;
                }
            }
            else     // move backward
            {                
                this->last = oldLast + n;
                pointer p = const_cast<pointer>(oldLast + (n - 1));
                for (size_type i = 0; i < number_move; ++i, --p) 
                    *p = tiny_stl::move(p[-static_cast<difference_type>(n)]);
                
                try
                {
                    _Copy(_first, _last, pos.ptr);
                }
                catch (...)
                {
                    // FIXME, move rollback
                    _Tidy();
                    throw;
                }
            }
        }
    }

public:
    template <typename InIter, typename = enable_if_t<is_iterator<InIter>::value>>
    iterator insert(const_iterator pos, InIter _first, InIter _last)
    {
        assert(pos.ptr >= this->first && pos.ptr <= this->last);
        const size_type offset = pos.ptr - this->first;
        _Insert_range(pos.ptr, _first, _last,
            typename iterator_traits<InIter>::iterator_category{});
        return this->first + offset;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist) 
    {
        return insert(pos, ilist.begin(), ilist.end());
    }

    iterator erase(const_iterator pos) 
    {
        assert(pos.ptr >= this->first && pos.ptr < this->last);

        const size_type offset = pos.ptr - this->first;
        _Move(pos.ptr + 1, this->last, pos.ptr);
        this->alloc.destroy(tiny_stl::addressof(*(this->last - 1)));
        --this->last;
        return this->first + offset;
    }

    iterator erase(const_iterator _first, const_iterator _last) 
    {
        assert(_first.ptr == _last.ptr
            || (_first.ptr >= this->first
                && _first.ptr < _last.ptr
                && _last.ptr <= this->last));
        const size_type offset = _first.ptr - this->first;
        if (_first != _last)
        {
            const pointer newLast = _Move(_last.ptr, this->last, _first.ptr);
            _Destroy_range(newLast, this->last);
            this->last = newLast;
        }
        return this->first + offset;
    }

    void pop_back() 
    {
        assert(!empty());
        allocator_traits<Alloc>::destroy(this->alloc, 
            tiny_stl::addressof(*(--this->last)));
    }

private:
    template <typename Lambda>
    void _Resize(size_type newSize, Lambda default_or_fill) 
    {
        const size_type oldSize = size();
        const size_type oldCapacity = capacity();

        if (newSize > oldCapacity) {        // reallocate
            if (newSize > max_size()) 
                _Xlength();

            size_type newCapacity = (newSize >> 1) + newSize;

            const pointer newFirst = nullptr;
            try
            {
                newFirst = this->alloc.allocate(newCapacity);
                _Move_or_copy(this->first, this->last, newFirst);
            }
            catch (...)
            {
                _Tidy();
                throw;
            }

            // destroy/deallocate old elements, update new pointer
            _Update_pointer(newFirst, newSize, newCapacity);
        }
        else if (newSize < oldSize)         // update pointer, size = newSize
        {      
            const pointer newLast = this->first + newSize;
            _Destroy_range(newLast, this->last);
            this->last = newLast;
        }
        else if (newSize > oldSize)         // use lambda to append elements
        {       
            const pointer oldLast = this->last;
            this->last = default_or_fill(oldLast, newSize - oldSize);
        }
    }

public:
    void resize(size_type newSize) 
    {
        auto lambda_default = [this](pointer oldLast, size_type n) 
        {
            return _Default(oldLast, n);
        };
        _Resize(newSize, lambda_default);
    }

    void resize(size_type newSize, const T& val)
    {
        auto lambda_fill = [this, &val](pointer oldLast, size_type n) 
        {
            return _Fill(oldLast, n, val);
        };
        _Resize(newSize, lambda_fill);
    }

    void swap(vector& rhs) noexcept(noexcept(
            allocator_traits<Alloc>::propagate_on_container_swap::value
            || allocator_traits<Alloc>::is_always_equal::value)) 
    {
        _Swap_alloc(this->alloc, rhs.alloc);
        _Swap_ADL(this->first, rhs.first);
        _Swap_ADL(this->last, rhs.last);
        _Swap_ADL(this->end_of_storage, rhs.end_of_storage);
    }

private:
    [[noreturn]] static void _Xlength() 
    {
        throw "vector<T> too long";
    }

    [[noreturn]] static void _Xrange() 
    {
        throw "invalid vector<T> subscript";
    }
};  // class vector<T>

template <typename T, typename Alloc>
inline bool operator==(const vector<T, Alloc>& lhs, 
                       const vector<T, Alloc>& rhs)
{
    return lhs.size() == rhs.size()
        && tiny_stl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Alloc>
inline bool operator!=(const vector<T, Alloc>& lhs, 
                       const vector<T, Alloc>& rhs) 
{
    return (!(lhs == rhs));
}

template <typename T, typename Alloc>
inline bool operator<(const vector<T, Alloc>& lhs, 
                      const vector<T, Alloc>& rhs)
{
    return tiny_stl::lexicographical_compare(lhs.begin(), lhs.end(),
        rhs.begin(), rhs.end());
}

template <typename T, typename Alloc>
inline bool operator<=(const vector<T, Alloc>& lhs, 
                       const vector<T, Alloc>& rhs) 
{
    return (!(rhs < lhs));
}

template <typename T, typename Alloc>
inline bool operator>(const vector<T, Alloc>& lhs, 
                      const vector<T, Alloc>& rhs) 
{
    return rhs < lhs;
}

template <typename T, typename Alloc>
inline bool operator>=(const vector<T, Alloc>& lhs, 
                       const vector<T, Alloc>& rhs)
{
    return (!(lhs < rhs));
}

template <typename T, typename Alloc>
inline void swap(vector<T, Alloc>& lhs, vector<T, Alloc>& rhs) 
    noexcept(noexcept(lhs.swap(rhs))) 
{
    lhs.swap(rhs);
}

}   // namespace tiny_stl