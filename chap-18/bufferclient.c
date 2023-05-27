#include "lib/common.h"

// FTP Server采用的常见手段
// struct timeval tv;
// tv.tv_sec = 5;
// tv.tv_usec = 0;
// setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv);

// while (1) {
//     int nBytes = recv(connfd, buffer, sizeof(buffer), 0);
//     if (nBytes == -1) {
//         if (errno == EAGAIN || errno == EWOULDBLOCK) {
//             printf("read timeout\n");
//             onClientTimeout(connfd);
//         } else {
//             error(1, errno, "error read message");
//         }
//     } else if (nBytes == 0) {
//         error(1, 0, "client closed \n");
//     }
//     ...
// }

// struct timeval tv;
// tv.tv_sec = 5;
// tv.tv_usec = 0;

// FD_ZERO(&allreads);
// FD_SET(socket_fd, &allreads);
// for (;;) {
//     readmask = allreads;
//     int rc = select(socket_fd + 1, &readmask, NULL, NULL, &tv);
//     if (rc < 0) {
//       error(1, errno, "select failed");
//     }
//     if (rc == 0) {
//       printf("read timeout\n");
//       onClientTimeout(socket_fd);
//     }
//  ...   
// }




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


    char buf[12];
    int n;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        n = strlen(buf);
        if (send(socket_fd, (char *) &buf, n, 0) < 0)
            error(1, errno, "send failure");

    }
    exit(0);
}

