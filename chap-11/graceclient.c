# include "lib/common.h"

# define    MAXLINE     4096

// close所做的
// 在输入方向，系统内核会将该套接字设置为不可读，任何读操作都会返回异常。
// 在输出方向，系统内核尝试将发送缓冲区的数据发送给对端，并最后向对端发送一个 FIN 报文，接下来如果再对该套接字进行写操作会返回异常。

// shutdown 函数原型 int shutdown(int sockfd, int howto)
// 枚举值 SHUT_RD | SHUT_WR | SHUT_RDWR

// 辨析
// SHUT_RDWR | close
// 不会释放资源 | 释放资源
// 直接生效 | 根据引用计数机制判断是否生效
// 总是发出FIN | 不总是发出FIN
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: graceclient <IPaddress>");

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

    char send_line[MAXLINE], recv_line[MAXLINE + 1];
    int n;

    // 初始化相应描述字，为select作准备
    fd_set readmask;
    fd_set allreads;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);
    for (;;) {
        readmask = allreads;
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);
        if (rc <= 0)
            error(1, errno, "select failed");
        if (FD_ISSET(socket_fd, &readmask)) {
            n = read(socket_fd, recv_line, MAXLINE);
            // 处理异常
            if (n < 0) {
                error(1, errno, "read error");
            // 正常退出
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }
        if (FD_ISSET(0, &readmask)) {
            if (fgets(send_line, MAXLINE, stdin) != NULL) {
                if (strncmp(send_line, "shutdown", 8) == 0) {
                    // 处理shutdown
                    FD_CLR(0, &allreads);
                    if (shutdown(socket_fd, 1)) {
                        error(1, errno, "shutdown failed");
                    }
                } else if (strncmp(send_line, "close", 5) == 0) {
                    // 处理close
                    FD_CLR(0, &allreads);
                    if (close(socket_fd)) {
                        error(1, errno, "close failed");
                    }
                    sleep(6);
                    exit(0);
                } else {
                    // 处理换行符
                    int i = strlen(send_line);
                    if (send_line[i - 1] == '\n') {
                        send_line[i - 1] = 0;
                    }

                    printf("now sending %s\n", send_line);
                    size_t rt = write(socket_fd, send_line, strlen(send_line));
                    if (rt < 0) {
                        error(1, errno, "write failed ");
                    }
                    printf("send bytes: %zu \n", rt);
                }

            }
        }

    }

}

