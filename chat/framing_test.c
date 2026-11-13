#include "framing.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    // 0x48 = 'H'
    // 0x00 is a frame delimiter
    // 0x69 = 'i'
    // 0xFF tests if code can handle a full byte
    uint8_t message[] = {0x48, 0x00, 0x69, 0xFF};

    uint8_t encoded[32];
    uint8_t decoded[32];

    size_t encoded_len = 0;
    size_t decoded_len = 0;

    assert(frame_encode(
                message, sizeof(message),
                encoded, sizeof(encoded),
                &encoded_len) == 0); // did encoding succeed?
    
    assert(encoded_len > 0); // check if output exists
    // check if final byte is 0x00 (frame delimiter)
    assert(encoded[encoded_len - 1] == 0); 

    // check for earlier zero bytes
    for(size_t i = 0; i + 1 < encoded_len; i++) {
        assert(encoded[i] != 0);
    }

    assert(frame_decode(
                encoded, encoded_len - 1,
                decoded, sizeof(decoded),
                &decoded_len) == 0); // did decoding succeed?

    // check if original msg and decoded msg sizes match
    assert(decoded_len == sizeof(message));

    // check if all bytes match
    assert(memcmp(message, decoded, sizeof(message)) == 0);

    puts("Success. All bytes match.");
}

