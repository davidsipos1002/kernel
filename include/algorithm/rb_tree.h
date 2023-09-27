#ifndef ALGORITHM_RB_TREE_H_INCL
#define ALGORITHM_RB_TREE_H_INCL

#include <stdint.h>
#include <gcc/utils.h>

typedef struct rb
{
    uint64_t _parent;
    struct rb *_left, *_right;
    uint64_t _key;
} ALIGN(8) rb_node;

typedef int8_t (*rb_comparator) (uint64_t a, uint64_t b);

typedef struct
{
    rb_node *root;
    rb_node *nil;
    rb_comparator comp;
} rb_tree;

void rb_init_tree(rb_tree *t, rb_comparator comp);
void rb_insert(rb_tree *t, rb_node *z);
void rb_delete(rb_tree *t, rb_node *z);
rb_node* rb_find(rb_tree *t, uint64_t key);

int8_t rb_simple_comparator(uint64_t a, uint64_t b);

#endif