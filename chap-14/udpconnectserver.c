//
// Created by shengym on 2019-07-13.
//

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

    bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    socklen_t client_len;
    char message[MAXLINE];
    message[0] = 0;
    count = 0;

    signal(SIGINT, recvfrom_int);

    struct sockaddr_in client_addr;
    client_len = sizeof(client_addr);

    int n = recvfrom(socket_fd, message, MAXLINE, 0, (struct sockaddr *) &client_addr, &client_len);
    if (n < 0) {
        error(1, errno, "recvfrom failed");
    }
    message[n] = 0;
    printf("received %d bytes: %s\n", n, message);

    // 由于是udp，无法得知client地址
    // udp报文中包含了端口和地址信息
    // 日后每次发送都会携带客户端的地址和端口信息
    // 故先recvfrom 而后connect
    // connect意义在于绑定，无法多个服务端绑定同一个客户端（该点存疑）
    // 解答 可以对一个 UDP 套接字进行多次 connect 操作吗? 你不妨动手试试，看看结果
    // 答案是可以的
    // 如果想使用多播或广播，我们应该怎么去使用 connect 呢？
    // 对于广播的话，先把广播的option打开。然后再 connect 255.255.255.255(组播同样的)

    if (connect(socket_fd, (struct sockaddr *) &client_addr, client_len)) {
        error(1, errno, "connect failed");
    }

    while (strncmp(message, "goodbye", 7) != 0) {
        char send_line[MAXLINE];
        sprintf(send_line, "Hi, %s", message);

        size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
        if (rt < 0) {
            error(1, errno, "send failed ");
        }
        printf("send bytes: %zu \n", rt);

        size_t rc = recv(socket_fd, message, MAXLINE, 0);
        if (rc < 0) {
            error(1, errno, "recv failed");
        }

        count++;
    }

    exit(0);
}

// 可以通过connect提高性能
// 因为如果不使用 connect 方式，每次发送报文都会需要这样的过程： 连接套接字→发送报文→断开套接字→连接套接字→发送报文→断开套接字 →……… 
// 而如果使用 connect 方式，就会变成下面这样： 连接套接字→发送报文→发送报文→……→最后断开套接字

// udp 如何实现可靠性
// 我能想到的： 1.udp可以增加消息编号； 2.对每个消息编号提供ACK，在udp应用层增加应答机制； 3.没有应答的增加重传机制 4.增加缓存，ACK完的才从缓存中清除

// 对于recvfrom函数，我们可以看成是TCP中accept函数和read函数的结合，前三个参数是read的参数，后两个参数是accept的参数。
// 对于sendto函数，则可以看成是TCP中connect函数和send函数的结合，前三个参数是send的参数，后两个参数则是connect的参数。
// 所以udp在发送和接收数据的过程中都会建立套接字连接，只不过每次调用sendto发送完数据后，内核都会将临时保存的对端地址数据删除掉，也就是断开套接字，从而就会出现老师所说的那个循环


