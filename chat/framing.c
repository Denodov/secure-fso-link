#include "framing.h"

/* 
 * COBS framing
 * [DATA/ACK][COUNTER][PAYLOAD][CHECK]
 * Encoding:
 * 1. Leave first spot blank
 * 2. go through data one byte at a time
 * 3. if byte is nonzero, copy it to the output array and increment counter
 * 4. if byte is zero, go back and write the counter into the blank spot.
 * 5. leave a new blank spot and restart the counter
 * 6. at the end, fill in the last blank spot and add the 0x00 end marker
 *
 * Ex. original msg: 48 00 69 FF
 *     sent msg:     02 48 03 69 FF 00
 *     02 means "1 byte of data, then a zero"
 *     03 means "2 bytes of data, then end of frame"
 *
 * Decoding:
 * 1. read a number
 * 2. copy that number minus one byte to the output
 * 3. put a zero back, unless code was FF or this was last block
 * 4. repeat until done
 *
 * Ex. encoded msg: 02 48 03 69 FF
 *     read 02 -> copy 1 byte -> 48 -> put a zero back
 *     output so far: 48 00
 *     read 03 -> copy 69 FF -> (last section)
 *     output: 48 00 69 FF
*/

int frame_encode (const uint8_t *packet,
                  size_t packet_len,
                  uint8_t *output,
                  size_t output_capacity,
                  size_t *output_len)
{
    // reject invalid pointers 
    if(output == NULL || output_len == NULL ||
            (packet == NULL && packet_len > 0)) {
        return -1;
    }
    // set output length to 0 up front
    *output_len = 0;

    if(output_capacity < 1) {
        return -1;
    }

    /* Block: The `code` followed by some data bytes
     * Code: number of bytes until the next zero, counting itself
     * Code index: position in the array where the code goes.
     * */
    size_t code_indx = 0; // location of current block's code byte 
    size_t out_indx  = 1; // next free output slot
                          // 0 reserved for code_indx
    uint8_t code     = 1; // distance to block end
                          // empty block has code 1
    
    // go through every input byte. Zero ends current block
    for(size_t i = 0; i < packet_len; i++) {
        if(packet[i] == 0) {
            output[code_indx] = code;
            code_indx = out_indx;
            if(out_indx >= output_capacity) {
                return -1;
            }
            out_indx++;
            code = 1;
        } else {
            if(out_indx >= output_capacity) {
                return -1;
            }
            output[out_indx++] = packet[i];
            code++;
            if(code == 0xFF) { // full 254-byte block
                               // close it and start a new one
                output[code_indx] = code;
                code_indx = out_indx;
                if(out_indx >= output_capacity) {
                    return -1;
                }
                out_indx++;
                code = 1;
            }
        }
    }
    output[code_indx] = code;

    // delimiter
    if(out_indx >= output_capacity) {
        return -1;
    }
    output[out_indx++] = 0x00;

    *output_len = out_indx;
    return 0;
}

int frame_decode(const uint8_t *frame,
                 size_t frame_len,
                 uint8_t *packet,
                 size_t packet_capacity,
                 size_t *packet_len)
{
    if(frame == NULL || packet == NULL || packet_len == NULL) {
        return -1;
    }
    *packet_len = 0;

    if(frame_len == 0) {
        return -1;
    }
    
    size_t in_indx = 0;  // read cursor: next byte to read from encoded frame
    size_t out_indx = 0; // write cursor: next free slot in the decoded packet

    while(in_indx < frame_len) {
        uint8_t code = frame[in_indx++];
        if(code == 0) {
            return -1;
        }

        size_t n = (size_t)code - 1;
        if(n > frame_len - in_indx) {
            return -1;
        }
        if(n > packet_capacity - out_indx) {
            return -1;
        }

        for(size_t k = 0; k < n; k++) {
            uint8_t b = frame[in_indx++];
            if(b == 0) {
                return -1;
            }
            packet[out_indx++] = b;
        }

        // a block shorter than 0xFF implies a zero unless it is the last
        if(code != 0xFF && in_indx < frame_len) {
            if(out_indx >= packet_capacity) {
                return -1;
            }
            packet[out_indx++] = 0x00;
        }
    }
    *packet_len = out_indx;
    return 0;
}
