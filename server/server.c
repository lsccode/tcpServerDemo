#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>

#include "ev.h"
#include "netcommon.h"

#define BUFFER_SIZE 1024
#define M_MAX_CLIENT_NUMBER (1024)
typedef struct tagClientfdArray
{
    unsigned int size;
    int szClientFd[M_MAX_CLIENT_NUMBER];
}tClientfdArray;

tClientfdArray gstClientArray = {0};

//read client 
void passiveClientRead(struct ev_loop *loop, struct ev_io *watcher, int revents){
    char buffer[BUFFER_SIZE];
    ssize_t read;
    int index = 0;
    
    if(EV_ERROR & revents)
    {
        debug("error event in read");
        return;
    }
    
//recv 
    read = recv(watcher->fd, buffer, BUFFER_SIZE, 0);
    
    if(read < 0)
    {
        debug("read error");
        return;
    }
    
//client disconnect
    if(read == 0)
    {
        for(index = 0; index < M_MAX_CLIENT_NUMBER; ++index)
        {
            if(gstClientArray.szClientFd[index] == watcher->fd)
            {
                gstClientArray.szClientFd[index] = 0;
            }
        }
        debug("someone disconnected.\n");
        close(watcher->fd);
        ev_io_stop(loop,watcher);
        free(watcher);
        
        return;
    }
    else
    {
        tNetMsg *pstNetMsg = (tNetMsg *)buffer;
        if(pstNetMsg->ulMsgType == M_START_RAW)
        {
            debug("get the message ulMsgType :%d\n",pstNetMsg->ulMsgType);
            debug("start send rawdata\n");
            pstNetMsg->ulMsgType = 10000;
            send(watcher->fd,buffer,sizeof(tNetMsg),0);
        }
        
    }
}

//accept server callback
void serverAcceptRead(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sd;
    int index = 0;
    
    struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));
    
    if(EV_ERROR & revents)
    {
        debug("error event in accept");
        return;
    }
    
    if(NULL == w_client)
    {
        debug("create client error!\n");
        return;
    }
    
    client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_sd < 0)
    {
        debug("accept error");
        return;
    }
    
    for(index = 0; index < M_MAX_CLIENT_NUMBER; ++index)
    {
        if(gstClientArray.szClientFd[index] == 0)
        {
            gstClientArray.szClientFd[index] = client_sd;
            break;
        }
    }
    
    if(index == M_MAX_CLIENT_NUMBER)
    {
        free(w_client);
        close(client_sd);
    }
    else
    {
        debug("someone connected.\n");
        ev_io_init(w_client, passiveClientRead, client_sd, EV_READ);
        ev_io_start(loop, w_client);
    }
    

}

int createTcpServer()
{
    int server_sockfd;
    int server_len;
    struct sockaddr_in server_address;
    
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int reuseFlag = 1;
    socklen_t socklen = sizeof(reuseFlag);
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseFlag, socklen);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVICE_PORT);
    server_len = sizeof(server_address);
    if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0)
    {
        debug("bind ,%d %s failed\n",errno,strerror(errno));
    }

    listen(server_sockfd, 5);
    return server_sockfd;
}

void sigpipe(int sig)
{
    debug("server recv signal=%d\n",sig);
    return;
}
int main()
{
    struct ev_loop *loop = ev_default_loop(0);
    struct ev_io evServer;
    int serverfd = createTcpServer();   
    
    ev_io_init(&evServer, serverAcceptRead, serverfd, EV_READ);
    ev_io_start(loop, &evServer);
    
    signal(SIGPIPE,sigpipe);
    while (1)
    {
        ev_loop(loop, 0);
    }
    
    return 0;
}




