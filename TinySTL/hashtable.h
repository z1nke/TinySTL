#pragma once

#include "forward_list.h"
#include "vector.h"

namespace tiny_stl
{

template <typename T, typename HashTableType>
struct HashIterator;

template <typename T, typename HashTableType>
struct HashConstIterator
{
    using iterator_category = forward_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using reference         = const T&;
    using pointer           = const T*;

    size_t idx_bucket;
    FListConstIterator<T> iter;
    HashTableType* hashtable;
    
    HashConstIterator() = default;
    HashConstIterator(size_t idx, FListConstIterator<T> it, HashTableType* ht)
    : idx_bucket(idx), iter(it), hashtable(ht) { }

    HashConstIterator(const HashConstIterator&) = default;

    HashConstIterator(const HashIterator<T, remove_const_t<HashTableType>>& rhs)
    : idx_bucket(rhs.idx_bucket), iter(rhs.iter.ptr), 
        hashtable(static_cast<HashTableType*>(rhs.hashtable)) { }

    reference operator*() const
    {
        return *iter;
    }

    pointer operator->() const
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    HashConstIterator& operator++()
    {
        ++iter;
        if (iter == hashtable->buckets[idx_bucket].end())   // jump to the next 
        {                                                   // bucket with element(s)
            for (;;)
            {
                if (idx_bucket == hashtable->buckets.size() - 1)
                {
                    iter = hashtable->buckets[idx_bucket].end();
                    break;
                }
                else
                {
                    ++idx_bucket;
                    if (!hashtable->buckets[idx_bucket].empty())
                    {
                        iter = hashtable->buckets[idx_bucket].begin();
                        break;
                    }
                }
            }
        }

        return *this;
    }

    HashConstIterator operator++(int)
    {
        HashConstIterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const HashConstIterator& rhs) const
    {
        return iter == rhs.iter;
    }

    bool operator!=(const HashConstIterator& rhs) const
    {
        return iter != rhs.iter;
    }
};

template <typename T, typename HashTableType>
struct HashIterator 
{
    using iterator_category = forward_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using reference         = T&;
    using pointer           = T*;

    size_t idx_bucket;
    FListIterator<T> iter;
    HashTableType* hashtable;

    HashIterator() = default;
    HashIterator(size_t idx, FListIterator<T> it, HashTableType* ht)
    : idx_bucket(idx), iter(it), hashtable(ht) { }

    reference operator*() const
    {
        return *iter;
    }

    pointer operator->() const
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    HashIterator& operator++()
    {
        ++iter;
        if (iter == hashtable->buckets[idx_bucket].end())   
        {                                                 
            for (;;)
            {
                if (idx_bucket == hashtable->buckets.size() - 1)
                {
                    iter = hashtable->buckets[idx_bucket].end();
                    break;
                }
                else
                {
                    ++idx_bucket;
                    if (!hashtable->buckets[idx_bucket].empty())
                    {
                        iter = hashtable->buckets[idx_bucket].begin();
                        break;
                    }
                }
            }
        }

        return *this;
    }

    HashIterator operator++(int)
    {
        HashIterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const HashIterator& rhs) const
    {
        return iter == rhs.iter;
    }

    bool operator!=(const HashIterator& rhs) const
    {
        return iter != rhs.iter;
    }
};

// FIXME: write a helper class for prime
static const int stlPrimesSize = 28;
static const size_t stlPrimesArray[stlPrimesSize] =
{
    53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 
    49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 
    6291469, 12582917, 25165843, 50331653, 100663319, 201326611,
    402653189, 805306457, 1610612741, 3221225473U, 4294967291U
};

inline size_t stlNextPrime(size_t n)
{
    const size_t* first = stlPrimesArray;
    const size_t* last = stlPrimesArray + stlPrimesSize;
    const size_t* pos = lower_bound(first, last, n);

    return pos == last ? *(last - 1) : *pos;
}

template <typename T, typename Hash, typename KeyEqual, 
          typename Alloc, bool isMap>
class HashTable
{
    friend HashConstIterator<T, const HashTable>;
    friend HashIterator<T, HashTable>;
public:
    using key_type              = typename AssociatedTypeHelper<T, isMap>::key_type;
    using mapped_type           = typename AssociatedTypeHelper<T, isMap>::mapped_type;
    using value_type            = T;
    using size_type             = size_t;
    using difference_type       = ptrdiff_t;
    using hasher                = Hash;
    using key_equal             = KeyEqual;
    using allocator_type        = Alloc;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using AlTraits              = allocator_traits<Alloc>;
    using List                  = forward_list<T>;
    using pointer               = typename AlTraits::pointer;
    using const_pointer         = typename AlTraits::const_pointer;
    
