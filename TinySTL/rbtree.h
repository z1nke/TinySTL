#pragma once

#include <initializer_list> // for std::initializer_list

#include "memory.h"

namespace tiny_stl
{

namespace 
{

enum class Color : uint16_t
{
    RED,
    BLACK
};

template <typename T>
struct RBTNode
{
    using IsNil = uint16_t;
    Color color;
    IsNil isNil;            // 1 is nil, 0 is not nil
    RBTNode* parent;
    RBTNode* left;
    RBTNode* right;
    T value;
};

template <typename T>
inline RBTNode<T>* _RBTree_min_value(RBTNode<T>* ptr)
{
    while (!ptr->left->isNil)
        ptr = ptr->left;

    return ptr;
}

template <typename T>
inline RBTNode<T>* _RBTree_max_value(RBTNode<T>* ptr)
{
    while (!ptr->right->isNil)
        ptr = ptr->right;

    return ptr;
}

// 
//        |                                  |
//        x                                  y
//      /   \          left-rotate         /   \
//     a     y      ---------------->     x     c
//         /   \                        /   \
//        b     c                      a     b
// 
template <typename T>
inline void _RBTree_left_rotate(RBTNode<T>*& root, RBTNode<T>* x)
{
    RBTNode<T>* y = x->right;      

    x->right = y->left;                
    if (!y->left->isNil)
        y->left->parent = x;

    y->parent = x->parent;

    if (x->parent->isNil)
        root = y;
    else if (x == x->parent->left)  // x is the left child of x.p
        x->parent->left = y;
    else
        x->parent->right = y;

    x->parent = y;
    y->left = x;
}


//        |                                 |
//        y                                 x
//      /   \         right-rotate        /   \
//     x     c      ---------------->    a     y
//   /   \                                   /   \
//  a     b                                 b     c
//
template <typename T>
inline void _RBTree_right_rotate(RBTNode<T>*& root, RBTNode<T>* y)
{
    RBTNode<T>* x = y->left;

    y->left = x->right;
    if (!x->right->isNil)
        x->right->parent = y;

    x->parent = y->parent;

    if (y->parent->isNil)
        root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;

    x->right = y;
    y->parent = x;
}



}   // unnamed namespace


template <typename T>
struct _RBTree_const_iterator
{
    using iterator_category = bidirectional_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = const T*;
    using reference         = const T&;

    using Ptr               = RBTNode<T>*;

    Ptr ptr;

    _RBTree_const_iterator() : ptr() { }
    _RBTree_const_iterator(Ptr x) : ptr(x) { }
    _RBTree_const_iterator(const _RBTree_const_iterator& rhs) : ptr(rhs.ptr) { }

    reference operator*() const
    {
        return ptr->value;
    }

    pointer operator->() const
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _RBTree_const_iterator& operator++()            
    {
        if (!ptr->right->isNil)
        {
            ptr = ptr->right;
            ptr = _RBTree_min_value(ptr);
        }
        else                                
        {
            Ptr x = ptr->parent;
            while (!x->isNil && ptr == x->right)
            {
                ptr = x;
                x = x->parent;
            }
            ptr = x;
        }

        return *this;
    }

    _RBTree_const_iterator operator++(int)
    {
        _RBTree_const_iterator tmp = *this;
        ++*this;

        return tmp;
    }

    _RBTree_const_iterator& operator--()
    {
        if (ptr->isNil)
            ptr = ptr->right;
        else if (!ptr->left->isNil)
        {
            ptr = ptr->left;
            ptr = _RBTree_max_value(ptr);
        }
        else
        {
            Ptr x = ptr->parent;
            while (!x->isNil && ptr == x->left)
            {
                ptr = x;
                x = x->parent;
            }
            ptr = x;
        }

        return *this;
    }

    _RBTree_const_iterator operator--(int)
    {
        _RBTree_const_iterator tmp = *this;
        --*this;

        return tmp;
    }

    bool operator==(const _RBTree_const_iterator& rhs) const
    {
        return ptr == rhs.ptr;
    }

    bool operator!=(const _RBTree_const_iterator& rhs) const
    {
        return !(ptr == rhs.ptr);
    }
    
};  // _RBTree_const_iterator


template <typename T>
struct _RBTree_iterator : _RBTree_const_iterator<T>
{
    using iterator_category = bidirectional_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;

    using Ptr               = RBTNode<T>*;
    using _Base             = _RBTree_const_iterator<T>;

