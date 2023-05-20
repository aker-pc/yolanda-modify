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

    // 设置套接字重用
    // SO_REUSEADDR 套接字选项还有一个作用
    // 那就是本机服务器如果有多个地址，可以在不同地址上使用相同的端口提供服务
    // 没有不安全的问题，毕竟四元组不重复即可
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

    char message[MAXLINE];
    count = 0;

    for (;;) {
        int n = read(connfd, message, MAXLINE);
        if (n < 0) {
            error(1, errno, "error read");
        } else if (n == 0) {
            error(1, 0, "client closed \n");
        }
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        count++;
    }

}

// TCP 的机制绝对不允许在相同的地址和端口上绑定不同的服务器
// tcp_tw_reuse 是内核选项，主要用在连接的发起方。
// TIME_WAIT 状态的连接创建时间超过 1 秒后，新的连接才可以被复用，注意，这里是连接的发起方；
// SO_REUSEADDR 是用户态的选项，SO_REUSEADDR 选项用来告诉操作系统内核，如果端口已被占用，但是 TCP 连接状态位于 TIME_WAIT
// 可以重用端口。如果端口忙，而 TCP 处于其他状态，重用端口时依旧得到“Address already in use”的错误信息。注意，这里一般都是连接的服务方

// 关于tcp_tw_reuse和SO_REUSEADDR的区别，可以概括为：tcp_tw_reuse是为了缩短time_wait的时间，避免出现大量的time_wait链接而占用系统资源，解决的是accept后的问题；
// SO_REUSEADDR是为了解决time_wait状态带来的端口占用问题，以及支持同一个port对应多个ip，解决的是bind时的问题。