#include "framing.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Roundtrip test
 * Tests three patterns:
 * - All zeroes
 * - No zeroes
 * - Scrambled bytes: each byte is the previous one + 37, wrapping at 256,
 *   so all 256 byte values appear before the pattern repeats
 */
static void test_roundtrip(void) {
    uint8_t in[300], enc[320], dec[300]; // enc 320 because 300 byte input needs
                                         // at most 300 + 2 overhead + 1 delimiter

    for(size_t len = 0; len <= 300; len++) {
        for(int pattern = 0; pattern < 3; pattern ++) {
            for(size_t i = 0; i < len; i++) {
                if(pattern == 0) 
                    in[i] = 0x00; // all zeroes
                else if(pattern == 1) 
                    in[i] = 0xAA; // no zeroes (hits 254 boundary)
                else 
                    in[i] = (uint8_t)((i * 37u + 11u) % 256u); // scrambled bytes 
            }

            size_t enc_len = 0, dec_len = 0;
            int rc = frame_encode(in, len, enc, sizeof(enc), &enc_len);
            assert(rc == 0);
            assert(enc[enc_len - 1] == 0);
            for(size_t i = 0; i + 1 < enc_len; i++) assert(enc[i] != 0);

            rc = frame_decode(enc, enc_len - 1, dec, sizeof(dec), &dec_len);
            assert(rc == 0);
            assert(dec_len == len);
            assert(memcmp(in, dec, len) == 0);
        }
    }
}

/* Check if bad inputs are rejected
 * 1. code points past the end of the block
 * 2. check if there's a zero in the frame
 * 3. check for empty frame 
 * 4. decode into buffer too small 
 * 5. encode into buffer too small
 */
static void test_bad(void) {
    uint8_t out[16];
    size_t n;

    const uint8_t bad1[] = {0x05, 0x41}; // code points past the end of the block
    const uint8_t bad2[] = {0x03, 0x41, 0x00}; // zero inside frame
    int rc;

    rc = frame_decode(bad1, sizeof(bad1), out, sizeof(out), &n);
    assert(rc == -1);
    rc = frame_decode(bad2, sizeof(bad2), out, sizeof(out), &n);
    assert(rc == -1);
    rc = frame_decode(bad1, 0, out, sizeof(out), &n); // empty frame
    assert(rc == -1);

    // encode into output array too small by one
    const uint8_t msg[] = {0x11, 0x22, 0x33}; // encodes into 5 bytes (04 11 22 33 00)
    uint8_t small[4];                         // but array is sized 4
    rc = frame_encode(msg, sizeof(msg), small, sizeof(small), &n);
    assert(rc == -1);

    // decode into output array too small by one
    const uint8_t enc[] = {0x04, 0x11, 0x22, 0x33}; // decodes into 3 bytes
    uint8_t tiny[2];                                // but array is sized 2
    rc = frame_decode(enc, sizeof(enc), tiny, sizeof(tiny), &n);
    assert(rc == -1);
}

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
                                     //
    // check if output exists
    assert(encoded_len > 0); 
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

    test_roundtrip();
    test_bad();

    puts("Success. All bytes match.");
    return 0;
}

