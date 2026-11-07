#include "ring_buffer.h"
#include <stdint.h>
#include <string.h>

#define RING_BUFFER_MAX 1

struct ring_buffer
{
    size_t s_elem;
    size_t n_elem;
    uint8_t *buf;
    volatile size_t head; // volatile since head & tail will be accessed from both application context and interrupt context
    volatile size_t tail;
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
                ring_buffers[indx].head = 0;
                ring_buffers[indx].tail = 0;
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
static int rb_full(struct ring_buffer *rb) {
    return ((rb->head - rb->tail) == rb->n_elem) ? 1 : 0;
}
static int rb_empty(struct ring_buffer *rb) {
    return ((rb->head - rb->tail) == 0u) ? 1 : 0;
}

int ring_buffer_write(rbd_t rbd, const void *data) {
    int err = 0;

    // if ring buffer is not full
    if((rbd < RING_BUFFER_MAX) && (rb_full(&ring_buffers[rbd]) == 0)) {
        /* find the element head is currently on 
         * 2^4 = 16 = 0b10000
         * 16 - 1 =   0b01111 = mask  
         * if head is 54 = 0b110110
         * mask & head: 0001111
         *            & 0110110 
         *            = 0000110 = 6 (element 7) */
        const size_t offset = 
            (ring_buffers[rbd].head & (ring_buffers[rbd].n_elem - 1)) *
             ring_buffers[rbd].s_elem; 
        // copy data into ring buffer
        memcpy(&(ring_buffers[rbd].buf[offset]), data, ring_buffers[rbd].s_elem); 
        ring_buffers[rbd].head++;
    } else {
        err = -1;
    }
    return err;
}

int ring_buffer_read(rbd_t rbd, void *data) {
    int err = 0;
    
    // if ring buffer is not empty 
    if((rbd < RING_BUFFER_MAX) && (rb_empty(&ring_buffers[rbd]) == 0)) {
        const size_t offset = 
            (ring_buffers[rbd].tail & (ring_buffers[rbd].n_elem - 1)) *
             ring_buffers[rbd].s_elem; 
        // copy data from ring buffer
        memcpy(data, &(ring_buffers[rbd].buf[offset]), ring_buffers[rbd].s_elem);
        ring_buffers[rbd].tail++;
    } else {
        err = -1;
    }
    return err;
}
