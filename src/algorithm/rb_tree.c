#include <algorithm/rb_tree.h>

#include <memory/manipulate.h>

void rb_init_tree(rb_tree *t, rb_comparator comp)
{
    memset(t->nil, 0, sizeof(rb_node));
    t->root = t->nil;
    t->comp = comp;
}

static inline rb_node ALWAYS_INLINE *get_parent(rb_node *x) {
    return (rb_node *) (x->_parent & ~0x1);
}

static inline void ALWAYS_INLINE set_parent(rb_node *x, rb_node *new_parent) {
    x->_parent = ((uint64_t) new_parent) | (x->_parent & 0x1);
}

static inline uint8_t ALWAYS_INLINE is_red(rb_node *x) {
    return x->_parent & 0x1;
}

static inline void ALWAYS_INLINE set_red(rb_node *x) {
    x->_parent |= 0x1;
}

static inline void ALWAYS_INLINE set_black(rb_node *x) {
    x->_parent &= ~0x1;
}

static inline void ALWAYS_INLINE left_rotate(rb_tree *t, rb_node *x) {
    rb_node *y = x->_right;
    x->_right = y->_left;
    if (y->_left != t->nil)
        set_parent(y->_left, x);
    set_parent(y, get_parent(x));
    if (get_parent(x) == t->nil)
        t->root = y;
    else if (x == get_parent(x)->_left)
        get_parent(x)->_left = y;
    else
        get_parent(x)->_right = y;
    y->_left = x;
    set_parent(x, y);
}

static inline void ALWAYS_INLINE right_rotate(rb_tree *t, rb_node *x) {
    rb_node *y = x->_left;
    x->_left = y->_right;
    if (y->_right != t->nil)
        set_parent(y->_right, x);
    set_parent(y, get_parent(x));
    if (get_parent(x) == t->nil)
        t->root = y;
    else if (x == get_parent(x)->_left)
        get_parent(x)->_left = y;
    else
        get_parent(x)->_right = y;
    y->_right = x;
    set_parent(x, y);
}

static inline void ALWAYS_INLINE rb_insert_fixup(rb_tree *t, rb_node *z) {
    while (is_red(get_parent(z)))
    {
        rb_node *y;
        if (get_parent(z) == get_parent(get_parent(z))->_left)
        {
            y = get_parent(get_parent(z))->_right;
            if (is_red(y))
            {
                set_black(get_parent(z));
                set_black(y);
                set_red(get_parent(get_parent(z)));
                z = get_parent(get_parent(z));
            }
            else
            {
                if (z == get_parent(z)->_right)
                {
                    z = get_parent(z);
                    left_rotate(t, z);
                }
                set_black(get_parent(z));
                set_red(get_parent(get_parent(z)));
                right_rotate(t, get_parent(get_parent(z)));
            }
        }
        else
        {
            y = get_parent(get_parent(z))->_left;
            if (is_red(y))
            {
                set_black(get_parent(z));
                set_black(y);
                set_red(get_parent(get_parent(z)));
                z = get_parent(get_parent(z));
            }
            else
            {
                if (z == get_parent(z)->_left)
                {
                    z = get_parent(z);
                    right_rotate(t, z);
                }
                set_black(get_parent(z));
                set_red(get_parent(get_parent(z)));
                left_rotate(t, get_parent(get_parent(z)));
            }
        }
    }
    set_black(t->root);
}

void rb_insert(rb_tree *t, rb_node *z) {
    rb_node *y = t->nil;
    rb_node *x = t->root;
    z->_parent = 0x1;
    while (x != t->nil)
    {
        y = x;
        if (t->comp(z->_key, x->_key) < 0)
            x = x->_left;
        else
            x = x->_right;
    }
    set_parent(z, y);
    if (y == t->nil)
        t->root = z;
    else if (t->comp(z->_key, y->_key) < 0)
        y->_left = z;
    else
        y->_right = z;
    z->_left = t->nil;
    z->_right = t->nil;
    rb_insert_fixup(t, z);
}