    using iterator              = HashIterator<T, HashTable>;
    using const_iterator        = HashConstIterator<T, const HashTable>;

    using local_iterator        = typename forward_list<T>::iterator;
    using const_local_iterator  = typename forward_list<T>::const_iterator;

    using AlFlist               
        = typename allocator_traits<Alloc>::template rebind_alloc<forward_list<T>>;
    using Bucket                = vector<forward_list<T>, AlFlist>;
    using Self                  = HashTable<T, Hash, KeyEqual, Alloc, isMap>;
public:
    Bucket    buckets;
    size_type num_elements;
    float     maxfactor;
    hasher    hashfunc;
    key_equal key_equ;
private:
    const key_type& getKey(const T& val, true_type) const    // map
    {
        return val.first;
    }

    const key_type& getKey(const T& val, false_type) const   // set
    {
        return val;
    }

    const key_type& get_key(const T& val) const
    {
        return getKey(val, tiny_stl::bool_constant<isMap>{});
    }

    size_type getNthBucket(const T& val) const
    {
        return hashfunc(get_key(val)) % buckets.size();
    }

    size_type getNthBucketK(const key_type& key) const
    {
        return hashfunc(key) % buckets.size();
    }

    void init(size_type n)
    {
        const size_type num_bucket = stlNextPrime(n);
        buckets.reserve(num_bucket);
        buckets.assign(num_bucket, List{});
        maxfactor = 1.0f;
        num_elements = 0;
    }

    void copyAux(const HashTable& rhs)
    {
        buckets = rhs.buckets;
    }

    size_type updateNextIter(const_local_iterator& iter, size_type idx) const
    {
        size_type iterIdx = idx;
        if (iter == buckets[iterIdx].end())
        {
            for (;;)
            {
                if (iterIdx == bucket_count() - 1)
                {
                    iter = buckets[iterIdx].end();
                    break;
                }
                else
                {
                    ++iterIdx;
                    if (!buckets[iterIdx].empty())
                    {
                        iter = buckets[iterIdx].begin();
                        break;
                    }
                }
            }
        }

        return iterIdx;
    }

    size_type updateNextIter(local_iterator& iter, size_type idx)
    {
        return static_cast<const Self&>(*this).updateNextIter(iter, idx);
    }

public:
    HashTable(size_type n,
        const Alloc& al = Alloc(),
        const hasher& hf = hasher(), 
        const key_equal& equ = key_equal())
    : hashfunc(hf), key_equ(equ),
        buckets(static_cast<AlFlist>(al))
    {
        init(n);
    }

    HashTable(const HashTable&) = default;

    HashTable(const HashTable& rhs, const Alloc& alloc)
    : hashfunc(rhs.hashfunc), key_equ(rhs.key_equ),
        buckets(static_cast<AlFlist>(alloc)),
        maxfactor(rhs.maxfactor), num_elements(rhs.num_elements)
    {
        copyAux(rhs); 
    }

    HashTable(HashTable&& rhs) noexcept
    : hashfunc(rhs.hashfunc), key_equ(rhs.key_equ),
        buckets(tiny_stl::move(rhs.buckets)),
        maxfactor(rhs.maxfactor), num_elements(rhs.num_elements)
    {
        rhs.num_elements = 0;
    }

    HashTable& operator=(const HashTable& rhs)
    {
        assert(this != tiny_stl::addressof(rhs));
        buckets = rhs.buckets;
        num_elements = rhs.num_elements;
        hashfunc = rhs.hashfunc;
        key_equ = rhs.key_equ;
        maxfactor = rhs.maxfactor;

        return *this;
    }

    HashTable& operator=(HashTable&& rhs)
    {
        assert(this != tiny_stl::addressof(rhs));
        buckets = tiny_stl::move(rhs.buckets);
        num_elements = rhs.num_elements;
        hashfunc = rhs.hashfunc;
        key_equ = rhs.key_equ;
        maxfactor = rhs.maxfactor;
        rhs.num_elements = 0;

        return *this;
    }

    ~HashTable() noexcept = default;

    allocator_type get_allocator() const 
    {
        return allocator_type{};
    }

    iterator begin() noexcept
    {
        size_type idx = 0;
        for (; idx != buckets.size(); ++idx)
        {
            if (!buckets[idx].empty())
                break;
        }
        if (idx == buckets.size())
            return end();

        return iterator(idx, begin(idx), this);
    }

    const_iterator begin() const noexcept
    {
        size_type idx = 0;
        for (; idx != buckets.size(); ++idx)
        {
            if (!buckets[idx].empty())
                break;
        }
        if (idx == buckets.size())
            return end();

        return const_iterator(idx, begin(idx), this);
    }

    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    iterator end() noexcept
    {
        size_type idx = bucket_count() - 1;
        return iterator(idx, buckets[idx].end(), this);
    }

