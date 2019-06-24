#pragma once

#include "forward_list.h"
#include "vector.h"

namespace tiny_stl
{

template <typename T, typename HashTableType>
struct _Hash_iterator;

template <typename T, typename HashTableType>
struct _Hash_const_iterator
{
    using iterator_category = forward_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using reference         = const T&;
    using pointer           = const T*;

    size_t idx_bucket;
    _FList_const_iterator<T> iter;
    HashTableType* hashtable;
    
    _Hash_const_iterator() = default;
    _Hash_const_iterator(size_t idx, _FList_const_iterator<T> it, HashTableType* ht)
    : idx_bucket(idx), iter(it), hashtable(ht) { }

    _Hash_const_iterator(const _Hash_const_iterator&) = default;

    _Hash_const_iterator(const _Hash_iterator<T, remove_const_t<HashTableType>>& rhs)
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

    _Hash_const_iterator& operator++()
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

    _Hash_const_iterator operator++(int)
    {
        _Hash_const_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const _Hash_const_iterator& rhs) const
    {
        return iter == rhs.iter;
    }

    bool operator!=(const _Hash_const_iterator& rhs) const
    {
        return iter != rhs.iter;
    }
};

template <typename T, typename HashTableType>
struct _Hash_iterator 
{
    using iterator_category = forward_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using reference         = T&;
    using pointer           = T*;

    size_t idx_bucket;
    _FList_iterator<T> iter;
    HashTableType* hashtable;

    _Hash_iterator() = default;
    _Hash_iterator(size_t idx, _FList_iterator<T> it, HashTableType* ht)
    : idx_bucket(idx), iter(it), hashtable(ht) { }

    reference operator*() const
    {
        return *iter;
    }

    pointer operator->() const
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _Hash_iterator& operator++()
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

    _Hash_iterator operator++(int)
    {
        _Hash_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator==(const _Hash_iterator& rhs) const
    {
        return iter == rhs.iter;
    }

    bool operator!=(const _Hash_iterator& rhs) const
    {
        return iter != rhs.iter;
    }
};

static const int _stl_primes_size = 28;
static const size_t _stl_primes_array[_stl_primes_size] =
{
    53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 
    49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 
    6291469, 12582917, 25165843, 50331653, 100663319, 201326611,
    402653189, 805306457, 1610612741, 3221225473U, 4294967291U
};

inline size_t _stl_next_prime(size_t n)
{
    const size_t* first = _stl_primes_array;
    const size_t* last = _stl_primes_array + _stl_primes_size;
    const size_t* pos = lower_bound(first, last, n);

    return pos == last ? *(last - 1) : *pos;
}

template <typename T, typename Hash, typename KeyEqual, typename Alloc, bool is_map>
class _HashTable
{
    friend _Hash_const_iterator<T, const _HashTable>;
    friend _Hash_iterator<T, _HashTable>;
public:
    using key_type              = typename _Asso_type_helper<T, is_map>::key_type;
    using mapped_type           = typename _Asso_type_helper<T, is_map>::mapped_type;
    using value_type            = T;
    using size_type             = size_t;
    using difference_type       = ptrdiff_t;
    using hasher                = Hash;
    using key_equal             = KeyEqual;
    using allocator_type        = Alloc;
    using reference             = value_type&;
    using const_reference       = const value_type&;
    using _Al_traits            = allocator_traits<Alloc>;
    using _List                 = forward_list<T>;
    using pointer               = typename _Al_traits::pointer;
    using const_pointer         = typename _Al_traits::const_pointer;
    
    using iterator              = _Hash_iterator<T, _HashTable>;
    using const_iterator        = _Hash_const_iterator<T, const _HashTable>;

    using local_iterator        = typename forward_list<T>::iterator;
    using const_local_iterator  = typename forward_list<T>::const_iterator;

    using _Al_flist             = typename allocator_traits<Alloc>::template rebind_alloc<forward_list<T>>;
    using _Bucket               = vector<forward_list<T>, _Al_flist>;
    using _Self                 = _HashTable<T, Hash, KeyEqual, Alloc, is_map>;
public:
    _Bucket   buckets;
    size_type num_elements;
    float     maxfactor;
    hasher    hashfunc;
    key_equal key_equ;
private:
    const key_type& _Get_key(const T& val, true_type) const    // map
    {
        return val.first;
    }

