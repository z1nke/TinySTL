#pragma once

#include "memory.h"
#include <initializer_list>

namespace tiny_stl
{

template <typename T>
struct StringConstIterator
{
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using pointer           = const T*;
    using reference         = const T&;
    using difference_type   = ptrdiff_t;
    using Self              = StringConstIterator<T>;

    T* ptr;

    StringConstIterator() = default;
    StringConstIterator(T* p) : ptr(p) {}

    reference operator*() const
    {
        return *ptr;
    }

    pointer operator->() const
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    Self& operator++()  // pre
    {
        ++ptr;
        return *this;
    }

    Self operator++(int) // post
    {
        Self tmp = *this;
        ++*this;
        return tmp;
    }

    Self& operator--()
    {
        ++ptr;
        return *this;
    }

    Self operator--(int)
    {
        Self tmp = *this;
        --*this;
        return tmp;
    }

    Self& operator+=(difference_type n)
    {
        ptr += n;
        return *this;
    }

    Self operator+(difference_type n) const
    {
        Self tmp = *this;
        return tmp += n;
    }

    Self& operator-=(difference_type n)
    {
        ptr -= n;
        return *this;
    }

    Self operator-(difference_type n)
    {
        Self tmp = *this;
        return tmp -= n;
    }

    difference_type operator-(const Self& rhs) const
    {
        return this->ptr - rhs.ptr;
    }

    reference operator[](difference_type n) const
    {
        return *(this->ptr + n);
    }

    bool operator==(const Self& rhs) const
    {
        return this->ptr == rhs.ptr;
    }

    bool operator!=(const Self& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator<(const Self& rhs) const
    {
        return this->ptr < rhs.ptr;
    }

    bool operator>(const Self& rhs) const
    {
        return rhs < *this;
    }

    bool operator<=(const Self& rhs) const
    {
        return !(rhs < *this);
    }

    bool operator>=(const Self& rhs) const
    {
        return !(*this < rhs);
    }
}; // StringConstIterator<T>

template <typename T>
struct StringIterator : StringConstIterator<T>
{
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;
    using difference_type   = ptrdiff_t;
    using Base              = StringConstIterator<T>;
    using Self              = StringIterator<T>;

    StringIterator() = default;
    StringIterator(T* p) : Base(p) {}

    reference operator*() const
    {
        return *this->ptr;
    }

    pointer operator->() const
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    Self& operator++() // pre
    {
        ++*static_cast<Base*>(this);
        return *this;
    }

    Self operator++(int) // post
    {
        Self tmp = *this;
        ++*this;
        return tmp;
    }

    Self& operator--() // pre
    {
        --*static_cast<Base*>(this);
        return *this;
    }

    Self operator--(int) // post
    {
        Self tmp = *this;
        --*this;
        return tmp;
    }

    Self& operator+=(difference_type n)
    {
        *static_cast<Base*>(this) += n;
        return *this;
    }

    Self& operator-=(difference_type n)
    {
        *static_cast<Base*>(this) -= n;
        return *this;
    }

    Self operator-(difference_type n) const
    {
        Self tmp = *this;
        return tmp -= n;
    }

    difference_type operator-(const Self& rhs) const
    {
        return this->ptr - rhs.ptr;
    }

    reference operator[](difference_type n) const
    {
        return *(this->ptr + n);
    }
}; // StringIterator<T>


template <typename CharT,
    typename Traits = std::char_traits<CharT>, 
    typename Alloc = allocator<CharT>>
class basic_string
{
public:
    static_assert(is_same<typename Traits::char_type, CharT>::value,
        "char type error");

public:
    using traits_type            = Traits;
    using value_type             = CharT;
    using allocator_type         = Alloc;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using iterator               = StringIterator<value_type>;
    using const_iterator         = StringConstIterator<value_type>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;
    using AllocTraits            = allocator_traits<Alloc>;
public:
    static const size_type npos  = static_cast<size_type>(-1);

private:

