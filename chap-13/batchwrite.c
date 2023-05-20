#include "lib/common.h"

// 在时延敏感的情形需要这么做
// int on = 1; 
// setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on)); 


int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: batchwrite <IPaddress>");
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

    char buf[128];
    struct iovec iov[2];

    char *send_one = "hello,";
    iov[0].iov_base = send_one;
    iov[0].iov_len = strlen(send_one);
    iov[1].iov_base = buf;
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        iov[1].iov_len = strlen(buf);
        int n = htonl(iov[1].iov_len);
        // 集中写进数据
        // 结构体数组 | 2 结构体数组长度
        if (writev(socket_fd, iov, 2) < 0)
            error(1, errno, "writev failure");
    }

    // 发送窗口用来控制发送和接收端的流量；阻塞窗口用来控制多条连接公平使用的有限带宽。 
    // 小数据包加剧了网络带宽的浪费，为了解决这个问题，引入了如 Nagle 算法、延时 ACK 等机制。
    // 在程序设计层面，不要多次频繁地发送小报文，如果有，可以使用 writev 批量发送
    exit(0);
}

