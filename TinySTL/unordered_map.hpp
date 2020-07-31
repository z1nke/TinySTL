#pragma once

#include "hashtable.hpp"

namespace tiny_stl
{

template <typename Key, typename T,
    typename Hash = hash<Key>,
    typename KeyEqual = equal_to<Key>,
    typename Alloc = allocator<pair<Key, T>>>
class unordered_map : public HashTable<pair<Key, T>, Hash, KeyEqual, Alloc, true>
{
public:
    using key_type              = Key;
    using mapped_type           = T;
    using value_type            = pair<const Key, T>;
    using size_type             = typename Alloc::size_type;
    using difference_type       = typename Alloc::difference_type;
    using hasher                = Hash;
    using key_equal             = KeyEqual;
    using allocator_type        = Alloc;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using AlTraits              = allocator_traits<allocator_type>;
    using pointer               = typename AlTraits::pointer;
    using const_pointer         = typename AlTraits::const_pointer;
    using Base                  = HashTable<pair<Key, T>, Hash, KeyEqual, Alloc, true>;
    using iterator              = typename Base::iterator;
    using const_iterator        = typename Base::const_iterator;
    using local_iterator        = typename Base::local_iterator;
    using const_local_iterator  = typename Base::const_local_iterator;

    
public:
    unordered_map() : unordered_map(0) { }                          // (1)
    explicit unordered_map(size_type num_bucket,
                           const Hash& hashfunc = Hash(),
                           const KeyEqual& eq = KeyEqual(),
                           const Alloc& alloc = Alloc()) 
    : Base(num_bucket, alloc, hashfunc, eq) { }

    unordered_map(size_type num_bucket, const Alloc& alloc)         // (1)
    : unordered_map(num_bucket, Hash(), KeyEqual(), alloc) { }

    unordered_map(size_type num_bucket,                             // (1)
                  const Hash& hashfunc, const Alloc& alloc)
    : unordered_map(num_bucket, hashfunc, KeyEqual(), alloc) { }

    explicit unordered_map(const Alloc& alloc)                      // (1)
    : Base(0, alloc) { }

    template <typename InIter>                                      // (2)
    unordered_map(InIter first, InIter last,
                  size_type num_bucket = 0,
                  const Hash& hashfunc = Hash(),
                  const KeyEqual& eq = KeyEqual(),
                  const Alloc& alloc = Alloc())
    : Base(num_bucket, alloc, hashfunc, eq)
    {
        this->insert_unique(first, last);
    }

    template <typename InIter>                                      // (2)
    unordered_map(InIter first, InIter last,
                  size_type num_bucket,
                  const Alloc& alloc)
    : unordered_map(first, last, num_bucket, 
                    Hash(), KeyEqual(), alloc) { }

    template <typename InIter>                                      // (2)
    unordered_map(InIter first, InIter last,
                  size_type num_bucket,
                  const Hash& hashfunc,
                  const Alloc& alloc)
    : unordered_map(first, last, num_bucket, 
                    hashfunc, KeyEqual(), alloc) { }


    unordered_map(const unordered_map& rhs)                         // (3)
    : Base(rhs) { }

    unordered_map(const unordered_map& rhs,                         // (3)
        const Alloc& alloc) : Base(rhs, alloc) { }

    unordered_map(unordered_map&& rhs) noexcept                     // (4)
    : Base(tiny_stl::move(rhs)) { }

    unordered_map(std::initializer_list<value_type> ilist,          // (5)
                  size_type num_bucket = 0,
                  const Hash& hashfunc = Hash(),
                  const KeyEqual& eq = KeyEqual(),
                  const Alloc& alloc = Alloc())
    : unordered_map(ilist.begin(), ilist.end(), 
                    num_bucket, hashfunc, eq, alloc) { }

    unordered_map(std::initializer_list<value_type> ilist,          // (5)
                  size_type num_bucket, const Alloc& alloc)
    : unordered_map(ilist, num_bucket,
                    Hash(), KeyEqual(), alloc) { }

    unordered_map(std::initializer_list<value_type> ilist,          // (5)
                  size_type num_bucket, const Hash& hashfunc,
                  const Alloc& alloc)
    : unordered_map(ilist, num_bucket,
                    hashfunc, KeyEqual(), alloc) { }

    unordered_map& operator=(const unordered_map& rhs)
    {
        Base::operator=(rhs);
        return *this;
    }

    unordered_map& operator=(unordered_map&& rhs)
    {
        Base::operator=(tiny_stl::move(rhs));
        return *this;
    }

    unordered_map& operator=(std::initializer_list<value_type> ilist)
    {
        this->clear();
        this->insert(ilist);
        return *this;
    }

    mapped_type& at(const key_type& key)
    {
        iterator pos = this->find(key);
        if (pos == this->end())
            throw "unordered_map: out of range";

        return pos->second;
    }

    const mapped_type& at(const key_type& key) const
    {
        const_iterator pos = this->find(key);
        if (pos == this->end())
            throw "unordered_map: out of range";

        return pos->second;
    }

    T& operator[](const key_type& key)
    {
        iterator pos = this->find(key);
        if (pos == this->end())
            return this->insert(make_pair(key, T{})).first->second;

        return pos->second;
    }