    const_iterator end() const noexcept
    {
        size_type idx = bucket_count() - 1;
        return const_iterator(idx, buckets[idx].end(), this);
    }

    const_iterator cend() const noexcept
    {
        return end();
    }

    size_type size() const noexcept
    {
        return num_elements;
    }

    bool empty() const noexcept
    {
        return num_elements == 0;
    }

    size_type max_size() const noexcept
    {
        return static_cast<size_t>(-1);
    }

    void clear() noexcept
    {
        init(0);
    }

private:
    template <typename Value>
    iterator insertEqualAux(Value&& val)
    {
        if (load_factor() > max_load_factor())
            rehash(stlNextPrime(size()));

        ++num_elements;

        size_type idx = getNthBucket(val);

        for (auto cpos = cbegin(idx); cpos != cend(idx); ++cpos)
        {
            if (key_equ(get_key(val), get_key(*cpos)))   // existing
            {
                auto pos = buckets[idx].insert_after(cpos, 
                    tiny_stl::forward<Value>(val));

                return iterator(idx, pos, this);
            }
        }

        // not exist
        auto pos = buckets[idx].insert_after(buckets[idx].before_begin(),
            tiny_stl::forward<Value>(val));
        return iterator(idx, pos, this);
    }

    template <typename Value>
    pair<iterator, bool> insertUniqueAux(Value&& val)
    {        
        size_type idx = getNthBucket(val);

        for (auto cpos = cbegin(idx); cpos != cend(idx); ++cpos)
        {
            if (key_equ(get_key(val), get_key(*cpos)))   // existing
            {
                return make_pair(iterator(idx, buckets[idx].makeIter(cpos), this), 
                                false);
            }
        }

        // not exist
        ++num_elements;
        if (load_factor() > max_load_factor())
        {
            rehash(stlNextPrime(size()));
            idx = getNthBucket(val);
        }
        
        auto pos = buckets[idx].insert_after(buckets[idx].before_begin(),
            tiny_stl::forward<Value>(val));

        return make_pair(iterator(idx, pos, this), true);
    }

protected:
    iterator insert_equal(const value_type& val)
    {
        return insertEqualAux(val);
    }

    iterator insert_equal(value_type&& val)
    {
        return insertEqualAux(tiny_stl::move(val));
    }

    template <typename InIter>
    void insert_equal(InIter first, InIter last)
    {
        for (; first != last; ++first)
            insertEqualAux(*first);
    }

    pair<iterator, bool> insert_unique(const value_type& val)
    {
        return insertUniqueAux(val);
    }

    pair<iterator, bool> insert_unique(value_type&& val)
    {
        return insertUniqueAux(tiny_stl::move(val));
    }

    template <typename InIter>
    void insert_unique(InIter first, InIter last)
    {
        for (; first != last; ++first)
            insertUniqueAux(*first);
    }

    template <typename... Args>
    iterator emplace_equal(Args&&... args)
    {
        T val(tiny_stl::forward<Args>(args)...);
        return insertEqualAux(tiny_stl::move(val));
    }

    template <typename... Args>
    pair<iterator, bool> emplace_unique(Args&&... args)
    {
        T val(tiny_stl::forward<Args>(args)...);
        return insertUniqueAux(tiny_stl::move(val));
    }

    size_type count_equal(const key_type& key) const
    {
        auto range = equal_range(key);
        return distance(range.first, range.second);
    }

    size_type count_unique(const key_type& key) const
    {
        auto iter = find(key);
        return iter == end() ? 0 : 1;
    }

public:
    iterator erase(const_iterator pos)
    {
        assert(pos != cend());
        --num_elements;
        
        size_type idx = getNthBucket(*pos);
        auto prev  = buckets[idx].cbefore_begin();         // pos prev
        auto next = prev;
        for (++next; next != pos.iter; ++next, ++prev); // find pos prev

        auto iter = buckets[idx].erase_after(prev);        // erase pos

        updateNextIter(iter, idx);

        return iterator(idx, iter, this);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        if (first == begin() && last == end())
        {
            clear();
            return end();
        }

        while (first != last)
            erase(first++);
       
        return iterator(first.idx_bucket,
            buckets[first.idx_bucket].makeIter(first.iter),
            this);
    }

    size_type erase(const key_type& key)
    {
        auto range = static_cast<const Self*>(this)->equal_range(key);
        size_type num = distance(range.first, range.second);

        erase(range.first, range.second);

        return num;
    }

