#pragma once
#include "heap.h"
#include <stddef.h>

#define LIST(t, s)              struct __list_##t##_node { \
                                    struct __list_##t##_node *prev, *next; \
                                    s value; \
                                } t##_node_t; \
                                typedef struct list_##t { \
                                    struct __list_##t##_node *begin, *end; \
                                } t##_t; \
                                typedef s __list_##t##_type_t
#define NODE(t)                 struct __list_##t##_node

#define list_append(l, t, v)    __list_append((__list_generic_t *) (l), v, sizeof(__list_##t##_type_t))
#define list_pop_front(l)       __list_pop_front((__list_generic_t *) (l))
#define list_init(l)            __list_init((__list_generic_t *) (l))
#define list_foreach(l, t, i)      for (struct __list_##t##_node *i = (l)->begin; i; i = i->next)
#define list_empty(l)           (!((__list_generic_t *) (l)->begin))

#define list_node_free(n)       __list_node_free((__list_node_generic_t *) (n))

typedef struct __list_node_generic {
    struct __list_node_generic *prev, *next;
    // Value follows
} __list_node_generic_t;

typedef struct {
    struct __list_node_generic *begin, *end;
} __list_generic_t;

void __list_init(__list_generic_t *l);
void __list_append(__list_generic_t *l, void *v, size_t isz);
void *__list_pop_front(__list_generic_t *l);
#define __list_node_free(n) heap_free(n)
