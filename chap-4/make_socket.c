#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int make_socket(uint16_t port) {
    int sock;
    struct sockaddr_in name;

    /* 创建字节流类型的IPV4 socket. */
    // 函数原型 int socket(int domain, int type, int protocol);
    // domain: PF_INET IPV4 PF_INET6 IPV6 ……
    // type: SOCK_STREAM TCP SOCK_DIGRAM UDP
    // protocol: 用于表示通信协议，现在默认为0即可，已经失去意义
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* 绑定到port和ip. */
    name.sin_family = AF_INET; /* IPV4 */
    // 由于网络字节序可能跟实际的硬件设备有关，需要通过htons、htonl这两个函数来保证字节序一致性
    name.sin_port = htons (port);  /* 指定端口 */
    name.sin_addr.s_addr = htonl (INADDR_ANY); /* 通配地址 */

    // 函数原型
    //    bind(int fd, void * addr, socklen_t len)
    /* 把IPV4地址转换成通用地址格式，同时传递长度 sockaddr_in-> sockaddr */
    if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    return sock;
}

// listen
// 函数原型 int listen (int socketfd, int backlog)
// backlog Linux 中表示已完成 (ESTABLISHED) 且未 accept 的队列大小，一般系统不允许修改，修改过大可能会影响操作系统的性能

// accept 阻塞
// 函数原型 int accept(int listensockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
// cliaddr 客户端地址及其大小
// listensockfd 为了并发的在同一个sockfd中处理多个事务

// 以上均是服务端的内容

// connect 
// sockfd = socket(PF_INET, SOCK_STREAM, 0);
// 函数原型 int connect(int sockfd, const struct sockaddr *servaddr, socklen_t addrlen)
// client 不一定非得要bind这个过程

// 出错情形
// 1.TIMEOUT 可能是服务端地址写错，客户端的SYN请求发出去后无响应
// 2.RST、CONNECTION REFUSE 端口写错了
//   产生条件：端口错误、TCP想取消已有连接、TCP收错了消息
// 3.destination unreachable 通常事路由不通，首先怀疑一下防火墙



int main(int argc, char **argv) {
    int sockfd = make_socket(12345);
    exit(0);
}