    const key_type& _Get_key(const T& val, false_type) const   // set
    {
        return val;
    }

    const key_type& get_key(const T& val) const
    {
        return _Get_key(val, tiny_stl::bool_constant<is_map>{});
    }

    size_type _Get_nth_bucket(const T& val) const
    {
        return hashfunc(get_key(val)) % buckets.size();
    }

    size_type _Get_nth_bucket_k(const key_type& key) const
    {
        return hashfunc(key) % buckets.size();
    }

    void _Init(size_type n)
    {
        const size_type num_bucket = _stl_next_prime(n);
        buckets.reserve(num_bucket);
        buckets.assign(num_bucket, _List{});
        maxfactor = 1.0f;
        num_elements = 0;
    }

    void _Copy(const _HashTable& rhs)
    {
        buckets = rhs.buckets;
    }

    size_type _Update_next_iter(const_local_iterator& iter, size_type idx) const
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

    size_type _Update_next_iter(local_iterator& iter, size_type idx)
    {
        return static_cast<const _Self&>(*this)._Update_next_iter(iter, idx);
    }

public:
    _HashTable(size_type n,
        const Alloc& al = Alloc(),
        const hasher& hf = hasher(), 
        const key_equal& equ = key_equal())
    : hashfunc(hf), key_equ(equ),
        buckets(static_cast<_Al_flist>(al))
    {
        _Init(n);
    }

    _HashTable(const _HashTable&) = default;

    _HashTable(const _HashTable& rhs, const Alloc& alloc)
    : hashfunc(rhs.hashfunc), key_equ(rhs.key_equ),
        buckets(static_cast<_Al_flist>(alloc)),
        maxfactor(rhs.maxfactor), num_elements(rhs.num_elements)
    {
        _Copy(rhs); 
    }

    _HashTable(_HashTable&& rhs) 
    : hashfunc(rhs.hashfunc), key_equ(rhs.key_equ),
        buckets(tiny_stl::move(rhs.buckets)),
        maxfactor(rhs.maxfactor), num_elements(rhs.num_elements)
    {
        rhs.num_elements = 0;
    }

    _HashTable& operator=(const _HashTable& rhs)
    {
        assert(this != tiny_stl::addressof(rhs));
        buckets = rhs.buckets;
        num_elements = rhs.num_elements;
        hashfunc = rhs.hashfunc;
        key_equ = rhs.key_equ;
        maxfactor = rhs.maxfactor;

        return *this;
    }

    _HashTable& operator=(_HashTable&& rhs)
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

    ~_HashTable() noexcept = default;

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
        _Init(0);
    }

private:
    template <typename Value>
    iterator _Insert_equal(Value&& val)
    {
        if (load_factor() > max_load_factor())
            rehash(_stl_next_prime(size()));

        ++num_elements;

        size_type idx = _Get_nth_bucket(val);

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
    pair<iterator, bool> _Insert_unique(Value&& val)
    {        
        size_type idx = _Get_nth_bucket(val);

        for (auto cpos = cbegin(idx); cpos != cend(idx); ++cpos)
        {
            if (key_equ(get_key(val), get_key(*cpos)))   // existing
            {
                return make_pair(iterator(idx, buckets[idx]._Make_iter(cpos), this), 
                                false);
            }
        }

        // not exist
        ++num_elements;
        if (load_factor() > max_load_factor())
        {
            rehash(_stl_next_prime(size()));
            idx = _Get_nth_bucket(val);
        }
        
        auto pos = buckets[idx].insert_after(buckets[idx].before_begin(),
            tiny_stl::forward<Value>(val));

        return make_pair(iterator(idx, pos, this), true);
    }

protected:
    iterator insert_equal(const value_type& val)
    {
        return _Insert_equal(val);
    }

    iterator insert_equal(value_type&& val)
    {
        return _Insert_equal(tiny_stl::move(val));
    }

    template <typename InIter>
    void insert_equal(InIter first, InIter last)
    {
        for (; first != last; ++first)
            _Insert_equal(*first);
    }

    pair<iterator, bool> insert_unique(const value_type& val)
    {
        return _Insert_unique(val);
    }

    pair<iterator, bool> insert_unique(value_type&& val)
    {
        return _Insert_unique(tiny_stl::move(val));
    }

    template <typename InIter>
    void insert_unique(InIter first, InIter last)
    {
        for (; first != last; ++first)
            _Insert_unique(*first);
    }

    template <typename... Args>
    iterator emplace_equal(Args&&... args)
    {
        T val(tiny_stl::forward<Args>(args)...);
        return _Insert_equal(tiny_stl::move(val));
    }

    template <typename... Args>
    pair<iterator, bool> emplace_unique(Args&&... args)
    {
        T val(tiny_stl::forward<Args>(args)...);
        return _Insert_unique(tiny_stl::move(val));
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
        
        size_type idx = _Get_nth_bucket(*pos);
        auto _prev  = buckets[idx].cbefore_begin();         // pos prev
        auto _next = _prev;
        for (++_next; _next != pos.iter; ++_next, ++_prev); // find pos prev

        auto iter = buckets[idx].erase_after(_prev);        // erase pos

        _Update_next_iter(iter, idx);

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
            buckets[first.idx_bucket]._Make_iter(first.iter),
            this);
    }

