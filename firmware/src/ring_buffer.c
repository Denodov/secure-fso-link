#include "ring_buffer.h"
#include <stdint.h>
#include <string.h>
#include <stdatomic.h>

#define RING_BUFFER_MAX 2

struct ring_buffer
{
    size_t s_elem;
    size_t n_elem;
    uint8_t *buf;
    _Atomic size_t head; 
    _Atomic size_t tail;
};

static struct ring_buffer ring_buffers[RING_BUFFER_MAX];

int ring_buffer_init(rbd_t *rbd, rb_attr_t *attr) {
    static int indx = 0;
    int err = -1;

    // check for free ring buffer, ID, and provided attributes
    if((indx < RING_BUFFER_MAX) && (rbd != NULL) && (attr != NULL)) {
        // verify that buffer pointer and element size are valid
        if((attr->buffer != NULL) && (attr->n_elem > 0) && (attr->s_elem > 0)) {
            // check if number of elements is a power of 2
            // allows for replacing modulo operations w/ bitwise masking
            /* any value which is a power of two will have only one ‘1’ in its binary representation
             * 2^4 = 16 = 0b10000 = value
             * 16 - 1 =   0b01111 = mask 
             * value & mask = always 0 for a power of 2
             * */
            if(((attr->n_elem) & (attr->n_elem-1)) == 0) {
                // initialize internal rb variables
                atomic_init(&ring_buffers[indx].head, 0);
                atomic_init(&ring_buffers[indx].tail, 0);

                if(!atomic_is_lock_free(&ring_buffers[indx].head) ||
                   !atomic_is_lock_free(&ring_buffers[indx].tail)) {
                    return -1;
                }
                
                ring_buffers[indx].buf  = attr->buffer;
                ring_buffers[indx].s_elem = attr->s_elem;
                ring_buffers[indx].n_elem = attr->n_elem;

                *rbd = indx++;
                err = 0;
            }
        }
    }
    return err;
}
/* if total writes - total reads == buffer size, buffer full
 * if total writes - total reads == 0, buffer empty */
/*
static int rb_full(struct ring_buffer *rb) {
    return ((rb->head - rb->tail) == rb->n_elem) ? 1 : 0;
}
static int rb_empty(struct ring_buffer *rb) {
    return ((rb->head - rb->tail) == 0u) ? 1 : 0;
}
*/

/* copy one element from data into buffer.
 * returns 0 on success, -1 if full or rbd is invalid.
 * requires initialized buffer and valid data pointer
 */
int ring_buffer_write(rbd_t rbd, const void *data) {
    if(rbd >= RING_BUFFER_MAX) {
        return -1;
    }

    struct ring_buffer *rb = &ring_buffers[rbd];

    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

    if((head-tail) == rb->n_elem) {
        return -1; // ring buffer full
    }

        /* find the element head is currently on 
         * 2^4 = 16 = 0b10000
         * 16 - 1 =   0b01111 = mask  
         * if head is 54 = 0b110110
         * mask & head: 0001111
         *            & 0110110 
         *            = 0000110 = 6 (element 7) */
        size_t offset = (head & (rb->n_elem - 1)) * rb->s_elem; 

        // copy data into ring buffer
        memcpy(rb->buf + offset, data, rb->s_elem); 
        // ring_buffers[rbd].head++;
        // publish new element only after copying it
        atomic_store_explicit(&rb->head, head+1, memory_order_release);

        return 0;
}

/* copy one element from buffer into data. 
 * returns 0 on success, -1 if buffer empty or rbd is invalid*/
int ring_buffer_read(rbd_t rbd, void *data) {
    if(rbd >= RING_BUFFER_MAX) {
        return -1;
    }

    struct ring_buffer *rb = &ring_buffers[rbd];

    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);

    if(head == tail) {
        return -1; // ring buffer empty
    }
    size_t offset = (tail & (rb->n_elem - 1)) * rb->s_elem; 
        // copy data from ring buffer
        memcpy(data, rb->buf + offset, rb->s_elem);

        // release the slot only after copying data out
        atomic_store_explicit(&rb->tail, tail+1, memory_order_release);
        return 0;
}
