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
        debug("connect %s:%d error ,%s(%d)!\n",ipaddr,SERVICE_PORT,strerror(errno),errno);
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
                debug("recv %llu, count %u,message type (%u)\n",sum,count,pstNetMsg->ulMsgType);
            //write(savefd,buffer,read);
        }
        else
        {
            tClientInfo  *pstClient = (tClientInfo *)watcher->data;
            debug("%s:%d fd(%d) recv error %d,disconnected!\n", pstClient->ipaddr,pstClient->port, watcher->fd,read);
            
            debug("total recv %llu, count %u,message type (%u)\n",sum,count,pstNetMsg->ulMsgType);
            close(watcher->fd);
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

void signalCallback(EV_P_ ev_signal *w, int revents)
{
	//debug("server recv signal=%d\n",sig);
	// this causes the innermost ev_run to stop iterating
	//ev_break (EV_A_ EVBREAK_ONE);
	if(w->signum == SIGINT)
	{
		debug("server recv SIGINT(%d)\n",w->signum);
        exit(0);
	}
    
    if(w->signum == SIGPIPE)
    {
        debug("server recv SIGPIPE(%d)\n",w->signum);
    }
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
    struct sockaddr_in addr;
    int len;
    tClientInfo  *pstClient = (tClientInfo*)malloc (sizeof(tClientInfo));
    
    if(sockfd < 0)
    {
        debug("can not connected %s:%d!\n", argv[1], SERVICE_PORT);
        return -1;
    }
    
    getpeername(sockfd, (struct sockaddr *)&addr, &len);
    inet_ntop(AF_INET,&(addr.sin_addr),pstClient->ipaddr,sizeof(pstClient->ipaddr)); 
    pstClient->port = ntohs(addr.sin_port);
    evClient.data = pstClient;
    debug("%s:%d fd(%d),connected!\n", pstClient->ipaddr, pstClient->port, sockfd);
    
    ev_io_init(&evClient, clientRead, sockfd, EV_READ | EV_WRITE);
    ev_io_start(loop, &evClient);

    //signal(SIGPIPE,sigpipe);
    ev_signal watcher_signalint;
	ev_signal_init(&watcher_signalint, signalCallback,SIGINT);
	ev_signal_start (loop, &watcher_signalint);
    
    ev_signal watcher_signalpipe;
	ev_signal_init(&watcher_signalpipe, signalCallback,SIGPIPE);
	ev_signal_start (loop, &watcher_signalpipe);
    
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
