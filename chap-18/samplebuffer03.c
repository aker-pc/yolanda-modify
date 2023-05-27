//
// Created by shengym on 2019-07-07.
//

#include "lib/common.h"

static int count;

static void sig_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}


int main(int argc, char **argv) {
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        error(1, errno, "bind failed ");
    }

    int rt2 = listen(listenfd, LISTENQ);
    if (rt2 < 0) {
        error(1, errno, "listen failed ");
    }

    signal(SIGPIPE, SIG_IGN);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
        error(1, errno, "bind failed ");
    }

    char buf[10];

    while (1) {
        int nBytes = readline(connfd, buf,10);
        if (nBytes == -1) {
            error(1, errno, "error read message");
        } else if (nBytes == 0) {
            error(1, 0, "client closed \n");
        }

        printf("received %d bytes: %s\n", nBytes, buf);
    }

    exit(0);

}

// 最终缓冲区容量的抉择 以及缓冲区是否可以通过动态分配来解决缓冲区溢出的问题
// 1，最终缓冲区的大小应该比预计接收的数据大小大一些，预防缓冲区溢出。
// 2，完全可以动态分配，但是要记得在return前释放缓冲区

// TTL 最大值为255 8位字段（跳数）
// 标准规定时间是2分钟 实际linux为30s

// listen的backlog 已完成的连接队列数量 默认为128 分发性能好的话 可以做到成千上万的并发

// UDP的connect目的在于记录映射关系
// UDP不进行connect还可以收到信息，是因为报文存储了对端的地址

// ICMP报文这里 用目的地址和端口进行区分，如果没有，你是没法做到通知对应程序 连接不可达这事的
// 我们知道，对于 TCP 套接字，connect 只能调用一次。
// 但是，对一个 UDP 套接字来说，进行多次 connect 操作是被允许的，这样主要有两个作用。

// 第一个作用是可以重新指定新的 IP 地址和端口号；第二个作用是可以断开一个已连接的套接字
// 为了断开一个已连接的 UDP 套接字，第二次调用 connect 时，调用方需要把套接字地址结构的地址族成员设置为 AF_UNSPEC。


