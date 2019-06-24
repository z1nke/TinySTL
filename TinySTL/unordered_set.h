#pragma once

#include "hashtable.h"

namespace tiny_stl
{

template <typename Key, 
    typename Hash = hash<Key>, 
    typename KeyEqual = equal_to<Key>, 
    typename Alloc = allocator<Key>>
class unordered_set : public _HashTable<Key, Hash, KeyEqual, Alloc, false>
{
public:
    using key_type              = Key;
    using value_type            = Key;
    using size_type             = size_t;
    using difference_type       = ptrdiff_t;
    using hasher                = Hash;
    using key_equal             = KeyEqual;
    using allocator_type        = Alloc;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using _Al_traits            = allocator_traits<allocator_type>;
    using pointer               = typename _Al_traits::pointer;
    using const_pointer         = typename _Al_traits::const_pointer;
    using _Base                 = _HashTable<Key, Hash, KeyEqual, Alloc, false>;
    using iterator              = typename _Base::iterator;
    using const_iterator        = typename _Base::const_iterator;
    using local_iterator        = typename _Base::local_iterator;
    using const_local_iterator  = typename _Base::const_local_iterator;

public:
    unordered_set() : unordered_set(0) { }                          // (1)
    explicit unordered_set(size_type num_bucket,
        const Hash& hashfunc = Hash(),
        const KeyEqual& eq = KeyEqual(),
        const Alloc& alloc = Alloc()) 
    : _Base(num_bucket, alloc, hashfunc, eq) { }

    unordered_set(size_type num_bucket, const Alloc& alloc)         // (1)
    : unordered_set(num_bucket, Hash(), KeyEqual(), alloc) { }

    unordered_set(size_type num_bucket,                             // (1)
        const Hash& hashfunc, const Alloc& alloc)
    : unordered_set(num_bucket, hashfunc, KeyEqual(), alloc) { }

    explicit unordered_set(const Alloc& alloc)                      // (1)
    : _Base(0, alloc) { }

    template <typename InIter>                                      // (2)
    unordered_set(InIter first, InIter last,
        size_type num_bucket = 0,
        const Hash& hashfunc = Hash(),
        const KeyEqual& eq = KeyEqual(),
        const Alloc& alloc = Alloc())
    : _Base(num_bucket, alloc, hashfunc, eq)
    {
        this->insert_unique(first, last);
    }

    template <typename InIter>                                      // (2)
    unordered_set(InIter first, InIter last,
        size_type num_bucket,
        const Alloc& alloc)
    : unordered_set(first, last, num_bucket, 
        Hash(), KeyEqual(), alloc) { }

    template <typename InIter>                                      // (2)
    unordered_set(InIter first, InIter last,
        size_type num_bucket,
        const Hash& hashfunc,
        const Alloc& alloc)
    : unordered_set(first, last, num_bucket, 
        hashfunc, KeyEqual(), alloc) { }


    unordered_set(const unordered_set& rhs)                         // (3)
    : _Base(rhs) { }

    unordered_set(const unordered_set& rhs,                         // (3)
        const Alloc& alloc) : _Base(rhs, alloc) { }

    unordered_set(unordered_set&& rhs)                              // (4)
    : _Base(tiny_stl::move(rhs)) { }

    unordered_set(std::initializer_list<value_type> ilist,          // (5)
        size_type num_bucket = 0,
        const Hash& hashfunc = Hash(),
        const KeyEqual& eq = KeyEqual(),
        const Alloc& alloc = Alloc())
    : unordered_set(ilist.begin(), ilist.end(), 
        num_bucket, hashfunc, eq, alloc) { }

    unordered_set(std::initializer_list<value_type> ilist,          // (5)
        size_type num_bucket, const Alloc& alloc)
    : unordered_set(ilist, num_bucket,
        Hash(), KeyEqual(), alloc) { }

    unordered_set(std::initializer_list<value_type> ilist,          // (5)
        size_type num_bucket, const Hash& hashfunc,
        const Alloc& alloc)
    : unordered_set(ilist, num_bucket,
        hashfunc, KeyEqual(), alloc) { }

    unordered_set& operator=(const unordered_set& rhs)
    {
        _Base::operator=(rhs);
        return *this;
    }

    unordered_set& operator=(unordered_set&& rhs)
    {
        _Base::operator=(tiny_stl::move(rhs));
        return *this;
    }

    unordered_set& operator=(std::initializer_list<value_type> ilist)
    {
        this->clear();
        this->insert(ilist);
        return *this;
    }

    size_type count(const key_type& key) const
    {
        return this->count_unique(key);
    }

    pair<iterator, bool> insert(const value_type& val)
    {
        return this->insert_unique(val);
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
        this->emplace_unique(tiny_stl::forward<Args>(args)...);
    }

