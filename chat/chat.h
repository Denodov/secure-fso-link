#ifndef CHAT_H
#define CHAT_H
#define TMP_BUFF_MAX 100
#include <stdint.h>

void prompt_msg(char* chat_buf, uint32_t length);
void recv_msg(char* msg);

static void send_msg(char* msg, uint32_t msg_length);

#endif
