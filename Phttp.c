#include "hi_stdlib.h"
//文件目录：C:\hi3861_for_AI_topic-master\src\device\hisilicon\hispark_pegasus\sdk_liteos\include
#include "lwip/sockets.h"
#include "lwip/netdb.h"
//文件目录：C:\hi3861_for_AI_topic-master\src\device\hisilicon\hispark_pegasus\sdk_liteos\third_party\lwip_sack\include\lwip

#define HTTPC_DEMO_RECV_BUFSIZE 64
#define SOCK_TARGET_PORT 80
static const char *g_request = "GET / HTTP/1.1\r\n\
 Content-Type: application/x-www-form-urlencoded;charset=UTF-8\r\n\
 Host: baidu.com\r\n\
 Connection: close\r\n\
 \r\n";

//
unsigned int http_clienti_get(int argc, char* argv[])
{
    if ((argc != 1) || (argv == NULL)) {
        return 1;
     }
    struct sockaddr_in addr = {0};//目标服务器地址
    int s, r; //s存储socket 描述符；r 存储从服务器接收数据的返回值 
    char recv_buf[HTTPC_DEMO_RECV_BUFSIZE]; //数据缓冲区
    addr.sin_family = AF_INET;
    addr.sin_port = PP_HTONS(SOCK_TARGET_PORT);
    addr.sin_addr.s_addr = inet_addr("192.168.43.1");//电脑ip地址
   
    /*分配socket*/
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        return 1;
    }
    printf("... allocated socket");

    /*connect 函数与目标服务器建立 TCP 连接*/
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        printf("... socket connect failed errno=%d", errno);
        lwip_close(s);
        return 1;
    }
    printf("... connected");
    
    /*存储在 g_request 中的 HTTP 请求发送到服务器*/
    if (lwip_write(s, g_request, strlen(g_request)) < 0) {
    lwip_close(s);
    return 1;
    }
    printf("... socket send success");
   
    /* 10S Timeout */
    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 10;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout, sizeof(receiving_timeout)) 
    < 0) {
    printf("... failed to set socket receiving timeout");
    lwip_close(s);
    return 1;
    }
    printf("... set socket receiving timeout success");

    /* 读取响应数据 */
    do {
    (void)memset_s(recv_buf, sizeof(recv_buf), 0, sizeof(recv_buf));
    r = lwip_read(s, recv_buf, sizeof(recv_buf) - 1);
    for (int i = 0; i < r; i++) {
    putchar(recv_buf[i]);
    }
    } while (r > 0);
    printf("... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
    lwip_close(s);
    return 0;
}

//注意：主机端开启服务器，并且监听端口80