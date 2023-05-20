//
// Created by shengym on 2019-07-07.
//

#include "lib/common.h"

static int count;

static void recvfrom_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}


int main(int argc, char **argv) {
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int err_bind = bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (err_bind < 0) { 
        printf("err bind!!!");
        return -1;
    }

    socklen_t client_len;
    char message[MAXLINE];
    count = 0;

    // 信号处理机制 kill -INT <进程号>
    signal(SIGINT, recvfrom_int);

    struct sockaddr_in client_addr;
    client_len = sizeof(client_addr);
    for (;;) {

        // __restrict 是 C99 标准中新增的关键字，用于告诉编译器指针所指向的内存区域不会被其他指针所访问
        // 函数原型 recvfrom (int __fd, void *__restrict __buf, size_t __n, int __flags, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len)
        // __flags 是和 I/O 相关的参数
        int n = recvfrom(socket_fd, message, MAXLINE, 0, (struct sockaddr *) &client_addr, &client_len);
        message[n] = 0;
        // message[1024] = 0;
        // 实际上为缓冲区内的字符串添加结束符0
        printf("received %d bytes: %s\n", n, message);

        char send_line[MAXLINE];
        sprintf(send_line, "Hi, %s", message);

        sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr *) &client_addr, client_len);

        count++;
    }

}


