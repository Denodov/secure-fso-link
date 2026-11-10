#include <stdint.h>
#include "udp.h"

static void assemble_udp(char* data, uint32_t data_length) {
    // UDP packet structure:
    // 2 bytes source port |
    // 2 bytes destination port |
    // 2 bytes length |
    // 2 bytes checksum |
    // variable length data |
    char packet[UDP_BUFF_SIZE];
    uint32_t src_port = SRC;
    uint32_t dst_port = DST;

    // Assemble the UDP header
    memcpy(packet, &src_port, UDP_SRC_LEN);

}
