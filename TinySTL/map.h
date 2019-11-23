#pragma once

#include "rbtree.h"

namespace tiny_stl
{

template <typename Key, typename T, 
    typename Compare = less<Key>, 
    typename Alloc = allocator<pair<Key, T>>>
class map : public _RBTree<pair<Key, T>, Compare, Alloc, true>
{
public:
    using key_type               = Key;
    using mapped_type            = T;
    using value_type             = pair<const Key, T>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using key_compare            = Compare;
    using allocator_type         = Alloc;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using _Al_traits             = allocator_traits<allocator_type>;
    using pointer                = typename _Al_traits::pointer;
    using const_pointer          = typename _Al_traits::const_pointer;
    using _Base                  = _RBTree<pair<Key, T>, Compare, Alloc, true>;
    using _Alnode                = typename _Base::_Alnode;
    using _Alnode_traits         = typename _Base::_Alnode_traits;
    using iterator               = typename _Base::iterator;
    using const_iterator         = typename _Base::const_iterator;
    using reverse_iterator       = typename _Base::reverse_iterator;
    using const_reverse_iterator = typename _Base::const_reverse_iterator;


    class value_compare
    {
        friend map;
    protected:
        Compare m_cmp;

        value_compare(Compare c) : m_cmp(c) { }
    public:
        bool operator()(const value_type& lhs, const value_type& rhs) const
        {
            return m_cmp(lhs.first, rhs.first);
        }
    };
public:
    map() : map(Compare()) { }
    explicit map(const Compare& cmp, const Alloc& alloc = Alloc()) 
    : _Base(cmp, alloc) { }

    explicit map(const Alloc& alloc) 
    : _Base(Compare(), alloc) { }

    template <typename InIter>
    map(InIter first, InIter last, const Compare& cmp = Compare(), 
        const Alloc& alloc = Alloc()) : _Base(cmp, alloc)
    {
        this->insert_unique(first, last);
    }

    template <typename InIter>
    map(InIter first, InIter last, const Alloc& alloc) : _Base(Compare(), alloc)
    {
        this->insert_unique(first, last);
    }

    map(const map& rhs)
    : _Base(rhs, _Al_traits::select_on_container_copy_construction(rhs.get_allocator()))
    { }

    map(const map& rhs, const Alloc& alloc)
    : _Base(rhs, alloc) { }

    map(map&& rhs) noexcept : _Base(tiny_stl::move(rhs))  {}

    map(map&& rhs, const Alloc& alloc) : _Base(tiny_stl::move(rhs), alloc) {}

    map(std::initializer_list<value_type> ilist,
        const Compare& cmp = Compare(),
        const Alloc& alloc = Alloc()) 
    : _Base(cmp, alloc)
    {
        this->insert_unique(ilist.begin(), ilist.end());
    }

    map(std::initializer_list<value_type> ilist, const Alloc& alloc)
    : map(ilist, Compare(), alloc) { }


    map& operator=(const map& rhs)
    {
        _Base::operator=(rhs);
        return *this;
    }

    map& operator=(map&& rhs)
    {
        _Base::operator=(tiny_stl::move(rhs));
        return *this;
    }

    map& operator=(std::initializer_list<value_type> ilist)
    {
        map tmp(ilist);
        this->swap(tmp);
        return *this;
    }

    T& at(const Key& key)
    {
        iterator pos = this->find(key);
        if (pos == this->end())
            _Xrange();

        return pos->second;
    }

    const T& at(const Key& key) const
    {
        const_iterator pos = this->find(key);
        if (pos == this->end())
            _Xrange();

        return pos->second;
    }

    T& operator[](const Key& key)
    {
        iterator pos = this->find(key);
        if (pos == this->end())
            return this->insert(make_pair(key, T{})).first->second;
           
        return pos->second;
    }


    T& operator[](Key&& key)
    {
        iterator pos = this->find(key);
        if (pos == this->end())
            return this->insert(make_pair(tiny_stl::move(key), T{})).first->second;

        return pos->second;
    }

    pair<iterator, bool> insert(const value_type& val)
    {
        return this->insert_unique(val);
    }

    template <typename P, typename =
        enable_if_t<is_constructible<value_type, P&&>::value>>
    pair<iterator, bool> insert(P&& val)
    {
        return this->insert_unique(tiny_stl::forward<P>(val));
    }

    pair<iterator, bool> insert(value_type&& val)
    {
        return this->insert_unique(tiny_stl::move(val));
    }

    template <typename InIter>
    void insert(InIter first, InIter last)
    {
        this->insert_unique(first, last);
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        this->insert_unique(ilist.begin(), ilist.end());
    }

    template <typename... Args>
    pair<iterator, bool> emplace(Args&&... args)
    {
        return this->emplace_unique(tiny_stl::forward<Args>(args)...);
    }

    void swap(map& rhs)
    {
        _Base::swap(rhs);
    }

    key_compare key_comp() const
    {
        return key_compare{};
    }

