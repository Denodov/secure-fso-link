#include "chat.h"
#include <stdio.h>

int main(void) {
    printf("Welcome to chat.\n");
    char buf[TMP_BUFF_MAX];
    while(1) {
        prompt_msg(buf, TMP_BUFF_MAX);
        recv_msg(buf);
    }
}
