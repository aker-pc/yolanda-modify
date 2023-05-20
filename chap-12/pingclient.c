#include "lib/common.h"
#include "message_objecte.h"

#define    MAXLINE     4096
#define    KEEP_ALIVE_TIME  10
#define    KEEP_ALIVE_INTERVAL  3
#define    KEEP_ALIVE_PROBETIMES  3

// 上述的可定义变量，分别被称为保活时间、保活时间间隔和保活探测次数。
// 在 Linux 系统中，这些变量分别对应 sysctl 变量net.ipv4.tcp_keepalive_time、net.ipv4.tcp_keepalive_intvl、 net.ipv4.tcp_keepalve_probes，
// 默认设置是 7200 秒（2 小时）、75 秒和 9 次探测。

// 注意场景
// 对端程序正常工作，保证保活时间可以重置
// 对端程序崩溃，但可响应，返回RST报文
// 对端程序崩溃，不可相应，达到保活探测次数表明效果

// 上述描述是基于TCP传输层的实现
// 这个时间间隔有些时候不足够应付现在的实时系统，故为提高探活效率

// 实现机制
// 1.定时器 I/O 自身机制
// 2.ping-pong协议

// 消息格式
// typedef struct {
//     u_int32_t type;
//     char data[1024];
// } messageObject;

// #define MSG_PING          1
// #define MSG_PONG          2
// #define MSG_TYPE1        11
// #define MSG_TYPE2        21

// 维持多个链接，可以保持效率
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

    char recv_line[MAXLINE + 1];
    int n;

    fd_set readmask;
    fd_set allreads;


    struct timeval tv;
    int heartbeats = 0;

    // 定时保活的请求参数
    tv.tv_sec = KEEP_ALIVE_TIME;
    tv.tv_usec = 0;

    messageObject messageObject;

    FD_ZERO(&allreads);
    FD_SET(0, &allreads);
    FD_SET(socket_fd, &allreads);
    for (;;) {
        readmask = allreads;
        // 引入超时处理
        int rc = select(socket_fd + 1, &readmask, NULL, NULL, &tv);
        if (rc < 0) {
            error(1, errno, "select failed");
        }
        // 超时处理逻辑
        if (rc == 0) {
            if (++heartbeats > KEEP_ALIVE_PROBETIMES) {
                error(1, 0, "connection dead\n");
            }
            printf("sending heartbeat #%d\n", heartbeats);
            messageObject.type = htonl(MSG_PING);
            rc = send(socket_fd, (char *) &messageObject, sizeof(messageObject), 0);
            if (rc < 0) {
                error(1, errno, "send failure");
            }
            tv.tv_sec = KEEP_ALIVE_INTERVAL;
            continue;
        }
        // 常规报文的处理逻辑
        if (FD_ISSET(socket_fd, &readmask)) {
            n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            printf("received heartbeat, make heartbeats to 0 \n");
            // 正常报文等同于探活
            heartbeats = 0;
            tv.tv_sec = KEEP_ALIVE_TIME;
        }
    }
}

