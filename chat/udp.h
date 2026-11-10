#ifndef UDP_H
#define UDP_H
#include <stdint.h>
#define UDP_BUFF_SIZE 256 // temporary limit

// hardcoded source and destination ports for now
#define SRC 5000
#define DST 5001

// UDP Header structure
#define UDP_SRC_LEN 2
#define UDP_DST_LEN 2
#define UDP_LENGTH_LEN 2
#define UDP_CHKSM_LEN 2

static void assemble_udp(char* data, uint32_t data_length);

#endif
