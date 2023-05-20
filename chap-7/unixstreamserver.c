//
// Created by shengym on 2019-07-14.
//

#include  "lib/common.h"

// 采用 read、write收发数据
// 注意事项：
// 必须是本地文件路径，而非目录的。若不存在，会在bind时创建
// 需要保障其为绝对路径
// 需要考虑权限问题
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: unixstreamserver <local_path>");
    }

    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_un cliaddr, servaddr;

    listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (listenfd < 0) {
        error(1, errno, "socket create failed");
    }

    char *local_path = argv[1];
    unlink(local_path);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, local_path);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        error(1, errno, "bind failed");
    }

    if (listen(listenfd, LISTENQ) < 0) {
        error(1, errno, "listen failed");
    }

    clilen = sizeof(cliaddr);
    if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
        if (errno == EINTR)
            error(1, errno, "accept failed");
        else
            error(1, errno, "accept failed");
    }

    char buf[BUFFER_SIZE];

    while (1) {
        bzero(buf, sizeof(buf));
        if (read(connfd, buf, BUFFER_SIZE) == 0) {
            printf("client quit");
            break;
        }
        printf("Receive: %s", buf);

        char send_line[MAXLINE];
        bzero(send_line, MAXLINE);
        sprintf(send_line, "Hi, %s", buf);

        int nbytes = sizeof(send_line);

        if (write(connfd, send_line, nbytes) != nbytes)
            error(1, errno, "write error");
    }

    close(listenfd);
    close(connfd);

    exit(0);

}

// TIME_WAIT 无用链接占用端口， 即端口占用、内存占用
// 本意：保障网络中的数据流正确消亡，以及
// 时间固定 一般为2MSL 最大报文生命周期的2倍
// Linux中一般为60s
// net.ipv4.tcp_tw_reuse 重用
// 达到指定时间后，仍处于TIME_WAIT,即可立即重用
// ps: net.ipv4.tcp_timestamps=1 打开对TCP时间戳的支持
// 无需同步时间，交换后可以获得时间，利用相对时间即可，故无需同步时间戳