static inline void ALWAYS_INLINE rb_transplant(rb_tree *t, rb_node *u, rb_node *v)
{
   if (get_parent(u) == t->nil)
      t->root = v;
   else if (u == get_parent(u)->_left)
       get_parent(u)->_left = v;
   else
       get_parent(u)->_right = v;
    set_parent(v, get_parent(u));
}

static inline rb_node* ALWAYS_INLINE rb_min(rb_tree *t, rb_node *root)
{
    rb_node *ret = t->nil;
    while (root != t->nil)
    {
        ret = root;
        root = root->_left;
    }
    return ret;
}

static inline void ALWAYS_INLINE rb_delete_fixup(rb_tree *t, rb_node *x)
{
    rb_node *w;
    while (x != t->root && !is_red(x))
    {
        if (x == get_parent(x)->_left)
        {
            w = get_parent(x)->_right;
            if (is_red(w))
            {
                set_black(w);
                set_red(get_parent(x));
                left_rotate(t, get_parent(x));
                w = get_parent(x)->_right;
            }
            if (!is_red(w->_left) && !is_red(w->_right))
            {
                set_red(w);
                x = get_parent(x);
            }
            else
            {
                if (!is_red(w->_right))
                {
                    set_black(w->_left);
                    set_red(w);
                    right_rotate(t, w);
                    w = get_parent(x)->_right;
                }
                w->_parent = (w->_parent & ~0x1) | is_red(get_parent(x));
                set_black(get_parent(x));
                w->_right->_parent = w->_right->_parent & ~0x1;
                left_rotate(t, get_parent(x));
                x = t->root;
            }
        }
        else
        {
            w = get_parent(x)->_left;
            if (is_red(w))
            {
                set_black(w);
                set_red(get_parent(x));
                right_rotate(t, get_parent(x));
                w = get_parent(x)->_left;
            }
            if (!is_red(w->_left) && !is_red(w->_right))
            {
                set_red(w);
                x = get_parent(x);
            }
            else
            {
                if (!is_red(w->_left))
                {
                    set_black(w->_right);
                    set_red(w);
                    left_rotate(t, w);
                    w = get_parent(x)->_left;
                }
                w->_parent = (w->_parent & ~0x1) | is_red(get_parent(x));
                set_black(get_parent(x));
                w->_left->_parent = w->_left->_parent & ~0x1;
                right_rotate(t, get_parent(x));
                x = t->root;
            }
        }
    }
    set_black(x);
}

void rb_delete(rb_tree *t, rb_node *z)
{
    rb_node *x;
    rb_node *y = z;
    uint8_t y_original = is_red(y);
    if (z->_left == t->nil)
    {
        x = z->_right;
        rb_transplant(t, z, z->_right);
    }
    else if (z->_right == t->nil)
    {
        x = z->_left;
        rb_transplant(t, z, z->_left);
    }
    else
    {
       y = rb_min(t, z->_right);
       y_original = is_red(y);
       x = y->_right;
       if (get_parent(y) == z)
           set_parent(x, y);
       else
       {
           rb_transplant(t, y, y->_right);
           y->_right = z->_right;
           set_parent(y->_right, y);
       }
        rb_transplant(t, z, y);
        y->_left = z->_left;
        set_parent(y->_left, y);
        y->_parent = (y->_parent & ~0x1) | is_red(z);
    }
    if (!y_original)
        rb_delete_fixup(t, x);
}

rb_node* rb_find(rb_tree *t, uint64_t key)
{
    rb_node *x = t->root;
    while (x != t->nil)
    {
        int8_t comp_res = t->comp(key, x->_key);
        if (comp_res < 0)
            x = x->_left;
        else if (comp_res > 0)
            x = x->_right;
        else
            return x;
    }
    return t->nil;
}