    _RBTree_iterator() : _Base() { }
    _RBTree_iterator(Ptr x) : _Base(x) { }
    _RBTree_iterator(const _RBTree_iterator& rhs) : _Base(rhs.ptr) { }

    reference operator*() const
    {
        return const_cast<reference>(_Base::operator*());
    }

    pointer operator->() const
    {
        return pointer_traits<pointer>::pointer_to(**this);
    }

    _RBTree_iterator& operator++()
    {
        ++*static_cast<_Base*>(this);
        return *this;
    }

    _RBTree_iterator operator++(int)
    {
        _RBTree_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    _RBTree_iterator& operator--()
    {
        --*static_cast<_Base*>(this);
        return *this;
    }

    _RBTree_iterator operator--(int)
    {
        _RBTree_iterator tmp = *this;
        --*this;
        return tmp;
    }
};  // _RBTree_iterator


template <typename T, typename Compare, typename Alloc>
class _RBTree_base
{
public:
    using value_type             = T;
    using allocator_type         = Alloc;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    
private:
    using _Al_traits             = allocator_traits<Alloc>;
    using _Node                  = RBTNode<T>;
    using _Nodeptr               = RBTNode<T>*;
    using _Alnode                = typename _Al_traits::template rebind_alloc<_Node>;
    using _Alnode_traits         = allocator_traits<_Alnode>;
    
protected:
    _Nodeptr  header;
    size_type m_count;
    _Alnode   alloc;
    Compare   compare;

public:
    _RBTree_base() : m_count(0), alloc(), compare()
    {
        _Create_header_node();
    }

    template <typename Any_alloc,
        typename = enable_if_t<!is_same<decay_t<Any_alloc>, _RBTree_base>::value>>
    _RBTree_base(const Compare& cmp, Any_alloc&& anyAlloc) 
    : m_count(0), alloc(tiny_stl::forward<Any_alloc>(anyAlloc)), compare(cmp)
    {
        _Create_header_node();
    }

    ~_RBTree_base()
    {
        alloc.deallocate(header, 1);
    }

private:
    void _Create_header_node()
    {
        try
        {
            header = alloc.allocate(1);
            header->left = header;
            header->right = header;
            header->parent = header;
            header->isNil = 1;
            header->color = Color::BLACK;
        }
        catch (...)
        {
            alloc.deallocate(header, 1);
            throw;
        }
    }
};  // _RBTree_base


template <typename T, typename Compare, typename Alloc, bool is_map>
class _RBTree : public _RBTree_base<T, Compare, Alloc>
{
public:
    using key_type               = typename _Asso_type_helper<T, is_map>::key_type;
    using mapped_type            = typename _Asso_type_helper<T, is_map>::mapped_type;
    using value_type             = T;
    using key_compare            = Compare;         // if it is map, compare pair
    using allocator_type         = Alloc;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    
    using _Al_traits             = allocator_traits<Alloc>;
    using _Node                  = RBTNode<value_type>;
    using _Nodeptr               = RBTNode<value_type>*;
    using _Alnode                = typename _Al_traits::template rebind_alloc<_Node>;
    using _Alnode_traits         = allocator_traits<_Alnode>;
    using _Base                  = _RBTree_base<value_type, Compare, Alloc>;

    using iterator               = _RBTree_iterator<value_type>;
    using const_iterator         = _RBTree_const_iterator<value_type>;
    using reverse_iterator       = tiny_stl::reverse_iterator<iterator>;
    using const_reverse_iterator = tiny_stl::reverse_iterator<const_iterator>;
private:
    template <typename... Args>
    _Nodeptr _Alloc_construct(Args&&... args)
    {
        _Nodeptr p = this->alloc.allocate(1);

        p->color = Color::RED;
        p->isNil = 0;
        p->parent = this->header;
        p->left = this->header;
        p->right = this->header;

        try
        {
            this->alloc.construct(tiny_stl::addressof(p->value),
                tiny_stl::forward<Args>(args)...);
        }
        catch (...)
        {
            this->alloc.deallocate(p, 1);
            throw;
        }

        return p;
    }

    void _Destroy_Free(_Nodeptr p)
    {
        if (p != nullptr)
        {
            this->alloc.destroy(tiny_stl::addressof(p->value));
            this->alloc.deallocate(p, 1);
        }
    }

    _Nodeptr& _Root() const noexcept
    {
        return this->header->parent;
    }

    const key_type& _Get_key_node(_Nodeptr ptr, true_type) const    // map
    {
        return ptr->value.first;
    }

