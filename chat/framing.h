#ifndef FRAMING_H 
#define FRAMING_H 

#include <stdint.h>
#include <stddef.h>

int frame_encode(const uint8_t *packet,
                 size_t packet_len,
                 uint8_t *output,
                 size_t output_capacity,
                 size_t *output_len);

int frame_decode(const uint8_t *frame,
                 size_t frame_len,
                 uint8_t *packet,
                 size_t packet_capacity,
                 size_t *packet_len);

#endif