    class StringValue
    {
    public:
        static_assert(sizeof(value_type) <= 16, "size of value_type is too large");
        static constexpr const size_type kBufferSize = 16 / sizeof(value_type);
        static constexpr const size_type kBufferMask = sizeof(value_type) <= 1 ? 15 :
            sizeof(value_type) <= 2 ? 7 :
            sizeof(value_type) <= 4 ? 3 :
            sizeof(value_type) <= 8 ? 1 : 0;
    public:
        size_type size;
        size_type capacity;
        
        // short string optimization
        union Data
        {
            Data() : buf() { }
            ~Data() noexcept { }

            value_type buf[kBufferSize];
            pointer ptr;
            char placeholder[kBufferSize]; // unused
        }data;

        StringValue() : size(0), capacity(0), data() { }

        const value_type* getPtr() const 
        {
            const value_type* ptr = data.ptr;
            if (isShortString()) 
            {
                ptr = data.buf;
            }
            return ptr;
        }

        value_type* getPtr()
        {
            return const_cast<value_type*>(
                static_cast<const StringValue*>(this)->getPtr());
        }

        void checkIndex(const size_type idx) const 
        {
            if (idx > size)
            {
                xRange();
            }
        }

        bool isShortString() const
        {
            return capacity < kBufferSize;
        }

        [[noreturn]] static void xRange()
        {
            throw "invalid tiny_stl::basic_string<CharT> index";
        }
    };

private:
    extra::compress_pair<Alloc, StringValue> allocVal;

public:
    explicit basic_string(const Alloc& a)
        : allocVal(a)
    {
        initEmpty();
    }

    basic_string() noexcept(noexcept(Alloc()))
        : basic_string(Alloc()) 
    { 
    }

    basic_string(size_type count, value_type ch, const Alloc& a = Alloc())
        : allocVal(a)
    {
        init(count, ch);
    }

    basic_string(const basic_string& rhs, size_type pos,
        const Alloc& a = Alloc())
        : basic_string(rhs, pos, npos, a)
    {
    }

    basic_string(const basic_string& rhs, size_type pos,
        size_type count, const Alloc& a = Alloc())
        : allocVal(a)
    {
        initEmpty(); // for setting capacity
        init(rhs, pos, count);
    }

    basic_string(const value_type* str, size_type count, const Alloc& a)
        : allocVal(a)
    {
        initEmpty(); // for setting capacity
        init(str, count);
    }

    basic_string(const value_type* str, const Alloc& a = Alloc())
        : allocVal(a)
    {
        initEmpty(); // for setting capacity
        init(str);
    }

    template<typename InIter,
        typename = enable_if_t<is_iterator<Initer>::value>>
    basic_string(InIter first, InIter last, const Alloc& a = Alloc())
        : allocVal(a)
    {
        initEmpty(); // for setting capacity
        constructRange(first, last,
            typename iterator_traits<InIter>::iterator_category{});
    }

    basic_string(const basic_string& rhs)
        : basic_string(rhs, 
            AllocTraits::select_on_container_copy_construction(rhs.getAlloc()))
    {
    }

    basic_string(const basic_string& rhs, const Alloc& a)
        : allocVal(a)
    {
        constructCopy(rhs);
    }

    basic_string(basic_string&& rhs) noexcept
        : allocVal(tiny_stl::move(rhs.getAlloc()))
    {
        constructMove(tiny_stl::move(rhs));
    }

    basic_string(basic_string&& rhs, const Alloc& a)
        noexcept(AllocTraits::is_always_equal::value)
        : allocVal(a)
    {
        constructMove(tiny_stl::move(rhs));
    }

    basic_string(std::initializer_list<value_type> ilist, const Alloc& a = Alloc())
        : allocVal(a)
    {
        initEmpty(); // for setting capacity
        checkLength(ilist.size());
        init(ilist.begin(), ilist.size());
    }

    ~basic_string()
    {
        tidy();
    }


private:
    void constructCopy(const basic_string& rhs)
    {
        auto& rhsValue = rhs.getVal();
        const size_type rhsSize = rhsValue.size;
        const value_type* rhsPtr = rhsValue.getPtr();
        auto& value = getVal();
        if (rhsSize < StringValue::kBufferSize)
        {
            Traits::move(value.data.buf, rhsPtr, StringValue::kBufferSize);
            value.size = rhsSize;
            value.capacity = StringValue::kBufferSize - 1;
            return;
        }
        auto& alloc = getAlloc();
        const size_type newCapacity = tiny_stl::min(rhsSize | StringValue::kBufferMask,
            max_size());
        pointer newPtr = alloc.allocate(newCapacity + 1);
        value.data.ptr = newPtr;
        Traits::move(newPtr, rhsPtr, rhsSize + 1);
        value.size = rhsSize;
        value.capacity = newCapacity;
    }

