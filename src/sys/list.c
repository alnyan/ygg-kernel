#include "list.h"
#include "mem.h"
#include "assert.h"

#define __list_node_create(sz)      ((__list_node_generic_t *) heap_alloc(sizeof(__list_node_generic_t) + sz))

void __list_append(__list_generic_t *l, void *v, size_t sz) {
    // Allocate a new node (__list_node_create may later abstract this and use some pooled allocation
    // for similar-sized nodes)
    __list_node_generic_t *new_node = __list_node_create(sz);
    assert(new_node);

    new_node->next = NULL;
    new_node->prev = l->end;

    // Copy the value
    memcpy(((char *) new_node) + sizeof(__list_node_generic_t), v, sz);

    if (l->end) {
        l->end->next = new_node;
    } else {
        l->begin = new_node;
    }

    l->end = new_node;
}

void *__list_pop_front(__list_generic_t *l) {
    void *r = NULL;

    if (l->begin) {
        r = l->begin;
        l->begin = ((__list_node_generic_t *) r)->next;
        if (l->begin) {
            l->begin->prev = NULL;
        }
    }

    return r;
}

void __list_init(__list_generic_t *l) {
    l->begin = NULL;
    l->end = NULL;
}
