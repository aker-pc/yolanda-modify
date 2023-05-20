#include "lib/common.h"

// send 对数据的实际发送形式对用户是透明的
// recv 同理，但会保障相对顺序

// POSIX 保证字节序一致的接口
// h host n network s short l long
// uint16_t htons (uint16_t hostshort)
// uint16_t ntohs (uint16_t netshort)
// uint32_t htonl (uint32_t hostlong)
// uint32_t ntohl (uint32_t netlong)
// 网络字节序 默认大端序


int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: tcpclient <IPaddress>");
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr *) &server_addr, server_len);
    if (connect_rt < 0) {
        error(1, errno, "connect failed ");
    }

    struct {
        u_int32_t message_length;
        u_int32_t message_type;
        char data[128];
    } message;


    int n;

    while (fgets(message.data, sizeof(message.data), stdin) != NULL) {
        n = strlen(message.data);
        message.message_length = htonl(n);
        message.message_type = 1;
        if (send(socket_fd, (char *) &message, sizeof(message.message_length) + sizeof(message.message_type) + n, 0) <
            0)
            error(1, errno, "send failure");

    }
    exit(0);
}