    void constructMoveAux(basic_string&& rhs, true_type) noexcept
    {
        auto& value = getVal();
        auto& rhsValue = rhs.getVal();
        if (rhsValue.isShortString())
        {
            // copy short string
            Traits::move(value.data.buf, rhsValue.data.buf, rhsValue.size + 1);
        }
        else
        {
            value.data.ptr = rhsValue.data.ptr;
            rhsValue.data.ptr = pointer();
        }

        value.size = rhsValue.size;
        value.capacity = rhsValue.capacity;
        rhs.initEmpty();
    }

public:
    allocator_type get_allocator() const noexcept
    {
        return static_cast<allocator_type>(getAlloc());
    }

private:
    void initEmpty() noexcept
    {
        getVal().size = 0;
        getVal().capacity = StringValue::kBufferSize - 1;
        Traits::assign(getVal().data.buf[0], value_type());
    }

    basic_string& init(size_type count, value_type ch)
    {
        if (count <= getVal().capacity)
        {
            value_type* const ptr = getVal().getPtr();
            getVal().size = count;
            Traits::assign(ptr, count, ch);
            Traits::assign(ptr[count], value_type());

            return *this;
        }

        // allocate and assign
        return reallocAndAssign(count,
            [](value_type* const dst, size_type count, const value_type* const src)
            {
                Traits::assign(dst, count, ch);
                Traits::assign(dst[count], value_type());
            },
            ch);
    }

    basic_string& init(const basic_string& rhs, size_type pos, size_type count = npos)
    {
        rhs.getVal().checkIndex(pos);
        count = tiny_stl::min(count, rhs.getVal().size - pos);
        return init(rhs.getVal().getPtr(), count);
    }

    basic_string& init(const value_type* str, size_type count)
    {
        if (count <= getVal().capacity())
        {
            value_type* const ptr = getVal().getPtr();
            getVal().size = count;
            Traits::move(ptr, str, count);
            Traits::assign(ptr[count], value_type());
            return *this;
        }

        // allocate and assign
        return reallocAndAssign(count,
            [](value_type* const dst, size_type count, const value_type* src)
            {
                Traits::move(dst, src, count);
                Traits::move(dst[count], value_type());
            },
            str);
    }

    basic_string& init(const value_type* str)
    {
        return init(str, Traits::length(str));
    }

    template <typename Iter>
    void constructRange(Iter first, Iter last, input_iterator_tag)
    {
        TidyRAII<basic_string> guard{ this };

        for (; first != last; ++first)
        {
            push_back(*first);
        }

        guard.obj = nullptr;
    }

    template <typename Iter>
    void constructRange(Iter first, Iter last, forward_iterator_tag)
    {
        const size_type count = static_cast<size_type>(
            tiny_stl::distance(first, last));
        reserve(count);
        constructRange(first, last, input_iterator_tag);
    }

    void constructRange(const value_type* const first, const value_type* const last,
        random_access_iterator_tag)
    {
        if (first == last)
        {
            return;
        }

        init(first, static_cast<size_type>(last - first));
    }

    void constructRange(value_type* const first, value_type* const last,
        random_access_iterator_tag)
    {
        if (first == last)
        {
            return;
        }

        init(first, static_cast<size_type>(last - first));
    }

    template <typename F, typename... Args>
    basic_string& reallocAndAssign(size_type newSize, F func, Args... args)
    {
        checkLength(newSize);
        Alloc& alloc = getAlloc();
        const size_type oldCapacity = getVal().capacity;
        const size_type newCapacity = capacityGrowth(newSize);

        pointer newPtr = alloc.allocate(newCapacity + 1); // for null character
        getVal().size = newSize;
        getVal().capacity = newCapacity;
        func(newPtr, newSize, args...);

        if (oldCapacity >= StringValue::kBufferSize)
        {
            alloc.deallocate(getVal().data.ptr, oldCapacity + 1);
        }
        getVal().data.ptr = newPtr;

        return *this;
    }

