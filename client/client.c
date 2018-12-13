#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ev.h"
#include "netcommon.h"

// getLocalTimeStr ignore error and null check
int savefd = -1;
void getLocalTimeStr(char *str)
{
    time_t t;
    struct tm *lt;
    time(&t);

    lt=localtime(&t);

    sprintf(str,"%04d%02d%02d-%02d%02d%02d.raw",
            lt->tm_year+1900,lt->tm_mon+1,lt->tm_mday,
            lt->tm_hour,lt->tm_min,lt->tm_sec);

    return;
}


int createTcpClient(char *ipaddr)
{
    int sockfd;
    int len;
    //struct sockaddr_un address;
    struct sockaddr_in address;
    int result;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ipaddr);
    address.sin_port = htons(SERVICE_PORT);
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr *)&address, len);

    if(result == -1) {
        debug("connect %s:%d error ,%d : %s,",ipaddr,9734,errno,strerror(errno));
        close(sockfd);
        sockfd = -1;
        //exit(1);
    }
    
    return sockfd;
}

unsigned int count = 0;
unsigned long long sum = 0;

void clientRead(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    char buffer[M_BUFFER_SIZE] = {0};    
    
    if(revents & EV_READ)
    {
        tNetMsg *pstNetMsg = (tNetMsg *)buffer;
        ssize_t read = recv(watcher->fd, buffer, M_PACKET_SIZE, 0);
        if(read > 0)
        {
            sum += read;
            if(++count%(M_PRINT_RATE) == 0)
                debug("total recv %llu, count %u,message type (%u)\n",sum,count,pstNetMsg->ulMsgType);
            //write(savefd,buffer,read);
        }
        else
        {
            debug("recv error %d,disconnect...\n",read);
            debug("total recv %llu, count %u,message type (%u)\n",sum,count,pstNetMsg->ulMsgType);
            //close(watcher->fd);
            ev_io_stop(loop, watcher);
        }
    }
    
    if(revents & EV_WRITE)
    {
        tNetMsg *pstNetMsg = (tNetMsg *)buffer;
        
        pstNetMsg->ulMsgType = M_START_RAW;
        pstNetMsg->ulMsgLen  = 0;
        if(send(watcher->fd,buffer,sizeof(tNetMsg),0) < 0)
        {
            debug("send M_START_RAW message error!\n");
        }
        else
        {
            ev_io_stop(loop, watcher);
            ev_io_init(watcher, clientRead, watcher->fd, EV_READ);
            ev_io_start(loop, watcher);
            debug("send start message ok!\n");
        }
    }
    
 
}

void sigpipe(int sig)
{
    debug("server recv signal=%d\n",sig);
    return;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"    client [ipaddr]\n\n");
        return -1;
    }
    
    char filename[128] = {0};
    struct ev_loop *loop = ev_default_loop(0);
    int sockfd = createTcpClient(argv[1]);
    struct ev_io evClient;
    
    ev_io_init(&evClient, clientRead, sockfd, EV_READ | EV_WRITE);
    ev_io_start(loop, &evClient);

    signal(SIGPIPE,sigpipe);//捕捉第二次write的SIGPIPE信号,默认终止进程
    
    getLocalTimeStr(filename);
    
    savefd = open(filename,O_WRONLY | O_CREAT);
    if(savefd < 0)
    {
        debug("open %s error\n",filename);
    }
    
    while (1)
    {
        ev_loop(loop, 0);
    }
    
    return 0;
    
}
