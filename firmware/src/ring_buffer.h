#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>

typedef struct {
    size_t s_elem; // size of each element
    size_t n_elem; // number of elements
    void *buffer;
} rb_attr_t;
// memory used by ring buffer to store data must be provided 

typedef unsigned int rbd_t;

#endif