    const key_type& _Get_key_node(_Nodeptr ptr, false_type) const   // set
    {
        return ptr->value;
    }

    const key_type& get_key(_Nodeptr ptr) const
    {
        return _Get_key_node(ptr, tiny_stl::bool_constant<is_map>{});
    }

    const key_type& _Get_key_value(const T& val, true_type) const   // map
    {
        return val.first;
    }

    const key_type& _Get_key_value(const T& val, false_type) const   // map
    {
        return val;
    }

    const key_type& _Get_key_from_value(const T& val) const
    {
        return _Get_key_value(val, tiny_stl::bool_constant<is_map>{});
    }


    template <typename K>
    _Nodeptr _Low_bound(const K& val) const
    {
        _Nodeptr pos = this->header;
        _Nodeptr p = pos->parent;

        while (!p->isNil)
        {
            if (this->compare(get_key(p), val))
            {
                p = p->right;
            }
            else
            {
                pos = p;
                p = p->left;
            }
        }

        return pos;
    }

    template <typename K>
    _Nodeptr _Upp_bound(const K& val) const
    {
        _Nodeptr pos = this->header;
        _Nodeptr p = pos->parent;

        while (!p->isNil)
        {
            if (!this->compare(val, get_key(p)))
            {
                p = p->right;
            }
            else
            {
                pos = p;
                p = p->left;
            }
        }

        return pos;
    }
    
    _Nodeptr _Copy_nodes(_Nodeptr rhsRoot, _Nodeptr thisPos)
    {
        _Nodeptr newheader = this->header;

        if (!rhsRoot->isNil)
        {
            _Nodeptr p = this->alloc.allocate(1);
            p->color = rhsRoot->color;
            p->isNil = 0;
            p->parent = thisPos;
            try
            {
                p->value = rhsRoot->value;
            }
            catch (...)
            {
                this->alloc.deallocate(p, 1);
                throw;
            }

            if (newheader->isNil)
                newheader = p;

            p->left = _Copy_nodes(rhsRoot->left, p);
            p->right = _Copy_nodes(rhsRoot->right, p);
        }

        return newheader;
    }

    void _Copy(const _RBTree& rhs)
    {
        _Root() = _Copy_nodes(rhs._Root(), this->header);
        this->m_count = rhs.m_count;

        if (!_Root()->isNil)
        {
            this->header->left = _RBTree_min_value(_Root());
            this->header->right = _RBTree_max_value(_Root());
        }
        else
        {
            this->header->left = this->header;
            this->header->right = this->header;
        }
    }

    void _Move(_RBTree&& rhs)
    {
        tiny_stl::_Swap_ADL(this->compare, rhs.compare);
        tiny_stl::_Swap_ADL(this->header, rhs.header);
        tiny_stl::_Swap_ADL(this->m_count, rhs.m_count);
    }


    void _RBT_fixup_for_insert(_Nodeptr& root, _Nodeptr z)
    {
        while (z->parent->color == Color::RED)                  // parent is red
        {
            if (z->parent == z->parent->parent->left)           // parent is grandfather's left child
            {
                _Nodeptr y = z->parent->parent->right;          // y is z's uncle

                if (y->color == Color::RED)                     // case 1, z's uncle is red
                {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                }
                else
                {
                    if (z == z->parent->right)                  // case 2, z is parent's right child
                    {
                        z = z->parent;
                        _RBTree_left_rotate(root, z);
                    }
                    z->parent->color = Color::BLACK;            // case 3, z is parent's left child
                    z->parent->parent->color = Color::RED;
                    _RBTree_right_rotate(root, z->parent->parent);
                }
            }
            else                                                // parent is grandfather's right
            {
                _Nodeptr y = z->parent->parent->left;           // y is z's uncle
    
                if (y->color == Color::RED)                     // case 1, z's uncle is red
                {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                }
                else
                {
                    if (z == z->parent->left)                   // case 2, z is parent's left child
                    {
                        z = z->parent;
                        _RBTree_right_rotate(root, z);
                    }
                    z->parent->color = Color::BLACK;            // case 3, z is parent's left child
                    z->parent->parent->color = Color::RED;
                    _RBTree_left_rotate(root, z->parent->parent);
                }
            }
        }

        root->color = Color::BLACK;
    }

