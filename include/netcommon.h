#ifndef _NET_COMMON_H__
#define _NET_COMMON_H__

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVICE_PORT (8123)
typedef struct tagNetMsg
{
    unsigned int ulMsgType;
    unsigned int ulMsgLen;
    char data[0];
}tNetMsg;

typedef struct tagClientInfo
{
    int fd;
    char ipaddr[128];
    int  port;
}tClientInfo;

#define M_START_RAW (1)
#define M_BUFFER_SIZE (2*1024)
#define M_PACKET_SIZE (1500)
#define M_PRINT_RATE (1000*30)
#define M_MAX_SEND_COUNT (1000*300)

#define debug(format,...) \
    do{ \
    fprintf(stderr,"file( %s ), fun( %s ),line( %d ), "format, __FILE__,__func__,__LINE__, ##__VA_ARGS__); \
} \
while(0)

#endif