    T& operator[](key_type&& key)
    {
        iterator pos = this->find(key);
        if (pos == this->end())
            return this->insert(make_pair(tiny_stl::move(key), T{})).first->second;

        return pos->second;
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

    void swap(unordered_map& rhs)
    {
        Base::swap(rhs);
    }
};  // unordered_map

template <typename Key, typename T, typename Hash, 
          typename KeyEqual, typename Alloc>
void swap(unordered_map<Key, T, Hash, KeyEqual, Alloc>& lhs,
          unordered_map<Key, T, Hash, KeyEqual, Alloc>& rhs)
{
    lhs.swap(rhs);
}


template <typename Key, typename T,
    typename Hash = hash<Key>,
    typename KeyEqual = equal_to<Key>,
    typename Alloc = allocator<pair<Key, T>>>
class unordered_multimap 
    : public HashTable<pair<Key, T>, Hash, KeyEqual, Alloc, true>
{
public:
    using key_type              = Key;
    using mapped_type           = T;
    using value_type            = pair<const Key, T>;
    using size_type             = size_t;
    using difference_type       = ptrdiff_t;
    using hasher                = Hash;
    using key_equal             = KeyEqual;
    using allocator_type        = Alloc;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using AlTraits              = allocator_traits<allocator_type>;
    using pointer               = typename AlTraits::pointer;
    using const_pointer         = typename AlTraits::const_pointer;
    using Base                  = HashTable<pair<Key, T>, Hash, KeyEqual, Alloc, true>;
    using iterator              = typename Base::iterator;
    using const_iterator        = typename Base::const_iterator;
    using local_iterator        = typename Base::local_iterator;
    using const_local_iterator  = typename Base::const_local_iterator;

    
public:
    unordered_multimap() : unordered_multimap(0) { }                    // (1)
    explicit unordered_multimap(size_type num_bucket,
                                const Hash& hashfunc = Hash(),
                                const KeyEqual& eq = KeyEqual(),
                                const Alloc& alloc = Alloc()) 
    : Base(num_bucket, alloc, hashfunc, eq) { }

    unordered_multimap(size_type num_bucket, const Alloc& alloc)        // (1)
    : unordered_multimap(num_bucket, Hash(), KeyEqual(), alloc) { }

    unordered_multimap(size_type num_bucket,                            // (1)
                       const Hash& hashfunc, const Alloc& alloc)
    : unordered_multimap(num_bucket, hashfunc, KeyEqual(), alloc) { }

    explicit unordered_multimap(const Alloc& alloc)                     // (1)
    : Base(0, alloc) { }

    template <typename InIter>                                          // (2)
    unordered_multimap(InIter first, InIter last,
                       size_type num_bucket = 0,
                       const Hash& hashfunc = Hash(),
                       const KeyEqual& eq = KeyEqual(),
                       const Alloc& alloc = Alloc())
    : Base(num_bucket, alloc, hashfunc, eq)
    {
        this->insert_equal(first, last);
    }

    template <typename InIter>                                          // (2)
    unordered_multimap(InIter first, InIter last,
                       size_type num_bucket,
                       const Alloc& alloc)
    : unordered_multimap(first, last, num_bucket, 
                         Hash(), KeyEqual(), alloc) { }

    template <typename InIter>                                          // (2)
    unordered_multimap(InIter first, InIter last,
                       size_type num_bucket,
                       const Hash& hashfunc,
                       const Alloc& alloc)
    : unordered_multimap(first, last, num_bucket, 
                         hashfunc, KeyEqual(), alloc) { }


    unordered_multimap(const unordered_multimap& rhs)                   // (3)
    : Base(rhs) { }

    unordered_multimap(const unordered_multimap& rhs,                   // (3)
        const Alloc& alloc) : Base(rhs, alloc) { }

    unordered_multimap(unordered_multimap&& rhs)                        // (4)
    : Base(tiny_stl::move(rhs)) { }

    unordered_multimap(std::initializer_list<value_type> ilist,         // (5)
                       size_type num_bucket = 0,
                       const Hash& hashfunc = Hash(),
                       const KeyEqual& eq = KeyEqual(),
                       const Alloc& alloc = Alloc())
    : unordered_multimap(ilist.begin(), ilist.end(), 
                         num_bucket, hashfunc, eq, alloc) { }

    unordered_multimap(std::initializer_list<value_type> ilist,         // (5)
                       size_type num_bucket, const Alloc& alloc)
    : unordered_multimap(ilist, num_bucket,
                         Hash(), KeyEqual(), alloc) { }

    unordered_multimap(std::initializer_list<value_type> ilist,         // (5)
                       size_type num_bucket, const Hash& hashfunc,
                       const Alloc& alloc)
    : unordered_multimap(ilist, num_bucket,
                         hashfunc, KeyEqual(), alloc) { }

    unordered_multimap& operator=(const unordered_multimap& rhs)
    {
        Base::operator=(rhs);
        return *this;
    }

    unordered_multimap& operator=(unordered_multimap&& rhs)
    {
        Base::operator=(tiny_stl::move(rhs));
        return *this;
    }

    unordered_multimap& operator=(std::initializer_list<value_type> ilist)
    {
        this->clear();
        this->insert(ilist);
        return *this;
    }

    size_type count(const key_type& key) const
    {
        return this->count_equal(key);
    }

    pair<iterator, bool> insert(const value_type& val)
    {
        return this->insert_equal(val);
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
        this->emplace_equal(tiny_stl::forward<Args>(args)...);
    }

    void swap(unordered_multimap& rhs)
    {
        Base::swap(rhs);
    }
};  // unordered_multimap

template <typename Key, typename T, typename Hash, typename KeyEqual, typename Alloc>
void swap(unordered_multimap<Key, T, Hash, KeyEqual, Alloc>& lhs,
          unordered_multimap<Key, T, Hash, KeyEqual, Alloc>& rhs)
{
    lhs.swap(rhs);
}



}   // namespace tiny_stl

