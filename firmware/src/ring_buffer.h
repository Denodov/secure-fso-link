#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>

typedef struct {
    size_t s_elem; // size of each element
    size_t n_elem; // number of elements (must be power of 2)
    void *buffer;  // data storage sized s_elem * n_elem bytes
} rb_attr_t;
// memory used by ring buffer to store data must be provided 

typedef unsigned int rbd_t;

int ring_buffer_init(rbd_t *rbd, rb_attr_t *attr);
int ring_buffer_write(rbd_t rbd, const void *data);
int ring_buffer_read(rbd_t rbd, void *data);

#endif