    size_type erase(const key_type& key)
    {
        auto range = static_cast<const _Self*>(this)->equal_range(key);
        size_type num = distance(range.first, range.second);

        erase(range.first, range.second);

        return num;
    }

    void swap(_HashTable& rhs)
    {
        _Swap_ADL(hashfunc, rhs.hashfunc);
        _Swap_ADL(key_equ, rhs.key_equ);
        _Swap_ADL(maxfactor, rhs.maxfactor);
        _Swap_ADL(num_elements, rhs.num_elements);
        buckets.swap(rhs.buckets);
    }
public:
    iterator find(const key_type& key)
    {
        size_type idx = _Get_nth_bucket_k(key);        
        for (auto pos = begin(idx); pos != end(idx); ++pos)
        {
            if (key_equ(get_key(*pos), key))
                return iterator(idx, pos, this);
        }

        return end();
    }

    const_iterator find(const key_type& key) const
    {
        size_type idx = _Get_nth_bucket_k(key);
        for (auto pos = begin(idx); pos != end(idx); ++pos)
        {
            if (key_equ(*pos, key))
                return const_iterator(idx, pos, this);
        }

        return end();
    }

    pair<iterator, iterator> equal_range(const key_type& key)
    {
        size_type idx = _Get_nth_bucket_k(key);

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

        size_type lastIdx = _Update_next_iter(last, idx);

        return make_pair(iterator(idx, first, this), 
            iterator(lastIdx, last, this));
    }

    pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        size_type idx = _Get_nth_bucket_k(key);

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

        size_type lastIdx = _Update_next_iter(last, idx);

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
        return _stl_primes_array[_stl_primes_size - 1] >> 2;
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
        return _Get_nth_bucket_k(key);
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
        _HashTable tmp(_stl_next_prime(n));
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
};  // _HashTable


template <typename T, typename Hash, typename KeyEqual, typename Alloc, bool is_map>
inline bool operator==(const _HashTable<T, Hash, KeyEqual, Alloc, is_map>& lhs,
                       const _HashTable<T, Hash, KeyEqual, Alloc, is_map>& rhs)
{
    return lhs.size() == rhs.size()
        && equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Hash, typename KeyEqual, typename Alloc, bool is_map>
inline bool operator!=(const _HashTable<T, Hash, KeyEqual, Alloc, is_map>& lhs,
                       const _HashTable<T, Hash, KeyEqual, Alloc, is_map>& rhs)
{
    return !(lhs == rhs);
}

template <typename T, typename Hash, typename KeyEqual, typename Alloc, bool is_map>
inline bool operator==(_HashTable<T, Hash, KeyEqual, Alloc, is_map>& lhs,
                       _HashTable<T, Hash, KeyEqual, Alloc, is_map>& rhs)
{
    return lhs.swap(rhs);
}

}   // namespace tiny_stl