    void _RBT_fixup_for_erase(_Nodeptr& root, _Nodeptr x)
    {
        while (x != root && x->color == Color::BLACK)
        {
            if (x == x->parent->left)
            {
                _Nodeptr w = x->parent->right;
                if (w->color == Color::RED)                     // case 1, x's brother w is red
                {
                    w->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    _RBTree_left_rotate(root, x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == Color::BLACK              // case 2, w's left and 
                    && w->right->color == Color::BLACK)         // right child is black
                {
                    w->color = Color::RED;
                    x = x->parent;
                }
                else                                            // case 3, w's right child is black
                {
                    if (w->right->color == Color::BLACK)       
                    {
                        w->left->color = Color::BLACK;
                        w->color = Color::RED;
                        _RBTree_right_rotate(root, w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = Color::BLACK;
                    w->right->color = Color::BLACK;
                    _RBTree_left_rotate(root, x->parent);
                    x = root;
                }
            }
            else
            {
                _Nodeptr w = x->parent->left;
                if (w->color == Color::RED)                     // case 1
                {
                    w->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    _RBTree_right_rotate(root, x->parent);
                    w = x->parent->left;
                }
                if (w->left->color == Color::BLACK              // case 2
                    && w->right->color == Color::BLACK)
                {
                    w->color = Color::RED;
                    x = x->parent;
                }
                else
                {
                    if (w->left->color == Color::BLACK)         // case 3
                    {
                        w->right->color = Color::BLACK;
                        w->color = Color::RED;
                        _RBTree_left_rotate(root, w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;                // case 4
                    x->parent->color = Color::BLACK;
                    w->left->color = Color::BLACK;
                    _RBTree_right_rotate(root, x->parent);
                    x = root;
                }
            }
        }

        x->color = Color::BLACK;
        this->header->color = Color::BLACK;
    }

public:
    _RBTree() : _Base() {}

    _RBTree(const Compare& cmp) : _Base(cmp) {}

    _RBTree(const Compare& cmp, const allocator_type& alloc)
    : _Base(cmp, alloc) { }

    template <typename Any_alloc>
    _RBTree(const _RBTree& rhs, Any_alloc&& alloc) 
    : _Base(rhs.compare, tiny_stl::forward<Any_alloc>(alloc))
    {
        _Copy(rhs);
    }
    
    _RBTree(_RBTree&& rhs) : _Base(rhs.compare, rhs.alloc) 
    {
        _Move(tiny_stl::move(rhs));
    }

    _RBTree(_RBTree&& rhs, const Alloc& alloc) : _Base(rhs.compare, alloc)
    {
        _Move(tiny_stl::move(rhs));
    }

    _RBTree& operator=(const _RBTree& rhs)
    {
        // Non-standard, user allocator may be wrong
        clear();
        _Copy(rhs);

        return *this;
    }

    _RBTree& operator=(_RBTree&& rhs) 
    {
        // Non-standard, user allocator may be wrong
        clear();
        _Move(tiny_stl::move(rhs));

        return *this;
    }

    ~_RBTree()
    {
        clear();
    }

    allocator_type get_allocator() const noexcept
    {
        return this->alloc;
    }

    size_type size() const noexcept
    {
        return this->m_count;
    }

    bool empty() const noexcept
    {
        return size() == 0;
    }

    size_type max_size() const noexcept
    {
        return _Alnode_traits::max_size(this->alloc);
    }

    iterator lower_bound(const key_type& val)
    {
        return iterator(_Low_bound(val));
    }

    const_iterator lower_bound(const key_type& val) const
    {
        return const_iterator(_Low_bound(val));
    }

    template <typename K, 
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    iterator lower_bound(const K& val)
    {
        return iterator(_Low_bound(val));
    }

    template <typename K,
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    const_iterator lower_bound(const K& val) const
    {
        return iterator(_Low_bound(val));
    }

    iterator upper_bound(const key_type& val)
    {
        return iterator(_Upp_bound(val));
    }

    const_iterator upper_bound(const key_type& val) const
    {
        return const_iterator(_Upp_bound(val));
    }

    template <typename K,
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    iterator upper_bound(const K& val)
    {
        return iterator(_Upp_bound(val));
    }

    template <typename K,
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    const_iterator upper_bound(const K& val) const
    {
        return const_iterator(_Upp_bound(val));
    }

    pair<iterator, iterator> equal_range(const key_type& key)
    {
        return { lower_bound(key), upper_bound(key) };
    }

    pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        return { lower_bound(key), upper_bound(key) };
    }

    template <typename K,
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    pair<iterator, iterator> equal_range(const K& key)
    {
        return { lower_bound(key), upper_bound(key) };
    }

    template <typename K,
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    pair<const_iterator, const_iterator> equal_range(const K& key) const
    {
        return { lower_bound(key), upper_bound(key) };
    }

    size_type count(const key_type& key) const
    {
        pair<const_iterator, const_iterator> range = equal_range(key);
        return tiny_stl::distance(range.first, range.second);
    }

    template <typename K>
    size_type count(const K& key) const
    {
        pair<const_iterator, const_iterator> range = equal_range(key);
        return tiny_stl::distance(range.first, range.second);
    }

    iterator find(const key_type& val)
    {
        iterator pos = lower_bound(val);
        return (pos == end() || this->compare(val, get_key(pos.ptr)))
            ? end() : pos;
    }

    const_iterator find(const key_type& val) const
    {
        const_iterator pos = lower_bound(val);
        return (pos == end() || this->compare(val, get_key(pos.ptr)))
            ? end() : pos;
    }

    template <typename K,
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    iterator find(const K& val)
    {
        iterator pos = lower_bound(val);
        return (pos == end() || this->compare(val, get_key(pos.ptr)))
            ? end() : pos;
    }

    template <typename K,
        typename Cmp = Compare,
        typename = typename Cmp::is_transparent>
    const_iterator find(const K& val) const 
    {
        iterator pos = lower_bound(val);
        return (pos == end() || this->compare(val, get_key(pos.ptr)))
            ? end() : pos;
    }


private:
    iterator _Insert(_Nodeptr z)
    {
        _Nodeptr x = _Root();
        _Nodeptr y = this->header;

        if (y->left->isNil || this->compare(get_key(z), get_key(y->left)))
            y->left = z;

        if (y->right->isNil || !this->compare(get_key(z), get_key(y->right)))
            y->right = z;

        while (!x->isNil)
        {
            y = x;
            if (this->compare(get_key(z), get_key(x)))
                x = x->left;
            else
                x = x->right;
        }

        z->parent = y;

        if (y->isNil)
        {
            _Root() = z;
            this->header->left = z;
            this->header->right = z;
        }
        else if (this->compare(get_key(z), get_key(y)))
        {
            y->left = z;
            if (this->compare(get_key(z), 
                            get_key(this->header->left)))   // z.key < min_value
                this->header->left = z;
        }
        else
        {
            y->right = z;
            if (!this->compare(get_key(z), 
                            get_key(this->header->right)))   // z.key >= max_value
                this->header->right = z;
        }

        _RBT_fixup_for_insert(_Root(), z);

        ++this->m_count;

        return iterator(z);
    }

    template <typename Value>
    iterator _Insert_equal(Value&& val)
    {
        _Nodeptr z = _Alloc_construct(tiny_stl::forward<Value>(val));
        return _Insert(z);
    }

    template <typename Value>
    pair<iterator, bool> _Insert_unique(Value&& val)
    {
        T value(tiny_stl::forward<Value>(val));
        iterator pos = find(_Get_key_from_value(val));

        if (pos != end())
        {
            return make_pair(pos, false);
        }
            
        _Nodeptr z = _Alloc_construct(tiny_stl::move(value));
        return make_pair(_Insert(z), true);
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
        for (;first != last; ++first)
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
        return _Insert_equal(tiny_stl::forward<Args>(args)...);
    }

    template <typename... Args>
    pair<iterator, bool> emplace_unique(Args&&... args)
    {
        return _Insert_unique(tiny_stl::forward<Args>(args)...);
    }

private:
    // transplant v to the location of u
    inline void _Transplant_for_erase(_Nodeptr& root, _Nodeptr u, _Nodeptr v)
    {
        if (u->parent->isNil)                   // u is root
            root = v;
        else if (u == u->parent->left)          // u is left child
            u->parent->left = v;
        else                                    // u is right child
            u->parent->right = v;
       
        v->parent = u->parent;
    }

    // erase node z
    void _Erase(_Nodeptr root, _Nodeptr z)
    {
        _Nodeptr y = z;
        _Nodeptr x = nullptr;
       
        
        Color yOriginColor = y->color;

        if (z == this->header->left)
            this->header->left = (++iterator(z)).ptr;

        if (z == this->header->right)
            this->header->right = (--iterator(z)).ptr;

        if (z->left->isNil)                     // z has not left child
        {
            x = z->right;
            _Transplant_for_erase(_Root(), z, z->right);
        }
        else if (z->right->isNil)               // z has not right child
        {
            x = z->left;
            _Transplant_for_erase(_Root(), z, z->left);
        }
        else                                    // z has left and right child
        {
            y = _RBTree_min_value(z->right);
            yOriginColor = y->color;
            x = y->right;

            if (y->parent == z)
            {
                x->parent = y;
            }
            else
            {
                _Transplant_for_erase(_Root(), y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            
            _Transplant_for_erase(_Root(), z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        if (yOriginColor == Color::BLACK)
            _RBT_fixup_for_erase(root, x);

        _Destroy_Free(z);
        --this->m_count;

        this->header->parent = root;
        root->parent = this->header;
    }

public:
    iterator erase(const_iterator pos)
    {
        _Nodeptr z = pos.ptr;

        ++pos;
        _Erase(_Root(), z);
        return iterator(pos.ptr);
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

        return iterator(first.ptr);
    }

    size_type erase(const key_type& key)
    {
        auto ppos = equal_range(key);
        size_type num = tiny_stl::distance(ppos.first, ppos.second);

        erase(ppos.first, ppos.second);

        return num;
    }

public:
    iterator begin() noexcept
    {
        return iterator(this->header->left);
    }

    const_iterator begin() const noexcept
    {
        return const_iterator(this->header->left);
    }

    const_iterator cbegin() const noexcept
    {
        return begin();
    }

    iterator end() noexcept
    {
        return iterator(this->header);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(this->header);
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
    void _Clear(_Nodeptr root)
    {
        for (_Nodeptr p = root; !p->isNil; root = p)
        {
            _Clear(p->right);
            p = p->left;
            _Alnode_traits::destroy(this->alloc, tiny_stl::addressof(root->value));
            this->alloc.deallocate(root, 1);
        }
    }

public:
    void clear()
    {
        _Clear(_Root());
        this->header->left = this->header;
        this->header->right = this->header;
        this->header->parent = this->header;
        this->m_count = 0;
    }

    void swap(_RBTree& rhs) 
        noexcept(_Al_traits::is_always_equal::value 
            && std::_Is_nothrow_swappable<Compare>::value)
    {
        assert(this->alloc == rhs.alloc);

#pragma warning(push)   // if constexpr
#pragma warning(disable : 4984)
        if constexpr (allocator_traits<Alloc>::propagate_on_container_swap::value)
            tiny_stl::_Swap_alloc(this->alloc, rhs.alloc);
#pragma warning(pop)

        tiny_stl::_Swap_ADL(this->header, rhs.header);
        tiny_stl::_Swap_ADL(this->compare, rhs.compare);
        tiny_stl::_Swap_ADL(this->m_count, rhs.m_count);
    }

    
};  // _RBTree

template <typename T, typename Compare, typename Alloc, bool is_map>
bool operator==(const _RBTree<T, Compare, Alloc, is_map>& lhs,
                const _RBTree<T, Compare, Alloc, is_map>& rhs)
{
    return lhs.size() == rhs.size() &&
        tiny_stl::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, typename Compare, typename Alloc, bool is_map>
bool operator!=(const _RBTree<T, Compare, Alloc, is_map>& lhs,
                const _RBTree<T, Compare, Alloc, is_map>& rhs)
{
    return !(lhs == rhs);
}


template <typename T, typename Compare, typename Alloc, bool is_map>
bool operator<(const _RBTree<T, Compare, Alloc, is_map>& lhs,
               const _RBTree<T, Compare, Alloc, is_map>& rhs)
{
    return tiny_stl::lexicographical_compare(lhs.begin(), lhs.end(),
                                             rhs.begin(), rhs.end());
}

template <typename T, typename Compare, typename Alloc, bool is_map>
bool operator>(const _RBTree<T, Compare, Alloc, is_map>& lhs,
               const _RBTree<T, Compare, Alloc, is_map>& rhs)
{
    return rhs < lhs;
}

template <typename T, typename Compare, typename Alloc, bool is_map>
bool operator<=(const _RBTree<T, Compare, Alloc, is_map>& lhs,
                const _RBTree<T, Compare, Alloc, is_map>& rhs)
{
    return !(rhs < lhs);
}

template <typename T, typename Compare, typename Alloc, bool is_map>
bool operator>=(const _RBTree<T, Compare, Alloc, is_map>& lhs,
                const _RBTree<T, Compare, Alloc, is_map>& rhs)
{
    return !(lhs < rhs);
}

}   // namespace tiny_stl