    value_compare value_comp() const
    {
        return value_compare{key_comp()};
    }

private:
    [[noreturn]] static void _Xrange()
    {
        throw "map<Key, T>, key is not exist";
    }
};  // map

template <typename Key, typename T, typename Cmp, typename Alloc>
inline void swap(map<Key, T, Cmp, Alloc>& lhs, map<Key, T, Cmp, Alloc>& rhs) 
    noexcept(lhs.swap(rhs))
{
    lhs.swap(rhs);
}

template <typename Key, typename T, 
    typename Compare = less<Key>, 
    typename Alloc = allocator<pair<Key, T>>>
class multimap : public _RBTree<pair<Key, T>, Compare, Alloc, true>
{
public:
    using key_type               = Key;
    using mapped_type            = T;
    using value_type             = pair<const Key, T>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using key_compare            = Compare;
    using allocator_type         = Alloc;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using _Al_traits             = allocator_traits<allocator_type>;
    using pointer                = typename _Al_traits::pointer;
    using const_pointer          = typename _Al_traits::const_pointer;
    using _Base                  = _RBTree<pair<Key, T>, Compare, Alloc, true>;
    using _Alnode                = typename _Base::_Alnode;
    using _Alnode_traits         = typename _Base::_Alnode_traits;
    using iterator               = typename _Base::iterator;
    using const_iterator         = typename _Base::const_iterator;
    using reverse_iterator       = typename _Base::reverse_iterator;
    using const_reverse_iterator = typename _Base::const_reverse_iterator;


    class value_compare
    {
        friend multimap;
    protected:
        Compare m_cmp;

        value_compare(Compare c) : m_cmp(c) {}
    public:
        bool operator()(const value_type& lhs, const value_type& rhs) const
        {
            return m_cmp(lhs.first, rhs.first);
        }
    };
public:
    multimap() : multimap(Compare()) { }
    explicit multimap(const Compare& cmp, const Alloc& alloc = Alloc()) 
    : _Base(cmp, alloc) { }

    explicit multimap(const Alloc& alloc) 
    : _Base(Compare(), alloc) { }

    template <typename InIter>
    multimap(InIter first, InIter last, const Compare& cmp = Compare(), 
        const Alloc& alloc = Alloc()) : _Base(cmp, alloc)
    {
        this->insert_equal(first, last);
    }

    template <typename InIter>
    multimap(InIter first, InIter last, const Alloc& alloc) : _Base(Compare(), alloc)
    {
        this->insert_equal(first, last);
    }

    multimap(const multimap& rhs)
    : _Base(rhs, _Al_traits::select_on_container_copy_construction(rhs.get_allocator()))
    { }

    multimap(const multimap& rhs, const Alloc& alloc)
    : _Base(rhs, alloc) { }

    multimap(multimap&& rhs) : _Base(tiny_stl::move(rhs)) {}

    multimap(multimap&& rhs, const Alloc& alloc) : _Base(tiny_stl::move(rhs), alloc) {}

    multimap(std::initializer_list<value_type> ilist,
        const Compare& cmp = Compare(),
        const Alloc& alloc = Alloc()) : _Base(cmp, alloc)
    {
        this->insert_equal(ilist.begin(), ilist.end());
    }

    multimap(std::initializer_list<value_type> ilist, const Alloc& alloc)
    : multimap(ilist, Compare(), alloc) { }


    multimap& operator=(const multimap& rhs)
    {
        _Base::operator=(rhs);
        return *this;
    }

    multimap& operator=(multimap&& rhs)
    {
        _Base::operator=(tiny_stl::move(rhs));
        return *this;
    }

    multimap& operator=(std::initializer_list<value_type> ilist)
    {
        multimap tmp(ilist);
        this->swap(tmp);
        return *this;
    }

    pair<iterator, bool> insert(const value_type& val)
    {
        return this->insert_equal(val);
    }

    template <typename P, 
        typename = enable_if_t<is_constructible<value_type, P&&>::value>>
    pair<iterator, bool> insert(P&& val)
    {
        return this->insert_equal(tiny_stl::forward<P>(val)...);
    }

    pair<iterator, bool> insert(value_type&& val)
    {
        return this->insert_equal(tiny_stl::move(val));
    }

    template <typename InIter>
    void insert(InIter first, InIter last)
    {
        this->insert_equal(first, last);
    }

    void insert(std::initializer_list<value_type> ilist)
    {
        this->insert_equal(ilist.begin(), ilist.end());
    }

    template <typename... Args>
    pair<iterator, bool> emplace(Args&&... args)
    {
        return this->emplace_equal(tiny_stl::forward<Args>(args)...);
    }

    void swap(multimap& rhs)
    {
        _Base::swap(rhs);
    }

    key_compare key_comp() const
    {
        return key_compare{};
    }

    value_compare value_comp() const
    {
        return value_compare{key_comp()};
    }

private:
    [[noreturn]] static void _Xrange()
    {
        throw "multimap<Key, T>, key is not exist";
    }
};  // multimap

template <typename Key, typename T, typename Cmp, typename Alloc>
inline void swap(multimap<Key, T, Cmp, Alloc>& lhs, multimap<Key, T, Cmp, Alloc>& rhs) 
    noexcept(lhs.swap(rhs))
{
    lhs.swap(rhs);
}

}   // namespace tiny_stl