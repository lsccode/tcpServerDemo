#ifndef _NET_COMMON_H__
#define _NET_COMMON_H__

#define SERVICE_PORT (8123)
typedef struct tagNetMsg
{
    unsigned int ulMsgType;
    unsigned int ulMsgLen;
    char data[0];
}tNetMsg;

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