    void swap(unordered_set& rhs)
    {
        _Base::swap(rhs);
    }
};  // unordered_set

template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
void swap(unordered_set<Key, Hash, KeyEqual, Alloc>& lhs,
          unordered_set<Key, Hash, KeyEqual, Alloc>& rhs)
{
    lhs.swap(rhs);
}


template <typename Key, 
    typename Hash = hash<Key>, 
    typename KeyEqual = equal_to<Key>, 
    typename Alloc = allocator<Key>>
class unordered_multiset : public _HashTable<Key, Hash, KeyEqual, Alloc, false>
{
public:
    using key_type              = Key;
    using value_type            = Key;
    using size_type             = size_t;
    using difference_type       = ptrdiff_t;
    using hasher                = Hash;
    using key_equal             = KeyEqual;
    using allocator_type        = Alloc;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using _Al_traits            = allocator_traits<allocator_type>;
    using pointer               = typename _Al_traits::pointer;
    using const_pointer         = typename _Al_traits::const_pointer;
    using _Base                 = _HashTable<Key, Hash, KeyEqual, Alloc, false>;
    using iterator              = typename _Base::iterator;
    using const_iterator        = typename _Base::const_iterator;
    using local_iterator        = typename _Base::local_iterator;
    using const_local_iterator  = typename _Base::const_local_iterator;

public:
    unordered_multiset() : unordered_multiset(0) { }                    // (1)
    explicit unordered_multiset(size_type num_bucket,
        const Hash& hashfunc = Hash(),
        const KeyEqual& eq = KeyEqual(),
        const Alloc& alloc = Alloc()) 
    : _Base(num_bucket, alloc, hashfunc, eq) { }

    unordered_multiset(size_type num_bucket, const Alloc& alloc)        // (1)
    : unordered_multiset(num_bucket, Hash(), KeyEqual(), alloc) { }

    unordered_multiset(size_type num_bucket,                            // (1)
        const Hash& hashfunc, const Alloc& alloc)
    : unordered_multiset(num_bucket, hashfunc, KeyEqual(), alloc) { }

    explicit unordered_multiset(const Alloc& alloc)                     // (1)
    : _Base(0, alloc) { }

    template <typename InIter>                                          // (2)
    unordered_multiset(InIter first, InIter last,
        size_type num_bucket = 0,
        const Hash& hashfunc = Hash(),
        const KeyEqual& eq = KeyEqual(),
        const Alloc& alloc = Alloc())
    : _Base(num_bucket, alloc, hashfunc, eq)
    {
        this->insert_equal(first, last);
    }

    template <typename InIter>                                          // (2)
    unordered_multiset(InIter first, InIter last,
        size_type num_bucket,
        const Alloc& alloc)
    : unordered_multiset(first, last, num_bucket, 
        Hash(), KeyEqual(), alloc) { }

    template <typename InIter>                                          // (2)
    unordered_multiset(InIter first, InIter last,
        size_type num_bucket,
        const Hash& hashfunc,
        const Alloc& alloc)
    : unordered_multiset(first, last, num_bucket, 
        hashfunc, KeyEqual(), alloc) { }


    unordered_multiset(const unordered_multiset& rhs)                   // (3)
    : _Base(rhs) { }

    unordered_multiset(const unordered_multiset& rhs,                   // (3)
        const Alloc& alloc) : _Base(rhs, alloc) { }

    unordered_multiset(unordered_multiset&& rhs)                        // (4)
    : _Base(tiny_stl::move(rhs)) { }

    unordered_multiset(std::initializer_list<value_type> ilist,         // (5)
        size_type num_bucket = 0,
        const Hash& hashfunc = Hash(),
        const KeyEqual& eq = KeyEqual(),
        const Alloc& alloc = Alloc())
    : unordered_multiset(ilist.begin(), ilist.end(), 
        num_bucket, hashfunc, eq, alloc) { }

    unordered_multiset(std::initializer_list<value_type> ilist,         // (5)
        size_type num_bucket, const Alloc& alloc)
    : unordered_multiset(ilist, num_bucket,
        Hash(), KeyEqual(), alloc) { }

    unordered_multiset(std::initializer_list<value_type> ilist,         // (5)
        size_type num_bucket, const Hash& hashfunc,
        const Alloc& alloc)
    : unordered_multiset(ilist, num_bucket,
        hashfunc, KeyEqual(), alloc) { }

    unordered_multiset& operator=(const unordered_multiset& rhs)
    {
        _Base::operator=(rhs);
        return *this;
    }

    unordered_multiset& operator=(unordered_multiset&& rhs)
    {
        _Base::operator=(tiny_stl::move(rhs));
        return *this;
    }

    unordered_multiset& operator=(std::initializer_list<value_type> ilist)
    {
        this->clear();
        this->insert(ilist);
        return *this;
    }

    size_type count(const key_type& key) const
    {
        return this->count_equal(key);
    }

    iterator insert(const value_type& val)
    {
        return this->insert_equal(val);
    }

    iterator insert(value_type&& val)
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
    iterator emplace(Args&&... args)
    {
        this->emplace_equal(tiny_stl::forward<Args>(args)...);
    }

    void swap(unordered_multiset& rhs)
    {
        _Base::swap(rhs);
    }
};  // unordered_multiset

template <typename Key, typename Hash, typename KeyEqual, typename Alloc>
void swap(unordered_multiset<Key, Hash, KeyEqual, Alloc>& lhs,
          unordered_multiset<Key, Hash, KeyEqual, Alloc>& rhs)
{
    lhs.swap(rhs);
}

}   // namespace tiny_stl