#include "chat.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void prompt_msg(char* chat_buf, uint32_t length) {
    fgets(chat_buf, length, stdin);
    int msg_len = strnlen(chat_buf, TMP_BUFF_MAX);
    send_msg(chat_buf, msg_len);
}

static void send_msg(char* msg, uint32_t msg_length) {
    assemble_udp(msg, msg_length);
}





void recv_msg(char* msg) {
    printf("New Chat: %s\n", msg);
}