    void swap(HashTable& rhs)
    {
        swapADL(hashfunc, rhs.hashfunc);
        swapADL(key_equ, rhs.key_equ);
        swapADL(maxfactor, rhs.maxfactor);
        swapADL(num_elements, rhs.num_elements);
        buckets.swap(rhs.buckets);
    }
public:
    iterator find(const key_type& key)
    {
        size_type idx = getNthBucketK(key);        
        for (auto pos = begin(idx); pos != end(idx); ++pos)
        {
            if (key_equ(get_key(*pos), key))
                return iterator(idx, pos, this);
        }

        return end();
    }

    const_iterator find(const key_type& key) const
    {
        size_type idx = getNthBucketK(key);
        for (auto pos = begin(idx); pos != end(idx); ++pos)
        {
            if (key_equ(*pos, key))
                return const_iterator(idx, pos, this);
        }

        return end();
    }

    pair<iterator, iterator> equal_range(const key_type& key)
    {
        size_type idx = getNthBucketK(key);

        auto first = begin(idx);
        for (; first != end(idx); ++first)
        {
            if (key_equ(*first, key))       // find first
                break;
        }

        if (first == end(idx))
            return make_pair(end(), end());

        auto last = first;

        for (++last; last != end(idx); ++last)
        {
            if (!key_equ(*last, key))       // find last
                break;
        }

        size_type lastIdx = updateNextIter(last, idx);

        return make_pair(iterator(idx, first, this), 
            iterator(lastIdx, last, this));
    }

    pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        size_type idx = getNthBucketK(key);

        auto first = begin(idx);
        for (; first != end(idx); ++first)
        {
            if (key_equ(*first, key))       // find first
                break;
        }

        if (first == end(idx))
            return make_pair(end(), end());

        auto last = first;

        for (++last; last != end(idx); ++last)
        {
            if (!key_equ(*last, key))       // find last
                break;
        }

        size_type lastIdx = updateNextIter(last, idx);

        return make_pair(const_iterator(idx, first, this),
            const_iterator(lastIdx, last, this));
    }

public:
    local_iterator begin(size_type n)
    {        
        assert(n < buckets.size());
        return buckets[n].begin();
    }

    const_local_iterator begin(size_type n) const
    {
        assert(n < buckets.size());
        return buckets[n].begin();
    }

    const_local_iterator cbegin(size_type n) const
    {
        return begin(n);
    }

    local_iterator end(size_type n)
    {
        assert(n < buckets.size());
        return buckets[n].end();
    }

    const_local_iterator end(size_type n) const
    {
        assert(n < buckets.size());
        return buckets[n].end();
    }

    const_local_iterator cend(size_type n) const
    {
        return end(n);
    }

    size_type bucket_count() const noexcept
    {
        return buckets.size();
    }

    size_type max_bucket_count() const noexcept
    {
        return stlPrimesArray[stlPrimesSize - 1] >> 2;
    }

    size_type bucket_size(size_type n) const
    {
        const_local_iterator first = begin(n);
        const_local_iterator last = end(n);

        return tiny_stl::distance(first, last);
    }

    size_type bucket(const key_type& key) const
    {
        assert(bucket_count() != 0);
        return getNthBucketK(key);
    }

    float load_factor() const
    {
        return static_cast<float>(size()) 
            / static_cast<float>(bucket_count());
    }

    float max_load_factor() const
    {
        return maxfactor;
    }

    void max_load_factor(float mlf)
    {
        if (mlf == mlf || mlf > 0.0)
            maxfactor = mlf;
    }

    void rehash(size_type n)
    {
        if (n <= size() / max_load_factor())
            return;
        HashTable tmp(stlNextPrime(n));
        for (const auto& val : *this)
            tmp.insert_equal(val);
        this->swap(tmp);
    }

    void reserve(size_type n)
    {
        rehash(std::ceil(n / max_load_factor()));
    }

    hasher hash_function() const
    {
        return hashfunc;
    }

    key_equal key_eq() const
    {
        return key_equ;
    }
};  // HashTable


template <typename T, typename Hash, typename KeyEqual, typename Alloc, bool isMap>
inline bool operator==(const HashTable<T, Hash, KeyEqual, Alloc, isMap>& lhs,
                       const HashTable<T, Hash, KeyEqual, Alloc, isMap>& rhs)
{
    return lhs.size() == rhs.size()
        && equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Hash, typename KeyEqual, typename Alloc, bool isMap>
inline bool operator!=(const HashTable<T, Hash, KeyEqual, Alloc, isMap>& lhs,
                       const HashTable<T, Hash, KeyEqual, Alloc, isMap>& rhs)
{
    return !(lhs == rhs);
}

template <typename T, typename Hash, typename KeyEqual, typename Alloc, bool isMap>
inline bool operator==(HashTable<T, Hash, KeyEqual, Alloc, isMap>& lhs,
                       HashTable<T, Hash, KeyEqual, Alloc, isMap>& rhs)
{
    return lhs.swap(rhs);
}

}   // namespace tiny_stl
