#include "lib/common.h"

#define  MAXLINE     1024

// select 拥有打开文件描述符号的数量限制，poll和epoll无此问题 限制FD_SET_SIZE 1024
// 每次返回值都会修改fd_set，故每次读取完成后，你需要充值fd_set的内容
// 函数原型如下
// int select(int nfds, fd_set *readfds, fd_set *writefd,
//                  fd_set *exceptfds, struct timeval *timeout);
// 参数解释如下
// nfds: 已经打开的文件描述符的数量，传入时需要+1 
// 监听三种事件 readfds\writefd\exceptfds
// timeout 设置等待时间，默认为null，没有数据就一直阻塞

// 0 代表无需处理 1代表需要处理
// void FD_CLR(int fd, fd_set *set); FD_CLR 将fd对应在set中的标志位设为0
// int  FD_ISSET(int fd, fd_set *set); FD_ISSET 判断fd对应在set中的标志位是0还是1
// void FD_SET(int fd, fd_set *set);  FD_SET 将fd对应在set中的标志位设为1
// void FD_ZERO(fd_set *set); FD_ZERO 初始化set，统一设置成0
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: select01 <IPaddress>");
    }
    int socket_fd = tcp_client(argv[1], SERV_PORT);

    char recv_line[MAXLINE], send_line[MAXLINE];
    int n;

    fd_set readmask;

    // allreads 用于初始化的fd_set
    // 套接字中 0 1 2 stdin stdout stderr
    fd_set allreads;
    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);


    // 什么情况下的套接字不可写呢
    // 有以下三种情况
    // 连接的写半边已经关闭 通过colose函数可以做到单向的关闭
    // 套接字上有错误待处理，直接返回
    // 套接字发送缓冲区足够大，如果使用阻塞套接字进行write操作，直接返回
    for (;;) {
        // 切记需要重置
        readmask = allreads;
        // 切记此处需要 + 1 由于索引的缘故
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, NULL);

        if (rc <= 0) {
            error(1, errno, "select failed");
        }

        if (FD_ISSET(socket_fd, &readmask)) {
            n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &readmask)) {
            if (fgets(send_line, MAXLINE, stdin) != NULL) {
                int i = strlen(send_line);
                if (send_line[i - 1] == '\n') {
                    send_line[i - 1] = 0;
                }

                printf("now sending %s\n", send_line);
                ssize_t rt = write(socket_fd, send_line, strlen(send_line));
                if (rt < 0) {
                    error(1, errno, "write failed ");
                }
                printf("send bytes: %zu \n", rt);
            }
        }
    }

}

// 1.可以检测管道的读写事件嘛，可以
// 2.需要穿最大描述基数嘛，不一定需要，传入的目的是为了缩小范围，提高相应的查询效率