    template <typename F, typename... Args>
    basic_string& reallocAndAssignGrowBy(size_type growSize, F func, Args... args)
    {
        auto& value = getVal();
        const size_type oldSize = value.size;
        // check length
        if (max_size() - oldSize < growSize)
        {
            xLength();
        }

        const size_type newSize = oldSize + growSize;
        const size_type oldCapacity = value.capacity;
        const size_type newCapacity = capacityGrowth(newSize);
        auto& alloc = getAlloc();
        pointer newPtr = alloc.allocate(newCapacity + 1);  // throws
        value.size = newSize;
        value.capacity = newCapacity;
        if (oldCapacity >= StringValue::kBufferSize)
        {
            pointer oldPtr = value.data.ptr;
            func(newPtr, oldPtr, oldSize, args...);
            alloc.deallocate(oldPtr, oldCapacity + 1);
        }
        else
        {
            func(newPtr, value.data.buf, oldSize, args...);
        }
        value.data.ptr = newPtr;
        return *this;
    }


    Alloc& getAlloc() noexcept
    {
        return allocVal.get_first();
    }

    const Alloc& getAlloc() const noexcept
    {
        return allocVal.get_first();
    }

    StringValue& getVal() noexcept
    {
        return allocVal.get_second();
    }

    const StringValue& getVal() const noexcept
    {
        return allocVal.get_second();
    }

    void tidy() noexcept
    {
        if (!getVal().isShortString())
        {
            Alloc& alloc = getAlloc();
            const pointer ptr = getVal().getPtr();
            alloc.deallocate(ptr, getVal().capacity + 1);
        }
        initEmpty();
    }

public:
    bool empty() const noexcept
    {
        return size() == 0;
    }

    size_type size() const noexcept
    {
        return allocVal.get_second().size;
    }

    size_type length() const noexcept
    {
        return size();
    }

    size_type max_size() const noexcept
    {
        return tiny_stl::min(static_cast<size_type>(-1) / sizeof(value_type) - 1,
            static_cast<size_type>(std::numeric_limits<difference_type>::max()));
    }

    void reserve(size_type newCapacity = 0)
    {
        if (newCapacity < getVal().size)
        {
            shrink_to_fit();
            return;
        }

        if (newCapacity <= getVal().capacity)
        {
            return; // do nothing
        }

        // reallocate memory if newCapacity > oldCapacity
        const size_type oldSize = getVal().size;
        reallocAndAssignGrowBy(newCapacity - oldSize,
            [](value_type* newPtr, const value_type* oldPtr, const size_type oldSizeX)
            {
                Traits::move(newPtr, oldPtr, oldSizeX + 1);
            });
        getVal().size = oldSize;
    }

    size_type capacity() const noexcept
    {
        return allocVal.get_second().capacity;
    }

    void shrink_to_fit()
    {
        // do nothing
    }

private:
    void checkLength(size_type newSize)
    {
        if (newSize >= max_size())
        {
            xLength();
        }
    }

    size_type capacityGrowth(size_type newSize)
    {
        const size_type oldSize = allocVal.get_second().size;
        const size_type masked = newSize | StringValue::kBufferMask;
        const size_type maxSize = max_size();
        if (masked > maxSize)
        {
            return maxSize;
        }

        if (oldSize > maxSize - oldSize / 2) // for avoiding overflow
        {
            return maxSize;
        }

        return tiny_stl::max(masked, oldSize + oldSize / 2);
    }

private:
    [[noreturn]] static void xLength()
    {
        throw "tiny_stl::basic_string<CharT> too long";
    }

    [[noreturn]] static void xRange()
    {
        throw "invalid tiny_stl::basic_string<CharT> index";
    }
};

using string     = basic_string<char>;
using wstring    = basic_string<wchar_t>;
using u16string  = basic_string<char16_t>;
using u32string  = basic_string<char32_t>;

} // namespace tiny_stl